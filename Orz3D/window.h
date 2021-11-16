#pragma once
#include "BzWin.h"
#include "BzException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <memory>

class Window
{
public:
	class Exception : public BzException
	{
	public:
		Exception(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
private:
    class WindowClass
    {
    public:
        static const wchar_t* GetName() noexcept;
        static HINSTANCE GetInstance() noexcept;
    private:
        WindowClass() noexcept;
        ~WindowClass();
        WindowClass(const WindowClass&) = delete;
        WindowClass& operator=(const WindowClass&) = delete;
        static constexpr const wchar_t* wndClassName = L"Orz3D Engine Window";
        static WindowClass wndClass;
        HINSTANCE hInst;
    };
public:
    Window(int width, int height, const wchar_t* name);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
	Graphics& Gfx();
private:
	static LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    int width;
    int height;
    HWND hWnd;
	Keyboard kbd;
	Mouse mouse;
	std::unique_ptr<Graphics> pGfx;
};

// error exception helper macro
#define CHWND_EXCEPT( hr ) Window::Exception( __LINE__,__FILE__,hr )
#define CHWND_LAST_EXCEPT() Window::Exception( __LINE__,__FILE__,GetLastError() )