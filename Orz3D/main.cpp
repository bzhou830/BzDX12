#include "App.h"

wchar_t* stringToWchar_t(std::string str) {
	std::string temp = str;
	int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL, 0);
	wchar_t* wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	return wszUtf8;
}


// 需要_In_检测, 删除时会有C28251：“WinMain"批注不一致的警告出现
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,          // handle to current instance
	_In_opt_ HINSTANCE hPrevInstance,  // handle to previous instance
	_In_ LPSTR lpCmdLine,              // command line
	_In_ int nCmdShow                  // show state
)
{
	try 
	{
		return App{800, 600, L"Orz.3D"}.Run();
	}
	catch (const BzException& e)
	{
		MessageBox(nullptr, stringToWchar_t(e.what()), stringToWchar_t(e.GetType()), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e) 
	{
		MessageBox(nullptr, stringToWchar_t(e.what()), L"std::exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, L"No details available", L"Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}
