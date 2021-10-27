#pragma once
#include "BzWin.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class Graphics
{
public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
	void EndFrame();
private:
	ComPtr<IDXGIFactory6> pDxgiFactory;
	ComPtr<ID3D12Device> pDevice;
	ComPtr<IDXGISwapChain4> pSwap;
	ComPtr<ID3D12CommandAllocator> pCmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> pCmdList;
	ComPtr<ID3D12CommandQueue> pCmdQueue;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps;
	ComPtr<ID3D12Fence> m_fence = nullptr;
	UINT64 m_fenceVal = 0;
};

