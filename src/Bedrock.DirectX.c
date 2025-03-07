#define INITGUID
#define COBJMACROS
#define WIDL_C_INLINE_WRAPPERS

#include <d3d11.h>
#include <MinHook.h>
#include <dxgi1_2.h>
#include <appmodel.h>

BOOL fFallback = {}, fForce = {}, fFlag = {};

HRESULT (*__Present__)(LPUNKNOWN, UINT, UINT) = {};

HRESULT _Present_(LPUNKNOWN This, UINT SyncInterval, UINT Flags)
{
    return fFlag ? __Present__(This, fForce ? (UINT){} : SyncInterval,
                               fForce         ? DXGI_PRESENT_ALLOW_TEARING
                               : SyncInterval ? Flags
                                              : DXGI_PRESENT_ALLOW_TEARING)
                 : DXGI_ERROR_DEVICE_RESET;
}

HRESULT (*__ResizeBuffers__)(LPUNKNOWN, UINT, UINT, UINT, DXGI_FORMAT, UINT) = {};

HRESULT _ResizeBuffers_(LPUNKNOWN This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                        UINT SwapChainFlags)
{
    return __ResizeBuffers__(This, BufferCount, Width, Height, NewFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
}

HRESULT (*__CreateSwapChainForCoreWindow__)(LPUNKNOWN, LPUNKNOWN, LPUNKNOWN, DXGI_SWAP_CHAIN_DESC1 *, LPUNKNOWN,
                                            IDXGISwapChain1 **ppSwapChain) = {};

HRESULT _CreateSwapChainForCoreWindow_(LPUNKNOWN This, LPUNKNOWN pDevice, LPUNKNOWN pWindow,
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

    pDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    return __CreateSwapChainForCoreWindow__(This, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

DWORD ThreadProc(PVOID pParameter)
{
    WCHAR szPath[MAX_PATH] = {};
    ExpandEnvironmentStringsW(L"%LOCALAPPDATA%\\..\\RoamingState\\Bedrock.DirectX.ini", szPath, MAX_PATH);

    fFallback = GetPrivateProfileIntW(L"Bedrock.DirectX", L"Fallback", FALSE, szPath) == TRUE;
    fForce = GetPrivateProfileIntW(L"Bedrock.DirectX", L"Force", FALSE, szPath) == TRUE;

    IDXGIFactory2 *pFactory = {};
    CreateDXGIFactory(&IID_IDXGIFactory2, (PVOID *)&pFactory);

    MH_CreateHook(pFactory->lpVtbl->CreateSwapChainForCoreWindow, &_CreateSwapChainForCoreWindow_,
                  (PVOID *)&__CreateSwapChainForCoreWindow__);
    MH_QueueEnableHook(pFactory->lpVtbl->CreateSwapChainForCoreWindow);

    IDXGISwapChain *pChain = {};
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
                                  &(DXGI_SWAP_CHAIN_DESC){.BufferCount = 1,
                                                          .BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
                                                          .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                                          .SampleDesc.Count = 1,
                                                          .Windowed = TRUE,
                                                          .OutputWindow = GetDesktopWindow()},
                                  &pChain, NULL, NULL, NULL);

    MH_CreateHook(pChain->lpVtbl->Present, &_Present_, (PVOID *)&__Present__);
    MH_QueueEnableHook(pChain->lpVtbl->Present);

    MH_CreateHook(pChain->lpVtbl->ResizeBuffers, &_ResizeBuffers_, (PVOID *)&__ResizeBuffers__);
    MH_QueueEnableHook(pChain->lpVtbl->ResizeBuffers);

    MH_ApplyQueued();

    IDXGISwapChain_Release(pChain);

    IDXGIFactory2_Release(pFactory);

    return EXIT_SUCCESS;
}

BOOL DllMainCRTStartup(HINSTANCE hInstance, DWORD dwReason, PVOID pReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetCurrentPackageFamilyName(&(UINT32){}, NULL) != ERROR_INSUFFICIENT_BUFFER)
            return FALSE;

        HANDLE hMutex = CreateMutexW(NULL, FALSE, L"Bedrock.DirectX");
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