#pragma once
#include "window.h"

class App
{
public:
	App();
	App(int width, int height, const wchar_t* name);
	~App();
	int Run();
private:
	void doFrame();
	Window wnd;
};

