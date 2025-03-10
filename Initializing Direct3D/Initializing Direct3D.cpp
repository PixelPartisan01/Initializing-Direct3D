#include <iostream>
#include <cstdio>
#include <string.h>
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h> 
#include <exception>
#include <WinHvPlatform.h>
#include <comdef.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

HRESULT ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        _com_error err(hr);
        std::wcerr << err.ErrorMessage() << std::endl;
        //std::wcerr << static_cast<const wchar_t*>(err.Description()) << std::endl;
        std::wcerr << err.HelpContext() << std::endl;
        return hr;
    }
}

/// <summary>
/// Creates the command objects for the Direct3D application.
/// </summary>
/// <param name="md3dDevice">The Direct3D device.</param>
/// <param name="mCommandQueue">The command queue to be created.</param>
/// <param name="mDirectCmdListAlloc">The command allocator to be created.</param>
/// <param name="mCommandList">The command list to be created.</param>
void CreateCommandOBjects(Microsoft::WRL::ComPtr<ID3D12Device> &md3dDevice, 
                         Microsoft::WRL::ComPtr<ID3D12CommandQueue> &mCommandQueue, 
                         Microsoft::WRL::ComPtr<ID3D12CommandAllocator> &mDirectCmdListAlloc,
                         Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &mCommandList)
{
   D3D12_COMMAND_QUEUE_DESC queueDesc = {};
   queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
   queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
   ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
   ThrowIfFailed(md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));
   ThrowIfFailed(md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));

   mCommandList->Close();
}


/// <summary>  
/// Creates the swap chain for the Direct3D application.  
/// </summary>  
/// <param name="mSwapChain">The swap chain to be created.</param>  
/// <param name="mdxgiFactory">The DXGI factory used to create the swap chain.</param>  
/// <param name="mCommandQueue">The command queue used for rendering commands.</param>  
/// <param name="clientWidth">The width of the client area.</param>  
/// <param name="clientHeight">The height of the client area.</param>  
/// <param name="refreshRate">The refresh rate of the swap chain.</param>  
/// <param name="SwapChainBufferCount">The number of buffers in the swap chain.</param>  
/// <param name="mhMainWnd">The handle to the main window.</param>  
/// <param name="m4xMsaaState">The state of 4x MSAA (enabled or disabled).</param>  
/// <param name="m4xMsaaQuality">The quality level of 4x MSAA.</param>  
/// <param name="windowed">Specifies whether the swap chain is windowed or full-screen.</param>
void CreateSwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain> &mSwapChain,
                     Microsoft::WRL::ComPtr<IDXGIFactory4> &mdxgiFactory,
                     Microsoft::WRL::ComPtr<ID3D12CommandQueue> &mCommandQueue,
                     UINT clientWidth, 
                     UINT clientHeight, 
                     UINT refreshRate, 
                     UINT SwapChainBufferCount,
                     HWND mhMainWnd, 
                     bool m4xMsaaState,
                     UINT m4xMsaaQuality = 1,
                     bool windowed = true)
{
    //Release previous swapchain
    mSwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = clientWidth;
    sd.BufferDesc.Height = clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = refreshRate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = mhMainWnd;
    sd.Windowed = windowed;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    ThrowIfFailed(mdxgiFactory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf()));
}


/// <summary>
/// Creates the Render Target View (RTV) and Depth Stencil View (DSV) descriptor heaps.
/// </summary>
/// <param name="SwapChainBufferCount">The number of buffers in the swap chain.</param>
/// <param name="md3dDevice">The Direct3D device.</param>
/// <param name="mRtvHeap">The RTV descriptor heap to be created.</param>
/// <param name="mDsvHeap">The DSV descriptor heap to be created.</param>
void CreateRtvAndDsvDescriptorHeaps(UINT SwapChainBufferCount, 
                                    Microsoft::WRL::ComPtr<ID3D12Device> &md3dDevice,
                                    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &mRtvHeap,
                                    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &mDsvHeap)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

//D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& mRtvHeap,
//                                                  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& mDsvHeap,
//                                                  int mCurrBackBuffer)
//{
//    return D3D12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrBackBuffer, 0);
//}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()  
{  
#if defined(DEBUG) || defined(_DEBUG)  
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));  
    debugController->EnableDebugLayer();
    std::printf("Debug - ON\n\n");
#endif

    static const UINT SwapChainBufferCount{ 2 };
    int mCurrBackBuffer{ 0 };
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    bool m4xMsaaState = false;
    UINT m4xMsaaQuality;
    UINT mRtvDescriptorSize{ 1 };
    UINT clientWidth{ 800 };
    UINT clientHeight{ 700 };

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap; // Render Target View
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap; // Depth/Stencil View
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    
    //Qulity
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;

    std::printf("Creating mdxgiFactory...\n");
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;  
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));
    std::printf("\tmdxgiFactory is created successfully.\n");

    //Creating Device  
    std::printf("Creating device...\n");
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;  
    HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));  
    if (!FAILED(hardwareResult))  
    {  
        Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;  
        ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));  
        ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice)));
        std::printf("\tDevice is created successfully.\n");
    } 

    //Fence  
    std::printf("Creating fence...\n");
    ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
    std::printf("\tFence is created successfully.\n");

    ThrowIfFailed(md3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

    m4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m4xMsaaQuality > 0 && "Unexpected MSAA qulity level!");
    
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"3D App", WS_OVERLAPPEDWINDOW, 100, 100, clientWidth, clientHeight, nullptr, nullptr, wc.hInstance, nullptr);
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    CreateRtvAndDsvDescriptorHeaps(SwapChainBufferCount, md3dDevice, mRtvHeap, mDsvHeap);//
    CreateSwapChain(mSwapChain, mdxgiFactory, mCommandQueue, clientWidth, clientHeight, 60, SwapChainBufferCount, hwnd, m4xMsaaState, m4xMsaaQuality, true);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeaphandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; i++)
    {
        ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
        md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeaphandle);
        rtvHeaphandle.Offset(1, mRtvDescriptorSize);
    }
    


    return 0;  
}