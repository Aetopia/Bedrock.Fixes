#define INITGUID
#define COBJMACROS
#define _MINAPPMODEL_H_
#define WIDL_C_INLINE_WRAPPERS

#include <windows.h>
#include <appmodel.h>
#include <shobjidl.h>

VOID WinMainCRTStartup()
{
    CoInitialize(NULL);

    IPackageDebugSettings *pSettings = {};
    CoCreateInstance(&CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, &IID_IPackageDebugSettings,
                     (PVOID *)&pSettings);

    WCHAR szPackage[PACKAGE_FULL_NAME_MAX_LENGTH + 1] = {};

    GetPackagesByPackageFamily(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe", &(UINT){PACKAGE_GRAPH_MIN_SIZE}, &(PWSTR){},
                               &((UINT32){PACKAGE_FULL_NAME_MAX_LENGTH + 1}), szPackage);

    IPackageDebugSettings_EnableDebugging(pSettings, szPackage, NULL, NULL);

    GetPackagesByPackageFamily(L"Microsoft.MinecraftWindowsBeta_8wekyb3d8bbwe", &(UINT){PACKAGE_GRAPH_MIN_SIZE}, &(PWSTR){},
                               &((UINT32){PACKAGE_FULL_NAME_MAX_LENGTH + 1}), szPackage);

    IPackageDebugSettings_EnableDebugging(pSettings, szPackage, NULL, NULL);

    IPackageDebugSettings_Release(pSettings);

    ExitProcess(EXIT_SUCCESS);
}