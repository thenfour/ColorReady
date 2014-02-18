// this will display 3 dimensions of color in a hexagon, and let the user drag 3 handles around
// for the 3 dimensions


#pragma once


#include "animbitmap.h"
#include "geom.h"


typedef CWinTraits<WS_CHILD, 0> TempTraits;

class TempControl :
  public CWindowImpl<TempControl, CWindow, TempTraits>
{
public:
  DECLARE_WND_CLASS("TempControl");

  TempControl()
  {
  }

  BEGIN_MSG_MAP_EX(TempControl)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
  END_MSG_MAP()

  LRESULT OnEraseBkgnd(HDC dc)
  {
    return 1;
  }

  void OnPaint(HDC)
  {
    PAINTSTRUCT ps;
    BeginPaint(&ps);
    HDC h = GetDC();
    _Draw(h);
    ReleaseDC(h);
    EndPaint(&ps);
  }

#define m_back RGB(255,0,0)
#define m_fcol RGB(0,0,0)
#define m_fcol2 RGB(0,255,0)

  void SetSolidPixel(long x, long y)
  {
    m_bmp.SetPixel(x, y, m_fcol);
  }

  void SetAlphaPixel(long x, long y, long f, long fmax)
  {
    COLORREF b = m_back;
    COLORREF cb = m_bmp.GetPixel(x, y);
    COLORREF c = MixColorsInt(f, fmax, m_fcol, cb);
    m_bmp.SetPixel(x, y, c);
  }

  void SetSolidPixelDonut(long x, long y)
  {
    m_bmp.SetPixel(x, y, m_fcol2);
  }

  void SetAlphaPixelDonut(long x, long y, long f, long fmax)
  {
    COLORREF b = m_back;
    COLORREF cb = m_bmp.GetPixel(x, y);
    COLORREF c = MixColorsInt(f, fmax, m_fcol2, cb);
    m_bmp.SetPixel(x, y, c);
  }

private:

  void _Draw(HDC hdc)
  {
    CRect rc;
    GetClientRect(&rc);
    m_bmp.SetSize(rc.Width(), rc.Height());
    m_bmp.BeginDraw();
    m_bmp.Fill(m_back);
    //DonutAAG(60, 60, 20, 5, this, SetSolidPixelDonut, this, SetAlphaPixelDonut);
    //FilledCircleAA(100, 100, 50, this, SetSolidPixel, this, SetAlphaPixel);
    m_bmp.Commit();
    m_bmp.Blit(hdc, 0, 0);
  }

  AnimBitmap m_bmp;
};
