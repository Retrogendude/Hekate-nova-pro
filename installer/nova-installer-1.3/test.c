#include "install.h"
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
static unsigned checks;static InstallContext *last;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);if(last)fprintf(stderr,"DETAIL: %ls\n",last->error);return 1;}}while(0)
static void WriteFixture(const wchar_t*root,const wchar_t*name,const char*text){
    wchar_t p[MAX_PATH];if(!SafePath(root,name,TRUE,p))abort();HANDLE f=CreateFileW(p,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)abort();DWORD n;WriteFile(f,text,(DWORD)strlen(text),&n,NULL);CloseHandle(f);
}
static BOOL Equals(const wchar_t*root,const wchar_t*name,const char*text){
    wchar_t p[MAX_PATH];if(!SafePath(root,name,FALSE,p))return FALSE;HANDLE f=CreateFileW(p,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);if(f==INVALID_HANDLE_VALUE)return FALSE;char buf[2048];DWORD n=0;BOOL ok=ReadFile(f,buf,2047,&n,NULL);buf[n]=0;CloseHandle(f);return ok&&!strcmp(buf,text);
}
static BOOL Missing(const wchar_t*root,const wchar_t*name){wchar_t p[MAX_PATH];return SafePath(root,name,FALSE,p)&&GetFileAttributesW(p)==INVALID_FILE_ATTRIBUTES;}
static BOOL Has(const wchar_t*root,const wchar_t*name,const char*fragment){
    wchar_t p[MAX_PATH];if(!SafePath(root,name,FALSE,p))return FALSE;
    HANDLE f=CreateFileW(p,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);if(f==INVALID_HANDLE_VALUE)return FALSE;
    char b[65536];DWORD n=0;BOOL ok=ReadFile(f,b,sizeof b-1,&n,NULL);b[n]=0;CloseHandle(f);return ok&&strstr(b,fragment)!=NULL;
}
static void Context(InstallContext*c,const wchar_t*base,const wchar_t*name,int variant){memset(c,0,sizeof*c);last=c;swprintf(c->root,MAX_PATH,L"%ls\\%ls",base,name);CreateDirectoryW(c->root,NULL);c->fixture=TRUE;c->variant=variant;}
int wmain(int argc,wchar_t**argv){
    C(argc==2);wchar_t absolute[MAX_PATH];C(GetFullPathNameW(argv[1],MAX_PATH,absolute,NULL)>0);C(GetFileAttributesW(absolute)!=INVALID_FILE_ATTRIBUTES);
    C(ValidRelative(L"bootloader/sys/nyx.bin"));C(!ValidRelative(L"../outside"));C(!ValidRelative(L"foo/../bar"));C(!ValidRelative(L"C:/Windows/a"));C(!ValidRelative(L"/outside"));C(!ValidRelative(L"\\\\server\\a"));C(!ValidRelative(L"foo//bar"));C(!ValidRelative(L"foo/bar."));C(!ValidRelative(L"foo/a:b"));
    InstallContext c={0};wcscpy(c.root,L"C:\\");C(!ValidateDrive(&c));wcscpy(c.root,L"C:\\Windows\\");C(!ValidateDrive(&c));
    Context(&c,absolute,L"standard",0);
    WriteFixture(c.root,L"bootloader/nyx.ini","USER SETTINGS");WriteFixture(c.root,L"bootloader/hekate_ipl.ini","USER BOOT");WriteFixture(c.root,L"bootloader/bluetooth/pairing.bin","USER PAIRING");
    WriteFixture(c.root,L"emuMMC/emummc.ini","USER EMUMMC");WriteFixture(c.root,L"Nintendo/save.dat","USER SAVE");WriteFixture(c.root,L"fonts/Lecture.nyxf","USER FONT");WriteFixture(c.root,L"wallpapers/Nova-Carbon-Cyan.bmp","USER WALL");WriteFixture(c.root,L"icone/Launcher.png","USER ICON");WriteFixture(c.root,L"bootloader/sys/nyx.bin","OLD NYX");
    WriteFixture(c.root,L"bootloader/payloads/User-Tool.bin","PERSONAL PAYLOAD");
    C(InstallPackage(&c));C(c.copied>100&&c.preserved==3);
    C(Equals(c.root,L"bootloader/payloads/User-Tool.bin","PERSONAL PAYLOAD"));
    C(Has(c.backup,L"installation.txt","bootloader/payloads/hekate_nova_pro_6.5.5_v34_alpha6.bin"));
    C(Equals(c.root,L"bootloader/nyx.ini","USER SETTINGS"));C(Equals(c.root,L"bootloader/hekate_ipl.ini","USER BOOT"));C(Equals(c.root,L"bootloader/bluetooth/pairing.bin","USER PAIRING"));C(Equals(c.root,L"emuMMC/emummc.ini","USER EMUMMC"));C(Equals(c.root,L"Nintendo/save.dat","USER SAVE"));
    C(Equals(c.root,L"fonts/Lecture.nyxf","USER FONT"));C(Equals(c.root,L"wallpapers/Nova-Carbon-Cyan.bmp","USER WALL"));C(Equals(c.root,L"icone/Launcher.png","USER ICON"));C(Equals(c.backup,L"bootloader/sys/nyx.bin","OLD NYX"));
    int count;const PackageFile*files=PackageFiles(0,&count);for(int i=0;i<count;++i){if(!wcscmp(files[i].name,L"fonts/Lecture.nyxf")||!wcscmp(files[i].name,L"wallpapers/Nova-Carbon-Cyan.bmp")||!wcscmp(files[i].name,L"icone/Launcher.png"))continue;wchar_t p[MAX_PATH];C(SafePath(c.root,files[i].name,FALSE,p)&&HashFileMatches(p,files[i].sha256));}
    C(count==167);
    C(InstallPackage(&c));C(c.copied==0&&c.preserved==16);C(Equals(c.root,L"bootloader/nyx.ini","USER SETTINGS"));
    Context(&c,absolute,L"eight",1);C(InstallPackage(&c));files=PackageFiles(1,&count);for(int i=0;i<count;++i){wchar_t p[MAX_PATH];C(SafePath(c.root,files[i].name,FALSE,p)&&HashFileMatches(p,files[i].sha256));}
    Context(&c,absolute,L"rollback",0);WriteFixture(c.root,L"bootloader/sys/nyx.bin","OLD NYX");WriteFixture(c.root,L"bootloader/nyx.ini","USER SETTINGS");files=PackageFiles(0,&count);int index=0;while(index<count&&wcscmp(files[index].name,L"bootloader/sys/nyx.bin"))++index;C(index<count);c.failCommit=index+2;
    C(!InstallPackage(&c));C(!c.rollbackIncomplete);C(Equals(c.root,L"bootloader/sys/nyx.bin","OLD NYX"));C(Equals(c.root,L"bootloader/nyx.ini","USER SETTINGS"));C(Missing(c.root,L"BUILD-INFO.json"));C(Missing(c.root,L"bootloader/update.bin"));
    Context(&c,absolute,L"missing-after-copy",0);WriteFixture(c.root,L"bootloader/sys/nyx.bin","OLD NYX");c.failFinalVerify=index+1;
    C(!InstallPackage(&c));C(!c.rollbackIncomplete);C(wcsstr(c.error,L"manque")!=NULL);C(Equals(c.root,L"bootloader/sys/nyx.bin","OLD NYX"));C(Missing(c.root,L"BUILD-INFO.json"));
    Context(&c,absolute,L"stage-failure",0);WriteFixture(c.root,L"bootloader/sys/nyx.bin","OLD NYX");c.failStage=2;C(!InstallPackage(&c));C(Equals(c.root,L"bootloader/sys/nyx.bin","OLD NYX"));C(Missing(c.root,L"BUILD-INFO.json"));
    Context(&c,absolute,L"cancel",0);WriteFixture(c.root,L"bootloader/sys/nyx.bin","OLD NYX");c.cancel=CreateEventW(NULL,TRUE,TRUE,NULL);C(!InstallPackage(&c));C(Equals(c.root,L"bootloader/sys/nyx.bin","OLD NYX"));C(Missing(c.root,L"BUILD-INFO.json"));CloseHandle(c.cancel);
    Context(&c,absolute,L"links",0);C(!InstallPackage(&c));C(Missing(c.root,L"BUILD-INFO.json"));
    wchar_t sentinel[MAX_PATH];swprintf(sentinel,MAX_PATH,L"%ls\\sentinel",absolute);C(Equals(sentinel,L"keep.txt","EXTERNAL SENTINEL"));
    printf("PASS: %u native assertions; real embedded CAB extraction, exact standard/8GB hashes, custom files preserved, backups, reinstall, commit rollback, staging failure, cancel, system-volume/path/junction refusal. No physical SD written.\n",checks);return 0;
}
