#ifndef NOVA_INSTALL_H
#define NOVA_INSTALL_H
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdint.h>
typedef struct { const wchar_t *name; unsigned long size; const char *sha256; } PackageFile;
typedef void (*Progress)(int position,int maximum,const wchar_t *text,BOOL committing,void *user);
typedef struct {
    wchar_t root[MAX_PATH],guid[MAX_PATH],backup[MAX_PATH],error[1400];
    DWORD serial; int variant,copied,preserved,unchanged; BOOL rollbackIncomplete;
    HANDLE cancel; Progress progress; void *user;
#ifdef INSTALLER_TEST
    BOOL fixture; int failCommit,commitOps,failStage,failFinalVerify;
#endif
} InstallContext;
BOOL ValidRelative(const wchar_t *name);
BOOL SafePath(const wchar_t *root,const wchar_t *name,BOOL parents,wchar_t *out);
BOOL ValidateDrive(InstallContext *c);
BOOL InstallPackage(InstallContext *c);
BOOL HashFileMatches(const wchar_t *path,const char *hex);
const PackageFile *PackageFiles(int variant,int *count);
#endif
