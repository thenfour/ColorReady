// 0626 color picker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "polarlut.h"
#include "Colorready.h"

WTL::CAppModule _Module;


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  CoInitialize(NULL);
	DefWindowProc(NULL, 0, 0, 0L);// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	WTL::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

  _Module.Init(NULL, hInstance);
  ColorReady* p = new ColorReady();
  p->Run();
  delete p;
	_Module.Term();
	CoUninitialize();
	return 0;
}


