/* Native Windows installer, no extra runtime or download required. */
#include "install.h"
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <wctype.h>
#define WM_PROGRESS (WM_APP+1)
#define WM_DONE (WM_APP+2)
static HWND win,drives,refresh,standard,eight,install,cancel,details,status,progress;
static HFONT font,small,title;static HBRUSH background,field;static HANDLE thread;static InstallContext job;
static wchar_t roots[26][4];static int driveCount;static BOOL running,committing;static int result;
static COLORREF bg=RGB(16,21,29),ink=RGB(242,245,250),muted=RGB(163,179,198),cyan=RGB(85,201,241);
static HWND Control(const wchar_t*cls,const wchar_t*text,DWORD style,int x,int y,int w,int h,int id){
    HWND c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,x,y,w,h,win,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);SendMessageW(c,WM_SETFONT,(WPARAM)font,TRUE);return c;
}
static void ShowDestination(void){
    int i=(int)SendMessageW(drives,CB_GETCURSEL,0,0);BOOL selected=i>=0&&i<driveCount;
    EnableWindow(install,selected&&!running);
    if(!selected){SetWindowTextW(details,L"Choisissez la lettre correspondant à votre carte SD.");SetWindowTextW(install,L"Installer Nova");return;}
    wchar_t text[512],fs[64]=L"";ULARGE_INTEGER freeBytes,total;
    if(GetDiskFreeSpaceExW(roots[i],&freeBytes,&total,NULL)&&GetVolumeInformationW(roots[i],NULL,0,NULL,NULL,NULL,fs,64)){
        swprintf(text,512,L"%ls  ·  %ls  ·  %.1f Go libres sur %.1f Go — vérifiez qu'il s'agit de votre carte SD.",roots[i],fs,freeBytes.QuadPart/1073741824.0,total.QuadPart/1073741824.0);SetWindowTextW(details,text);
    }
    swprintf(text,512,L"Installer Nova sur %lc:",roots[i][0]);SetWindowTextW(install,text);InvalidateRect(install,NULL,TRUE);
}
static void RefreshDrives(void){
    SendMessageW(drives,CB_RESETCONTENT,0,0);driveCount=0;wchar_t letters[512],windows[MAX_PATH];GetWindowsDirectoryW(windows,MAX_PATH);
    GetLogicalDriveStringsW(512,letters);
    for(wchar_t*p=letters;*p;p+=wcslen(p)+1){
        UINT type=GetDriveTypeW(p);if((type!=DRIVE_REMOVABLE&&type!=DRIVE_FIXED)||towupper(p[0])==towupper(windows[0]))continue;
        wchar_t label[96]=L"",fs[64],text[256];if(!GetVolumeInformationW(p,label,96,NULL,NULL,NULL,fs,64))continue;
        if(_wcsicmp(fs,L"FAT32")&&_wcsicmp(fs,L"exFAT"))continue;
        wcsncpy(roots[driveCount],p,4);swprintf(text,256,L"%lc:   %ls   (%ls)",p[0],label[0]?label:L"Sans nom",fs);
        SendMessageW(drives,CB_ADDSTRING,0,(LPARAM)text);++driveCount;
    }
    SendMessageW(drives,CB_SETCURSEL,(WPARAM)-1,0);ShowDestination();
    if(!driveCount)SetWindowTextW(details,L"Aucune carte FAT32/exFAT disponible. Branchez le lecteur SD puis cliquez sur Actualiser.");
}
static void Update(int pos,int maximum,const wchar_t*text,BOOL commit,void*user){
    (void)user;wchar_t*copy=_wcsdup(text);if(copy)PostMessageW(win,WM_PROGRESS,MAKELONG(pos,maximum),(LPARAM)copy);
    if(commit)PostMessageW(win,WM_PROGRESS,(WPARAM)-1,0);
}
static DWORD WINAPI Worker(void*arg){(void)arg;result=InstallPackage(&job);PostMessageW(win,WM_DONE,0,0);return 0;}
static void Start(void){
    int i=(int)SendMessageW(drives,CB_GETCURSEL,0,0);if(running||i<0||i>=driveCount)return;
    memset(&job,0,sizeof job);wcscpy(job.root,roots[i]);job.variant=SendMessageW(eight,BM_GETCHECK,0,0)==BST_CHECKED;
    if(!ValidateDrive(&job)){MessageBoxW(win,job.error,L"Destination à vérifier",MB_ICONWARNING);return;}
    job.cancel=CreateEventW(NULL,TRUE,FALSE,NULL);job.progress=Update;
    if(!job.cancel){MessageBoxW(win,L"Impossible de préparer l'installation.",L"Installation",MB_ICONERROR);return;}
    running=TRUE;committing=FALSE;EnableWindow(drives,FALSE);EnableWindow(refresh,FALSE);EnableWindow(standard,FALSE);EnableWindow(eight,FALSE);EnableWindow(install,FALSE);SetWindowTextW(cancel,L"Annuler");
    SetWindowTextW(status,L"Vérification des fichiers et préparation de la copie…");SendMessageW(progress,PBM_SETPOS,0,0);
    result=FALSE;thread=CreateThread(NULL,0,Worker,NULL,0,NULL);
    if(!thread){running=FALSE;CloseHandle(job.cancel);job.cancel=NULL;PostMessageW(win,WM_DONE,0,0);}
}
static void Close(void){
    if(running){if(!committing){SetEvent(job.cancel);SetWindowTextW(status,L"Annulation demandée. Attendez la fin du nettoyage…");EnableWindow(cancel,FALSE);}return;}
    DestroyWindow(win);
}
static void DrawBrand(HDC dc){
    HPEN pen=CreatePen(PS_SOLID,2,cyan);HGDIOBJ old=SelectObject(dc,pen),brush=SelectObject(dc,GetStockObject(NULL_BRUSH));
    POINT hex[]={{53,26},{77,40},{77,68},{53,82},{29,68},{29,40},{53,26}};Polyline(dc,hex,7);
    POINT n[]={{43,65},{43,43},{63,65},{63,43}};Polyline(dc,n,4);SelectObject(dc,brush);SelectObject(dc,old);DeleteObject(pen);
}
static LRESULT CALLBACK WindowProc(HWND h,UINT msg,WPARAM w,LPARAM l){
    switch(msg){
    case WM_CREATE:{
        win=h;background=CreateSolidBrush(bg);field=CreateSolidBrush(RGB(28,38,51));
        font=CreateFontW(-19,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        small=CreateFontW(-16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        title=CreateFontW(-29,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        HWND label=Control(L"STATIC",L"HEKATE NOVA PRO",0,100,26,700,42,0);SendMessageW(label,WM_SETFONT,(WPARAM)title,TRUE);
        Control(L"STATIC",L"Installation sur carte SD  ·  v34 alpha 7  ·  installateur 1.4",0,102,72,710,28,0);
        Control(L"STATIC",L"1   Choisissez votre carte SD",0,28,130,790,28,0);
        drives=Control(L"COMBOBOX",L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL|WS_TABSTOP,28,170,628,220,10);SendMessageW(drives,CB_SETITEMHEIGHT,(WPARAM)-1,32);SendMessageW(drives,CB_SETITEMHEIGHT,0,32);
        refresh=Control(L"BUTTON",L"Actualiser",BS_OWNERDRAW|WS_TABSTOP,674,170,158,36,11);
        details=Control(L"STATIC",L"",0,28,218,804,48,0);SendMessageW(details,WM_SETFONT,(WPARAM)small,TRUE);
        Control(L"STATIC",L"2   Choisissez le pack",0,28,284,790,28,0);
        standard=Control(L"BUTTON",L"Standard — console avec 4 Go de RAM",BS_AUTORADIOBUTTON|WS_GROUP|WS_TABSTOP,28,322,800,32,12);SendMessageW(standard,BM_SETCHECK,BST_CHECKED,0);
        eight=Control(L"BUTTON",L"8 Go expérimental — console physiquement modifiée",BS_AUTORADIOBUTTON|WS_TABSTOP,28,360,800,32,13);
        label=Control(L"STATIC",L"Le pack standard convient aux consoles V1, V2, Lite et OLED non modifiées en RAM.",0,54,400,778,24,0);SendMessageW(label,WM_SETFONT,(WPARAM)small,TRUE);
        label=Control(L"STATIC",L"Vos réglages, appairages, emuMMC et fichiers personnels sont conservés.\r\nLes fonds, icônes et polices existants sont gardés ; les fichiers remplacés sont sauvegardés.",0,28,450,804,54,0);SendMessageW(label,WM_SETFONT,(WPARAM)small,TRUE);
        progress=Control(PROGRESS_CLASSW,L"",PBS_SMOOTH,28,520,804,18,0);SendMessageW(progress,PBM_SETBARCOLOR,0,cyan);SendMessageW(progress,PBM_SETBKCOLOR,0,RGB(38,49,63));
        status=Control(L"STATIC",L"Cet installateur copie Nova. Il ne partitionne pas et ne formate pas la carte.",0,28,550,804,56,0);SendMessageW(status,WM_SETFONT,(WPARAM)small,TRUE);
        cancel=Control(L"BUTTON",L"Fermer",BS_OWNERDRAW|WS_TABSTOP,28,620,180,48,15);
        install=Control(L"BUTTON",L"Installer Nova",BS_OWNERDRAW|WS_TABSTOP,466,620,366,48,14);
        label=Control(L"STATIC",L"Pack intégré  ·  Aucun téléchargement nécessaire",0,28,680,804,20,0);SendMessageW(label,WM_SETFONT,(WPARAM)small,TRUE);
        RefreshDrives();return 0;
    }
    case WM_COMMAND:
        if(LOWORD(w)==10&&HIWORD(w)==CBN_SELCHANGE)ShowDestination();
        else if(LOWORD(w)==11&&!running)RefreshDrives();
        else if(LOWORD(w)==14)Start();else if(LOWORD(w)==15)Close();
        return 0;
    case WM_DEVICECHANGE:if(!running)RefreshDrives();return TRUE;
    case WM_PROGRESS:
        if(w==(WPARAM)-1){committing=TRUE;EnableWindow(cancel,FALSE);return 0;}
        SendMessageW(progress,PBM_SETRANGE32,0,HIWORD(w));SendMessageW(progress,PBM_SETPOS,LOWORD(w),0);if(l){SetWindowTextW(status,(wchar_t*)l);free((void*)l);}return 0;
    case WM_DONE:{
        running=FALSE;committing=FALSE;if(thread){CloseHandle(thread);thread=NULL;}if(job.cancel){CloseHandle(job.cancel);job.cancel=NULL;}
        EnableWindow(drives,TRUE);EnableWindow(refresh,TRUE);EnableWindow(standard,TRUE);EnableWindow(eight,TRUE);EnableWindow(cancel,TRUE);SetWindowTextW(cancel,L"Fermer");ShowDestination();
        if(result){wchar_t text[1400];swprintf(text,1400,L"Nova est installée sur %ls\r\n%d fichier(s) copié(s), %d élément(s) personnalisé(s) conservé(s).\r\nSauvegarde des fichiers remplacés :\r\n%ls\r\n\r\nÉjectez la carte avec Windows avant de la retirer. Le payload Nova est présent à la racine et dans bootloader/payloads. Le rapport installation.txt se trouve dans la sauvegarde.",job.root,job.copied,job.preserved,job.backup);SetWindowTextW(status,L"Installation terminée. Vous pouvez éjecter la carte SD avec Windows.");MessageBoxW(win,text,L"Installation terminée",MB_OK|MB_ICONINFORMATION);}
        else{SetWindowTextW(status,L"Installation arrêtée. Consultez le détail affiché.");MessageBoxW(win,job.error[0]?job.error:L"Impossible de démarrer la copie.",L"Installation arrêtée",MB_OK|MB_ICONWARNING);}return 0;
    }
    case WM_CTLCOLORSTATIC:case WM_CTLCOLORBTN:case WM_CTLCOLORLISTBOX:case WM_CTLCOLOREDIT:
        SetTextColor((HDC)w,ink);SetBkColor((HDC)w,bg);return (LRESULT)background;
    case WM_DRAWITEM:{
        DRAWITEMSTRUCT*d=(void*)l;wchar_t text[512];RECT r=d->rcItem;
        if(d->CtlType==ODT_COMBOBOX){
            FillRect(d->hDC,&r,field);if(d->itemID==(UINT)-1)wcscpy(text,L"Sélectionnez la lettre de la carte SD…");else SendMessageW(drives,CB_GETLBTEXT,d->itemID,(LPARAM)text);
            SetTextColor(d->hDC,ink);SetBkMode(d->hDC,TRANSPARENT);SelectObject(d->hDC,font);r.left+=12;DrawTextW(d->hDC,text,-1,&r,DT_SINGLELINE|DT_VCENTER);return TRUE;
        }
        GetWindowTextW(d->hwndItem,text,512);BOOL primary=d->hwndItem==install,disabled=d->itemState&ODS_DISABLED;HBRUSH b=CreateSolidBrush(primary&&!disabled?cyan:RGB(28,38,51));FillRect(d->hDC,&r,b);DeleteObject(b);
        HPEN p=CreatePen(PS_SOLID,1,RGB(63,80,98));HGDIOBJ old=SelectObject(d->hDC,p),oldB=SelectObject(d->hDC,GetStockObject(NULL_BRUSH));Rectangle(d->hDC,r.left,r.top,r.right,r.bottom);SelectObject(d->hDC,oldB);SelectObject(d->hDC,old);DeleteObject(p);
        SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,disabled?muted:primary?bg:ink);SelectObject(d->hDC,font);DrawTextW(d->hDC,text,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);if(d->itemState&ODS_FOCUS){InflateRect(&r,-4,-4);DrawFocusRect(d->hDC,&r);}return TRUE;
    }
    case WM_PRINTCLIENT:{RECT area;GetClientRect(h,&area);FillRect((HDC)w,&area,background);DrawBrand((HDC)w);return 0;}
    case WM_PAINT:{PAINTSTRUCT p;HDC dc=BeginPaint(h,&p);DrawBrand(dc);EndPaint(h,&p);return 0;}
    case WM_CLOSE:Close();return 0;
    case WM_DESTROY:DeleteObject(background);DeleteObject(field);DeleteObject(font);DeleteObject(small);DeleteObject(title);PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(h,msg,w,l);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show){
    (void)previous;(void)command;INITCOMMONCONTROLSEX cc={sizeof cc,ICC_PROGRESS_CLASS|ICC_STANDARD_CLASSES};InitCommonControlsEx(&cc);
    WNDCLASSW cls={0};cls.hInstance=instance;cls.lpfnWndProc=WindowProc;cls.lpszClassName=L"HekateNovaInstaller";cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));cls.hbrBackground=CreateSolidBrush(bg);RegisterClassW(&cls);
    RECT r={0,0,860,712};DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;AdjustWindowRect(&r,style,FALSE);
    win=CreateWindowExW(0,cls.lpszClassName,L"Hekate Nova Pro — installation v34 alpha 7 · installateur 1.4",style,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,NULL,NULL,instance,NULL);if(!win)return 1;
#ifdef INSTALLER_PREVIEW
    /* Off-screen rendering; does not open or install to any SD. */
    (void)show;
    RECT outer;GetWindowRect(win,&outer);int width=outer.right-outer.left,height=outer.bottom-outer.top;
    HDC dc=GetDC(win),memory=CreateCompatibleDC(dc);HBITMAP bitmap=CreateCompatibleBitmap(dc,width,height);SelectObject(memory,bitmap);
    SendMessageW(win,WM_PRINT,(WPARAM)memory,PRF_CLIENT|PRF_CHILDREN|PRF_ERASEBKGND);
    BITMAPINFOHEADER head={sizeof head,width,height,1,32,BI_RGB,0,0,0,0,0};BYTE *pixels=calloc(width*height,4);GetDIBits(dc,bitmap,0,height,pixels,(BITMAPINFO*)&head,DIB_RGB_COLORS);
    FILE*f=_wfopen(command,L"wb");if(!f)return 2;BITMAPFILEHEADER file={0x4D42,sizeof file+sizeof head+width*height*4,0,0,sizeof file+sizeof head};fwrite(&file,sizeof file,1,f);fwrite(&head,sizeof head,1,f);fwrite(pixels,width*height*4,1,f);fclose(f);free(pixels);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(win,dc);DestroyWindow(win);return 0;
#else
    ShowWindow(win,show);UpdateWindow(win);MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){if(!IsDialogMessageW(win,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}return (int)msg.wParam;
#endif
}
