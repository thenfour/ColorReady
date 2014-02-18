

#include "stdafx.h"
#include "colorready.h"


ColorReady::ColorReady()
{
}

ColorReady::~ColorReady()
{
}

void ColorReady::Run()
{
  WTL::CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

  MainWnd m;
  CRect rc(0, 0, 450, 440);
  m.Create((HWND)0, rc, _T("Color Ready"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);
  m.ShowWindow(SW_SHOWNORMAL);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
}

