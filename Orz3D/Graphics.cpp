#include "Graphics.h"
#include <string>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

Graphics::Graphics(HWND hWnd)
{
	HRESULT hr = S_OK;
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(pDxgiFactory.GetAddressOf()));
	LogAdapters();
	
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

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	
	hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.GetAddressOf()));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	hr = pSwap->GetDesc(&swcDesc);
	
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	backBuffers = std::vector<ComPtr<ID3D12Resource>>(swcDesc.BufferCount);
	for (UINT i = 0; i < swcDesc.BufferCount; ++i) 
	{
		hr = pSwap->GetBuffer(i, IID_PPV_ARGS(backBuffers[i].GetAddressOf()));
		pDevice->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
		handle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	hr = pDevice->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
}

void Graphics::EndFrame()
{
	auto bbIdx = pSwap->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = backBuffers[bbIdx].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	pCmdList->ResourceBarrier(1, &BarrierDesc);

	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>((ULONGLONG)bbIdx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	pCmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pCmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	pCmdList->ResourceBarrier(1, &BarrierDesc);

	pCmdList->Close();

	ID3D12CommandList* cmdlists[] = { pCmdList.Get() };
	pCmdQueue->ExecuteCommandLists(1, cmdlists);

	pCmdQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal) 
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	pCmdAllocator->Reset();
	pCmdList->Reset(pCmdAllocator.Get(), nullptr);
	pSwap->Present(1u, 0u);
}

void Graphics::LogAdapters()
{
	UINT i = 0;
	ComPtr<IDXGIAdapter> adapter = nullptr;
	std::vector<ComPtr<IDXGIAdapter>> adapterList;
	while (pDxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";
		OutputDebugString((text.c_str()));
		adapterList.push_back(adapter);
		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
	}
}

void Graphics::LogAdapterOutputs(ComPtr<IDXGIAdapter> adapter)
{
	UINT i = 0;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	ComPtr<IDXGIOutput> output = nullptr;
	while (adapter->EnumOutputs(i, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, mBackBufferFormat);
		++i;
	}
}


void Graphics::LogOutputDisplayModes(ComPtr<IDXGIOutput> output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		OutputDebugString(text.c_str());
	}
}