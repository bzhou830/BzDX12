#include "Graphics.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

Graphics::Graphics(HWND hWnd)
{
	HRESULT hr = S_OK;
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(pDxgiFactory.GetAddressOf()));
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(pDevice.GetAddressOf()));
	hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(pCmdAllocator.GetAddressOf()));
	hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCmdAllocator.Get(), nullptr, IID_PPV_ARGS(pCmdList.GetAddressOf()));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = pDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(pCmdQueue.GetAddressOf()));

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = 0;
	swapchainDesc.Height = 0;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = pDxgiFactory->CreateSwapChainForHwnd(pCmdQueue.Get(),
		hWnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)pSwap.GetAddressOf());
}

Graphics::~Graphics()
{
}

void Graphics::EndFrame()
{
	pSwap->Present(1u, 0u);
}
