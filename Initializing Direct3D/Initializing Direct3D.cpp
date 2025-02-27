#include <iostream>
#include <string.h>
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h> 
#include <exception>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

//void EnableDebugLayer()
//{
//    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
//    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
//    debugController->EnableDebugLayer();
//}

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
    //EnableDebugLayer();
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
#endif

    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
    HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));

    if (FAILED(hardwareResult))
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice)));
    }

    return 0;
}