#define INITGUID
#define COBJMACROS
#define WIDL_C_INLINE_WRAPPERS

#include <d3d11_1.h>
#include <MinHook.h>
#include <appmodel.h>

BOOL fFallback = {}, fForce = {}, fFlag = {};
HRESULT (*__Present)(LPUNKNOWN, UINT, UINT) = {};
HRESULT (*__ResizeBuffers)(LPUNKNOWN, UINT, UINT, UINT, DXGI_FORMAT, UINT) = {};
HRESULT (*__CreateSwapChainForCoreWindow)(LPUNKNOWN, LPUNKNOWN, LPUNKNOWN, DXGI_SWAP_CHAIN_DESC1 *, LPUNKNOWN,
                                          IDXGISwapChain1 **ppSwapChain) = {};
                                          
HRESULT _Present(LPUNKNOWN This, UINT SyncInterval, UINT Flags)
{
    if (fForce)
        SyncInterval = (UINT){};

    if (!SyncInterval)
        Flags |= DXGI_PRESENT_ALLOW_TEARING;

    return fFlag ? __Present(This, SyncInterval, Flags) : DXGI_ERROR_DEVICE_RESET;
}

HRESULT _ResizeBuffers(LPUNKNOWN This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                       UINT SwapChainFlags)
{
    return __ResizeBuffers(This, BufferCount, Width, Height, NewFormat,
                           SwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
}

HRESULT _CreateSwapChainForCoreWindow(LPUNKNOWN This, LPUNKNOWN pDevice, LPUNKNOWN pWindow,
                                      DXGI_SWAP_CHAIN_DESC1 *pDesc, LPUNKNOWN pRestrictToOutput,
                                      IDXGISwapChain1 **ppSwapChain)
{
    if (!fFlag)
        fFlag = TRUE;

    if (fFallback)
    {
        LPUNKNOWN pUnknown = {};

        if (IUnknown_QueryInterface(pDevice, &IID_ID3D11Device, (PVOID *)&pUnknown))
            return DXGI_ERROR_INVALID_CALL;

        IUnknown_Release(pUnknown);
    }

    pDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    return __CreateSwapChainForCoreWindow(This, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

DWORD ThreadProc(PVOID pParameter)
{
    WCHAR szPath[MAX_PATH] = {};
    ExpandEnvironmentStringsW(L"%LOCALAPPDATA%\\..\\RoamingState\\Bedrock.DirectX.ini", szPath, MAX_PATH);

    fFallback = GetPrivateProfileIntW(L"Bedrock.DirectX", L"Fallback", FALSE, szPath) == TRUE;
    fForce = GetPrivateProfileIntW(L"Bedrock.DirectX", L"Force", FALSE, szPath) == TRUE;

    IDXGIFactory2 *pFactory = {};
    CreateDXGIFactory(&IID_IDXGIFactory2, (PVOID *)&pFactory);

    MH_Initialize();

    MH_CreateHook(pFactory->lpVtbl->CreateSwapChainForCoreWindow, &_CreateSwapChainForCoreWindow,
                  (PVOID *)&__CreateSwapChainForCoreWindow);
    MH_QueueEnableHook(pFactory->lpVtbl->CreateSwapChainForCoreWindow);

    IDXGISwapChain *pSwapChain = {};
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, FALSE, NULL, FALSE, D3D11_SDK_VERSION,
                                  &(DXGI_SWAP_CHAIN_DESC){.BufferCount = TRUE,
                                                          .BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
                                                          .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                                          .SampleDesc.Count = TRUE,
                                                          .Windowed = TRUE,
                                                          .OutputWindow = GetDesktopWindow()},
                                  &pSwapChain, NULL, NULL, NULL);

    MH_CreateHook(pSwapChain->lpVtbl->Present, &_Present, (PVOID *)&__Present);
    MH_QueueEnableHook(pSwapChain->lpVtbl->Present);

    MH_CreateHook(pSwapChain->lpVtbl->ResizeBuffers, &_ResizeBuffers, (PVOID *)&__ResizeBuffers);
    MH_QueueEnableHook(pSwapChain->lpVtbl->ResizeBuffers);

    MH_ApplyQueued();

    IDXGISwapChain_Release(pSwapChain);

    IDXGIFactory2_Release(pFactory);

    return EXIT_SUCCESS;
}

BOOL DllMainCRTStartup(HINSTANCE hInstance, DWORD dwReason, PVOID pReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        if (GetCurrentPackageFamilyName(&(UINT32){}, NULL) != ERROR_INSUFFICIENT_BUFFER)
            return FALSE;

        HANDLE hMutex = CreateMutexW(NULL, FALSE, L"Bedrock.DirectX");
        if (!hMutex || GetLastError())
        {
            CloseHandle(hMutex);
            return FALSE;
        }

        QueueUserWorkItem(ThreadProc, NULL, WT_EXECUTEDEFAULT);
    }
    return TRUE;
}