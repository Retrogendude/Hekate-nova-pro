/* Hekate Nova Pro SD installer. GPL-2.0-or-later. */
#include "install.h"
#include <setupapi.h>
#include <wincrypt.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>
#include "payload.h"

const PackageFile *PackageFiles(int variant,int *count){*count=variant?COUNT8:COUNT4;return variant?files8:files4;}
static BOOL Error(InstallContext*c,const wchar_t *what,DWORD code){
    wchar_t detail[512]=L"";
    if(code)FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,code,0,detail,512,NULL);
    _snwprintf(c->error,1399,L"%ls\r\n%ls (code %lu)",what,detail,(unsigned long)code);c->error[1399]=0;return FALSE;
}
static void Report(InstallContext*c,int pos,int max,const wchar_t*text,BOOL commit){if(c->progress)c->progress(pos,max,text,commit,c->user);}
BOOL ValidRelative(const wchar_t*n){
    if(!n||!*n||wcslen(n)>180||n[0]==L'\\'||n[0]==L'/')return FALSE;
    const wchar_t *start=n;
    for(const wchar_t *p=n;;++p){
        if(*p==L':'||*p==L'*'||*p==L'?'||*p==L'"'||*p==L'<'||*p==L'>'||*p==L'|'||(*p&&*p<32))return FALSE;
        if(!*p||*p==L'/'||*p==L'\\'){
            size_t len=p-start;if(!len||(len==1&&*start==L'.')||(len==2&&start[0]==L'.'&&start[1]==L'.')||p[-1]==L'.'||p[-1]==L' ')return FALSE;
            if(!*p)break;
            start=p+1;
        }
    }
    return TRUE;
}
BOOL SafePath(const wchar_t*root,const wchar_t*name,BOOL parents,wchar_t*out){
    if(!ValidRelative(name)||wcslen(root)+wcslen(name)+2>=MAX_PATH){SetLastError(ERROR_INVALID_NAME);return FALSE;}
    DWORD a=GetFileAttributesW(root);
    if(a==INVALID_FILE_ATTRIBUTES||!(a&FILE_ATTRIBUTE_DIRECTORY)||(a&FILE_ATTRIBUTE_REPARSE_POINT)){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    swprintf(out,MAX_PATH,L"%ls%ls%ls",root,root[wcslen(root)-1]==L'\\'?L"":L"\\",name);
    for(wchar_t*p=out+wcslen(root);*p;++p){if(*p==L'/')*p=L'\\';}
    for(wchar_t*p=out+wcslen(root);*p;++p){
        if(*p!=L'\\')continue;
        *p=0;
        a=GetFileAttributesW(out);
        if(a==INVALID_FILE_ATTRIBUTES&&parents){if(!CreateDirectoryW(out,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS){*p=L'\\';return FALSE;}a=GetFileAttributesW(out);}
        *p=L'\\';
        if(a!=INVALID_FILE_ATTRIBUTES&&(!(a&FILE_ATTRIBUTE_DIRECTORY)||(a&FILE_ATTRIBUTE_REPARSE_POINT))){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    }
    a=GetFileAttributesW(out);
    if(a!=INVALID_FILE_ATTRIBUTES&&(a&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return TRUE;
}
static BOOL HashBytes(const BYTE*data,DWORD length,char*hex){
    HCRYPTPROV p=0;HCRYPTHASH h=0;BYTE sum[32];DWORD n=32;BOOL ok=FALSE;
    if(CryptAcquireContextW(&p,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)&&CryptCreateHash(p,CALG_SHA_256,0,0,&h)&&CryptHashData(h,data,length,0)&&CryptGetHashParam(h,HP_HASHVAL,sum,&n,0)){
        for(int i=0;i<32;++i)sprintf(hex+i*2,"%02x",sum[i]);
    hex[64]=0;ok=TRUE;
    }
    if(h)CryptDestroyHash(h);
    if(p)CryptReleaseContext(p,0);
    return ok;
}
BOOL HashFileMatches(const wchar_t*path,const char*expected){
    HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);if(f==INVALID_HANDLE_VALUE)return FALSE;
    HCRYPTPROV p=0;HCRYPTHASH h=0;BOOL ok=FALSE;BYTE *buf=malloc(262144),sum[32];DWORD n=0,sz=32;char hex[65];
    if(!buf||!CryptAcquireContextW(&p,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)||!CryptCreateHash(p,CALG_SHA_256,0,0,&h))goto out;
    for(;;){if(!ReadFile(f,buf,262144,&n,NULL))goto out;if(!n)break;if(!CryptHashData(h,buf,n,0))goto out;}
    if(!CryptGetHashParam(h,HP_HASHVAL,sum,&sz,0))goto out;
    for(int i=0;i<32;++i)sprintf(hex+i*2,"%02x",sum[i]);
    hex[64]=0;ok=!strcmp(hex,expected);
out:if(h)CryptDestroyHash(h);if(p)CryptReleaseContext(p,0);CloseHandle(f);free(buf);return ok;
}
BOOL ValidateDrive(InstallContext*c){
#ifdef INSTALLER_TEST
    if(c->fixture){DWORD a=GetFileAttributesW(c->root);return a!=INVALID_FILE_ATTRIBUTES&&(a&FILE_ATTRIBUTE_DIRECTORY)&&!(a&FILE_ATTRIBUTE_REPARSE_POINT);}
#endif
    if(wcslen(c->root)!=3||c->root[0]<L'A'||c->root[0]>L'Z'||c->root[1]!=L':'||c->root[2]!=L'\\')return Error(c,L"Choisissez une lettre de carte SD valide.",ERROR_INVALID_DRIVE);
    wchar_t windows[MAX_PATH],fs[64];GetWindowsDirectoryW(windows,MAX_PATH);
    if(towupper(windows[0])==c->root[0])return Error(c,L"Le lecteur de Windows ne peut pas être utilisé.",ERROR_ACCESS_DENIED);
    UINT type=GetDriveTypeW(c->root);if(type!=DRIVE_REMOVABLE&&type!=DRIVE_FIXED)return Error(c,L"Ce lecteur ne convient pas à une carte SD.",ERROR_INVALID_DRIVE);
    if(!GetVolumeInformationW(c->root,NULL,0,&c->serial,NULL,NULL,fs,64))return Error(c,L"Carte SD inaccessible. Réinsérez-la puis actualisez la liste.",GetLastError());
    if(_wcsicmp(fs,L"FAT32")&&_wcsicmp(fs,L"exFAT"))return Error(c,L"La carte doit déjà être en FAT32 ou exFAT. Cet installateur ne la formate pas.",ERROR_UNRECOGNIZED_VOLUME);
    if(!GetVolumeNameForVolumeMountPointW(c->root,c->guid,MAX_PATH))return Error(c,L"Impossible d'identifier ce lecteur.",GetLastError());
    return TRUE;
}
static BOOL SameDrive(InstallContext*c){
#ifdef INSTALLER_TEST
    if(c->fixture)return TRUE;
#endif
    wchar_t guid[MAX_PATH];DWORD serial=0;
    return GetVolumeNameForVolumeMountPointW(c->root,guid,MAX_PATH)&&!wcscmp(guid,c->guid)&&GetVolumeInformationW(c->root,NULL,0,&serial,NULL,NULL,NULL,0)&&serial==c->serial;
}
static BOOL PrivateDir(const wchar_t*base,const wchar_t*prefix,wchar_t*out){
    BYTE random[8];HCRYPTPROV p=0;
    if(!CryptAcquireContextW(&p,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))return FALSE;
    BOOL ok=CryptGenRandom(p,8,random);CryptReleaseContext(p,0);if(!ok)return FALSE;
    wchar_t id[17];for(int i=0;i<8;++i)swprintf(id+i*2,3,L"%02x",random[i]);
    if(wcslen(base)+wcslen(prefix)+20>=MAX_PATH){SetLastError(ERROR_FILENAME_EXCED_RANGE);return FALSE;}
    swprintf(out,MAX_PATH,L"%ls%ls%ls%ls",base,base[wcslen(base)-1]==L'\\'?L"":L"\\",prefix,id);
    return CreateDirectoryW(out,NULL);
}
/* Only called on freshly created installer-owned directories. Never follows links. */
static BOOL RemoveTree(const wchar_t*dir){
    wchar_t mask[MAX_PATH],path[MAX_PATH];if(wcslen(dir)+4>=MAX_PATH)return FALSE;
    DWORD a=GetFileAttributesW(dir);if(a==INVALID_FILE_ATTRIBUTES)return TRUE;
    if(a&FILE_ATTRIBUTE_REPARSE_POINT)return RemoveDirectoryW(dir);
    swprintf(mask,MAX_PATH,L"%ls\\*",dir);WIN32_FIND_DATAW data;HANDLE h=FindFirstFileW(mask,&data);BOOL ok=TRUE;
    if(h!=INVALID_HANDLE_VALUE){do{
        if(!wcscmp(data.cFileName,L".")||!wcscmp(data.cFileName,L".."))continue;
        if(wcslen(dir)+wcslen(data.cFileName)+2>=MAX_PATH){ok=FALSE;continue;}
        swprintf(path,MAX_PATH,L"%ls\\%ls",dir,data.cFileName);
        if(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){if(!RemoveTree(path))ok=FALSE;}
        else if(!DeleteFileW(path))ok=FALSE;
    }while(FindNextFileW(h,&data));FindClose(h);}
    return RemoveDirectoryW(dir)&&ok;
}
static BOOL Preserve(const wchar_t*n){return !_wcsnicmp(n,L"fonts/",6)||!_wcsnicmp(n,L"wallpapers/",11)||!_wcsnicmp(n,L"icone/",6);}
static BOOL Forbidden(const wchar_t*n){
    return !_wcsicmp(n,L"bootloader/nyx.ini")||!_wcsicmp(n,L"bootloader/hekate_ipl.ini")||!_wcsnicmp(n,L"bootloader/bluetooth/",21)||!_wcsnicmp(n,L"emuMMC/",7)||!_wcsnicmp(n,L"Nintendo/",9)||!_wcsnicmp(n,L"backup/",7)||!_wcsnicmp(n,L"atmosphere/",11);
}
typedef struct{const PackageFile*files;int count;BYTE*seen;wchar_t root[MAX_PATH];BOOL error;} ExtractContext;
static UINT CALLBACK CabinetCallback(PVOID user,UINT event,UINT_PTR first,UINT_PTR second){
    (void)second;ExtractContext*x=user;
    if(event==SPFILENOTIFY_NEEDNEWCABINET){x->error=TRUE;return ERROR_INVALID_DATA;}
    if(event==SPFILENOTIFY_FILEINCABINET){
        FILE_IN_CABINET_INFO_W*f=(void*)first;wchar_t normalized[MAX_PATH];wcsncpy(normalized,f->NameInCabinet,MAX_PATH-1);normalized[MAX_PATH-1]=0;
        for(wchar_t*p=normalized;*p;++p)if(*p==L'\\')*p=L'/';
        for(int i=0;i<x->count;++i)if(!wcscmp(normalized,x->files[i].name)){
            if(x->seen[i]||f->FileSize!=x->files[i].size||!SafePath(x->root,normalized,TRUE,f->FullTargetName)){x->error=TRUE;return FILEOP_ABORT;}
            x->seen[i]=1;return FILEOP_DOIT;
        }
        x->error=TRUE;return FILEOP_ABORT;
    }
    if(event==SPFILENOTIFY_FILEEXTRACTED){FILEPATHS_W*f=(void*)first;if(f->Win32Error){x->error=TRUE;return f->Win32Error;}}
    return NO_ERROR;
}
static BOOL Extract(InstallContext*c,const wchar_t*temp,const PackageFile*files,int count){
    HRSRC r=FindResourceW(NULL,MAKEINTRESOURCEW(c->variant?102:101),RT_RCDATA);if(!r)return Error(c,L"Le pack intégré est introuvable.",GetLastError());
    DWORD length=SizeofResource(NULL,r);const BYTE*bytes=LockResource(LoadResource(NULL,r));char hash[65];
    if(!bytes||!HashBytes(bytes,length,hash)||strcmp(hash,c->variant?cab8hash:cab4hash))return Error(c,L"Le pack intégré est endommagé. Téléchargez à nouveau l'installateur.",ERROR_CRC);
    wchar_t cab[MAX_PATH];swprintf(cab,MAX_PATH,L"%ls\\package.cab",temp);
    HANDLE f=CreateFileW(cab,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return Error(c,L"Impossible de préparer le pack sur le PC.",GetLastError());
    DWORD n=0;BOOL ok=WriteFile(f,bytes,length,&n,NULL)&&n==length&&FlushFileBuffers(f);CloseHandle(f);
    if(!ok)return Error(c,L"Écriture du pack temporaire impossible.",GetLastError());
    ExtractContext x={files,count,calloc(count,1),L"",FALSE};if(!x.seen)return Error(c,L"Mémoire insuffisante.",ERROR_OUTOFMEMORY);wcscpy(x.root,temp);
    ok=SetupIterateCabinetW(cab,0,CabinetCallback,&x)&&!x.error;
    for(int i=0;ok&&i<count;++i){wchar_t path[MAX_PATH];ok=x.seen[i]&&SafePath(temp,files[i].name,FALSE,path)&&HashFileMatches(path,files[i].sha256);}
    free(x.seen);if(!ok)return Error(c,L"Le contrôle des fichiers du pack a échoué.",ERROR_CRC);return TRUE;
}
static BOOL CopyChecked(InstallContext*c,const wchar_t*from,const wchar_t*to,const PackageFile*f){
    HANDLE in=CreateFileW(from,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if(in==INVALID_HANDLE_VALUE)return FALSE;
    HANDLE out=CreateFileW(to,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(out==INVALID_HANDLE_VALUE){CloseHandle(in);return FALSE;}
    BYTE*buf=malloc(262144);BOOL ok=buf!=NULL;DWORD n=0,w=0;unsigned long total=0;
    while(ok){
        if(c->cancel&&WaitForSingleObject(c->cancel,0)==WAIT_OBJECT_0){SetLastError(ERROR_CANCELLED);ok=FALSE;break;}
        if(!SameDrive(c)){SetLastError(ERROR_DEVICE_NOT_CONNECTED);ok=FALSE;break;}
        if(!ReadFile(in,buf,262144,&n,NULL)){ok=FALSE;break;}if(!n)break;
        if(!WriteFile(out,buf,n,&w,NULL)||w!=n){ok=FALSE;break;}total+=n;
    }
    if(ok)ok=FlushFileBuffers(out);
    DWORD error=GetLastError();CloseHandle(in);CloseHandle(out);free(buf);
    if(!ok){SetLastError(error);return FALSE;}
    if(total!=f->size||!HashFileMatches(to,f->sha256)){SetLastError(ERROR_CRC);return FALSE;}
    return TRUE;
}
static BOOL MoveCommit(InstallContext*c,const wchar_t*from,const wchar_t*to){
    (void)c;
#ifdef INSTALLER_TEST
    if(c->failCommit&&++c->commitOps==c->failCommit){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
#endif
    return MoveFileExW(from,to,MOVEFILE_WRITE_THROUGH);
}
static BOOL WriteReport(InstallContext*c,const PackageFile*files,int count,const BYTE*changed){
    wchar_t path[MAX_PATH];if(!SafePath(c->backup,L"installation.txt",TRUE,path))return FALSE;
    HANDLE out=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(out==INVALID_HANDLE_VALUE)return FALSE;
    const char*header="Hekate Nova Pro v34 alpha5 - Installer 1.2\r\nFinal verification successful. COPIE=copied, CONSERVE=existing asset kept, IDENTIQUE=already current.\r\n";
    DWORD n=0;BOOL ok=WriteFile(out,header,(DWORD)strlen(header),&n,NULL)&&n==strlen(header);
    for(int i=0;ok&&i<count;++i){
        char name[512],line[700];
        if(!WideCharToMultiByte(CP_UTF8,0,files[i].name,-1,name,sizeof name,NULL,NULL)){ok=FALSE;break;}
        int length=snprintf(line,sizeof line,"%s  %s\r\n",changed[i]==2?"COPIE":Preserve(files[i].name)?"CONSERVE":"IDENTIQUE",name);
        ok=length>0&&WriteFile(out,line,length,&n,NULL)&&n==(DWORD)length;
    }
    if(ok)ok=FlushFileBuffers(out);
    CloseHandle(out);return ok;
}
BOOL InstallPackage(InstallContext*c){
    int count=0;const PackageFile*files=PackageFiles(c->variant,&count);BYTE *plan=calloc(count,1),*changed=calloc(count,1);
    wchar_t tempBase[MAX_PATH],temp[MAX_PATH]=L"",stage[MAX_PATH]=L"",backupRoot[MAX_PATH],dest[MAX_PATH],src[MAX_PATH],staged[MAX_PATH],old[MAX_PATH];
    BOOL ok=FALSE,committing=FALSE;ULONGLONG needed=2*1024*1024;DWORD failure=0;
    c->copied=c->preserved=c->unchanged=0;c->backup[0]=c->error[0]=0;c->rollbackIncomplete=FALSE;
    if(!plan||!changed){Error(c,L"Mémoire insuffisante.",ERROR_OUTOFMEMORY);goto out;}
    if(!ValidateDrive(c))goto out;
    for(int i=0;i<count;++i){
        if(Forbidden(files[i].name)||!SafePath(c->root,files[i].name,FALSE,dest)){Error(c,L"Un chemin de destination est protégé ou contient un lien. Installation arrêtée.",ERROR_ACCESS_DENIED);goto out;}
        if(GetFileAttributesW(dest)!=INVALID_FILE_ATTRIBUTES){
            if(Preserve(files[i].name)){++c->preserved;continue;}
            if(HashFileMatches(dest,files[i].sha256)){++c->unchanged;continue;}
        }
        plan[i]=1;needed+=files[i].size;
    }
    ULARGE_INTEGER freeBytes;if(!GetDiskFreeSpaceExW(c->root,&freeBytes,NULL,NULL)||freeBytes.QuadPart<needed){Error(c,L"Espace libre insuffisant sur la carte SD (environ 40 Mo nécessaires).",ERROR_DISK_FULL);goto out;}
    Report(c,0,count*2,L"Préparation et vérification du pack intégré…",FALSE);
    DWORD tempLength=GetTempPathW(MAX_PATH,tempBase);
    if(!tempLength||tempLength>=MAX_PATH||!PrivateDir(tempBase,L"HekateNova-",temp)){Error(c,L"Impossible de préparer le dossier temporaire du PC.",GetLastError());goto out;}
    if(!Extract(c,temp,files,count))goto out;
    if(!SameDrive(c)){Error(c,L"La carte a été retirée ou remplacée.",ERROR_DEVICE_NOT_CONNECTED);goto out;}
    if(!PrivateDir(c->root,L".nova-stage-",stage)){Error(c,L"Impossible d'écrire sur la carte SD.",GetLastError());goto out;}
    for(int i=0;i<count;++i){
        if(!plan[i])continue;
        Report(c,i,count*2,files[i].name,FALSE);
        if(c->cancel&&WaitForSingleObject(c->cancel,0)==WAIT_OBJECT_0){Error(c,L"Installation annulée avant le remplacement des fichiers.",ERROR_CANCELLED);goto out;}
#ifdef INSTALLER_TEST
        if(c->failStage==i+1){Error(c,L"Erreur de copie simulée.",ERROR_DISK_FULL);goto out;}
#endif
        if(!SafePath(temp,files[i].name,FALSE,src)||!SafePath(stage,files[i].name,TRUE,staged)||!CopyChecked(c,src,staged,&files[i])){failure=GetLastError();Error(c,L"La copie ou sa vérification a échoué. Les fichiers installés n'ont pas été remplacés.",failure);goto out;}
    }
    if(c->cancel&&WaitForSingleObject(c->cancel,0)==WAIT_OBJECT_0){Error(c,L"Installation annulée avant le remplacement des fichiers.",ERROR_CANCELLED);goto out;}
    /* Once this phase starts, finish or roll back; do not interrupt on UI cancel. */
    committing=TRUE;Report(c,count,count*2,L"Finalisation et sauvegarde des fichiers remplacés…",TRUE);
    if(!SafePath(c->root,L"nova-backups/placeholder",TRUE,dest)){Error(c,L"Impossible de préparer la sauvegarde des fichiers remplacés.",GetLastError());goto rollback;}
    swprintf(backupRoot,MAX_PATH,L"%lsnova-backups",c->root);
#ifdef INSTALLER_TEST
    if(c->fixture)swprintf(backupRoot,MAX_PATH,L"%ls\\nova-backups",c->root);
#endif
    SYSTEMTIME time;GetLocalTime(&time);wchar_t prefix[64];swprintf(prefix,64,L"%04u%02u%02u-%02u%02u%02u-",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);
    if(!PrivateDir(backupRoot,prefix,c->backup)){Error(c,L"Impossible de créer la sauvegarde.",GetLastError());goto rollback;}
    for(int i=0;i<count;++i){
        if(!plan[i])continue;
        Report(c,count+i,count*2,files[i].name,TRUE);
        if(!SameDrive(c)||!SafePath(c->root,files[i].name,TRUE,dest)||!SafePath(stage,files[i].name,FALSE,staged)||!SafePath(c->backup,files[i].name,TRUE,old)){Error(c,L"La carte ou un chemin a changé pendant l'installation.",ERROR_DEVICE_NOT_CONNECTED);goto rollback;}
        if(GetFileAttributesW(dest)!=INVALID_FILE_ATTRIBUTES){
            if(Preserve(files[i].name)){++c->preserved;continue;}
            if(!MoveCommit(c,dest,old)){Error(c,L"Impossible de sauvegarder un fichier existant.",GetLastError());goto rollback;}changed[i]=1;
        }
        if(!MoveCommit(c,staged,dest)){Error(c,L"Impossible de remplacer un fichier. Restauration en cours.",GetLastError());goto rollback;}
        changed[i]=2;++c->copied;
    }
    Report(c,count*2-1,count*2,L"Contrôle final des fichiers sur la carte SD…",TRUE);
#ifdef INSTALLER_TEST
    if(c->failFinalVerify>0&&c->failFinalVerify<=count&&SafePath(c->root,files[c->failFinalVerify-1].name,FALSE,dest))DeleteFileW(dest);
#endif
    for(int i=0;i<count;++i){
        if(!SameDrive(c)||!SafePath(c->root,files[i].name,FALSE,dest)||GetFileAttributesW(dest)==INVALID_FILE_ATTRIBUTES){
            Error(c,L"Un fichier manque après la copie. Installation non validée ; restauration en cours.",ERROR_FILE_NOT_FOUND);goto rollback;
        }
        if(!(Preserve(files[i].name)&&changed[i]==0)&&!HashFileMatches(dest,files[i].sha256)){
            Error(c,L"Un fichier n'est pas conforme après la copie. Installation non validée ; restauration en cours.",ERROR_CRC);goto rollback;
        }
    }
    if(!WriteReport(c,files,count,changed)){Error(c,L"Impossible d'enregistrer le rapport d'installation.",GetLastError());goto rollback;}
    Report(c,count*2,count*2,L"Installation terminée. Les fichiers ont été vérifiés.",TRUE);ok=TRUE;goto out;
rollback:
    for(int i=count-1;i>=0;--i){
        if(!changed[i])continue;
        if(!SameDrive(c)||!SafePath(c->root,files[i].name,TRUE,dest)||!SafePath(c->backup,files[i].name,FALSE,old)){c->rollbackIncomplete=TRUE;continue;}
        if(changed[i]==2&&GetFileAttributesW(dest)!=INVALID_FILE_ATTRIBUTES){
            if(!HashFileMatches(dest,files[i].sha256)||!DeleteFileW(dest)){c->rollbackIncomplete=TRUE;continue;}
        }
        if(GetFileAttributesW(old)!=INVALID_FILE_ATTRIBUTES&&!MoveFileExW(old,dest,MOVEFILE_WRITE_THROUGH))c->rollbackIncomplete=TRUE;
    }
    c->copied=0;
out:
    if(stage[0]&&SameDrive(c))RemoveTree(stage);
    if(temp[0])RemoveTree(temp);
    if(!ok&&committing&&c->rollbackIncomplete)wcscat(c->error,L"\r\nRestauration incomplète : conservez le dossier nova-backups pour récupérer les fichiers.");
    free(plan);free(changed);return ok;
}
