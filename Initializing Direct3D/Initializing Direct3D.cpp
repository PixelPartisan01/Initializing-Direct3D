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


void createCommandOBjects(Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice, 
                          Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue, 
                          Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc,
                          Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
    ThrowIfFailed(md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));
    ThrowIfFailed(md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));

    mCommandList->Close();
}


int main()  
{  
#if defined(DEBUG) || defined(_DEBUG)  
   //EnableDebugLayer();  
   Microsoft::WRL::ComPtr<ID3D12Debug> debugController;  
   ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));  
   debugController->EnableDebugLayer();  
#endif  

   Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
   Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
   Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
   D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;

   Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;  
   ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));  

   //Creating Device  
   Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;  
   HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));  
   if (FAILED(hardwareResult))  
   {  
       Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;  
       ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));  
       ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice)));  
   }  

   //Fence  
   //ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));  

   //Qulity
   msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  
   msQualityLevels.SampleCount = 4;  
   msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;  
   msQualityLevels.NumQualityLevels = 0;  

   ThrowIfFailed(md3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
   assert(msQualityLevels.NumQualityLevels > 0 && "Unexpected MSAA qulity level!");

   return 0;  
}