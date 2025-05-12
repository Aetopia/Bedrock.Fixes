#define INITGUID
#define COBJMACROS
#define WIDL_C_INLINE_WRAPPERS
#define WIDL_using_Windows_UI_Core
#define WIDL_using_Windows_Foundation
#define WIDL_using_Windows_ApplicationModel_Core

#include <roapi.h>
#include <MinHook.h>
#include <appmodel.h>
#include <winstring.h>
#include <windows.applicationmodel.core.h>

HRESULT (*__put_PointerCursor)(ICoreWindow *, LPUNKNOWN) = {};

PVOID __wrap_memcpy(PVOID Destination, PVOID Source, SIZE_T Count)
{
    __movsb(Destination, Source, Count);
    return Destination;
}

PVOID __wrap_memset(PVOID Destination, BYTE Data, SIZE_T Count)
{
    __stosb(Destination, Data, Count);
    return Destination;
}

HRESULT _put_PointerCursor(PVOID This, PVOID value)
{
    ICoreCursor *pCursor = {};
    ICoreWindow_get_PointerCursor(This, &pCursor);

    if (!pCursor || !value)
    {
        Rect rcClient = {};
        PVOID pWindow = {};

        ICoreWindow_get_Bounds(This, &rcClient);
        ICoreWindow_QueryInterface(This, &IID_ICoreWindow2, &pWindow);

        ICoreWindow2_put_PointerPosition(pWindow,
                                         (Point){rcClient.X + rcClient.Width / 2, rcClient.Y + rcClient.Height / 2});
        ICoreWindow2_Release(pWindow);
    }

    if (pCursor)
        ICoreCursor_Release(pCursor);
    return __put_PointerCursor(This, value);
}

DWORD ThreadProc(PVOID pParameter)
{
    RoInitialize(RO_INIT_MULTITHREADED);

    HSTRING hString = {};
    WindowsCreateString(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication,
                        lstrlenW(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication), &hString);

    ICoreImmersiveApplication *pApplication = {};
    RoGetActivationFactory(hString, &IID_ICoreImmersiveApplication, (PVOID *)&pApplication);
    WindowsDeleteString(hString);

    ICoreApplicationView *pView = {};
    ICoreImmersiveApplication_get_MainView(pApplication, &pView);
    ICoreImmersiveApplication_Release(pApplication);

    ICoreWindow *pWindow = {};
    ICoreApplicationView_get_CoreWindow(pView, &pWindow);

    MH_CreateHook(pWindow->lpVtbl->put_PointerCursor, &_put_PointerCursor, (PVOID *)&__put_PointerCursor);
    MH_EnableHook(pWindow->lpVtbl->put_PointerCursor);

    ICoreWindow_Release(pWindow);

    RoUninitialize();

    return EXIT_SUCCESS;
}

BOOL DllMainCRTStartup(HINSTANCE hInstance, DWORD dwReason, PVOID pReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetCurrentPackageFamilyName(&(UINT32){}, NULL) != ERROR_INSUFFICIENT_BUFFER)
            return FALSE;

        HANDLE hMutex = CreateMutexW(NULL, FALSE, L"Bedrock.UWP");
        if (!hMutex || GetLastError())
        {
            CloseHandle(hMutex);
            return FALSE;
        }

        DisableThreadLibraryCalls(hInstance);

        MH_Initialize();

        CloseHandle(CreateThread(NULL, (SIZE_T){}, ThreadProc, NULL, (DWORD){}, NULL));
    }
    return TRUE;
}