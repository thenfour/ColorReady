

#pragma once


#include "colorpicker2d.h"
#include "colorwheel2d.h"
#include "colorpicker3d.h"


typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0> MainWndTraits;

class MainWnd :
  public CWindowImpl<MainWnd, CWindow, MainWndTraits>
{
public:
  MainWnd() :
    m_Picker2d(&m_cmgr),
    m_Wheel2d(&m_cmgr),
    m_bTracking(false)
  {
    // initialize the color manager
    m_cmgr.RegisterColorSpace(RGBGetInfo());
    m_cmgr.RegisterColorSpace(HWBGetInfo());
    //m_cmgr.RegisterColorSpace(YIQGetInfo());
    //m_cmgr.RegisterColorSpace(YUVGetInfo());
    m_cmgr.RegisterColorSpace(GrayGetInfo());
    m_cmgr.RegisterColorSpace(HSVGetInfo());
    //m_cmgr.RegisterColorSpace(XYZGetInfo());

    m_selcolor.SetManager(&m_cmgr);
  }

  BEGIN_MSG_MAP_EX(MainWnd)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_SIZE(OnSize)
    MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_NOTIFY(OnNotify)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_CAPTURECHANGED(OnCaptureChanged)
  END_MSG_MAP()

  void OnCaptureChanged(HWND h)
  {
    if(m_bTracking)
    {
      ReleaseCapture();
      m_bTracking = false;
    }
  }

  void OnLButtonDown(UINT wParam, WTL::CPoint& p)
  {
    SetCapture();
    m_bTracking = true;
  }

  void OnLButtonUp(UINT wParam, WTL::CPoint& p)
  {
    if(m_bTracking)
    {
      ReleaseCapture();
    }
  }

  void OnMouseMove(UINT wParam, WTL::CPoint& p)
  {
    if(m_bTracking)
    {
      CRect rc;
      GetClientRect(&rc);
      Colorant c = (Colorant)p.x / rc.right;
      clamp(c, 0, 1);
      m_Wheel2d.SetColorant(1, c);
    }
  }


  LRESULT OnEraseBkgnd(HDC dc)
  {
    return 1;
  }

  void OnPaint(HDC)
  {
    PAINTSTRUCT ps;
    BeginPaint(&ps);

    if(m_selcolor.GetColorSpaceID() != CS_Invalid)
    {
      HDC h = GetDC();
      char s[400];
      sprintf(s, "(x,y) = (%d,%d)    \r\n(x,y) = (%d,%d)    \r\n"
        "%s: %04.4f    \r\n"
        "%s: %04.4f    \r\n"
        "%s: %04.4f    ",
        m_selpoint.x, m_selpoint.y,
        m_point2.x, m_point2.y,
        m_selcolor.GetColorantAbbreviation(0).c_str(), m_selcolor.GetColorant(0),
        m_selcolor.GetColorantAbbreviation(1).c_str(), m_selcolor.GetColorant(1),
        m_selcolor.GetColorantAbbreviation(2).c_str(), m_selcolor.GetColorant(2));
      CRect rc;
      GetClientRect(&rc);

      m_offscreen.BeginDraw();
      m_offscreen.Fill(0);
      FilledCircleAAG(rc.right * 3 / 4, 50, 49, this, &MainWnd::__DrawHLine, this, &MainWnd::__DrawAlphaPixel);
      m_offscreen._DrawText(s, 0, 0);
      m_offscreen.Commit();
      m_offscreen.Blit(h, 0, 0);

      ReleaseDC(h);
    }
    EndPaint(&ps);
  }

  void __DrawHLine(long x1, long x2, long y)
  {
    m_offscreen.HLine(x1, x2+1, y, m_selcolor.GetRGBFast());
  }

  void __DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
  {
    RgbPixel back;
    RgbPixel fore = m_selcolor.GetRGBFast();
    if(m_offscreen.GetPixelSafe(back, cx+x, cy+y)) m_offscreen.SetPixelSafe(cx+x, cy+y, Colors::MixColorsInt(f, fmax, fore, back));
    if(m_offscreen.GetPixelSafe(back, cx+x, cy-y-1)) m_offscreen.SetPixelSafe(cx+x, cy-y-1, Colors::MixColorsInt(f, fmax, fore, back));
    if(m_offscreen.GetPixelSafe(back, cx-x-1, cy+y)) m_offscreen.SetPixelSafe(cx-x-1, cy+y, Colors::MixColorsInt(f, fmax, fore, back));
    if(m_offscreen.GetPixelSafe(back, cx-x-1, cy-y-1)) m_offscreen.SetPixelSafe(cx-x-1, cy-y-1, Colors::MixColorsInt(f, fmax, fore, back));
  }

  ColorSpec m_selcolor;
  CPoint m_selpoint;
  CPoint m_point2;
  bool m_bTracking;

  LRESULT OnNotify(int wParam, LPNMHDR phdr)
  {
    if(phdr->hwndFrom == m_Wheel2d)
    {
      if(phdr->code == CPN_MOUSEMOVE)
      {
        CPNMouseMoveStruct* p = reinterpret_cast<CPNMouseMoveStruct*>(phdr);
        m_selpoint = p->p;
        if(m_Wheel2d.ColorFromPosition(m_selcolor, p->p.x, p->p.y))
        {
          m_Wheel2d.PositionFromColor(m_point2, m_selcolor);
          RedrawWindow();
        }
      }
      else if(phdr->code == CPN_SELCHANGED)
      {
      }
    }
    return 0;
  }

  LRESULT OnCreate(LPCREATESTRUCT p)
  {
    //m_2d.Create(*this);
    //m_2d.ShowWindow(SW_NORMAL);
    ColorSpec cs(&m_cmgr);
    cs.InitNew(CS_HWB);
    cs.GetColorant(0) = 0;
    cs.GetColorant(1) = 1;
    cs.GetColorant(2) = 0;

    m_Wheel2d.Create(*this);
    //m_Wheel2d.SetBorder(MakeRgbPixel(120,80,140), 3);
    m_Wheel2d.SetColor(cs);
    m_Wheel2d.ShowWindow(SW_NORMAL);
    m_Wheel2d.SetAxes(0, 2);
    m_Wheel2d.SetRotation(0.20f);
    m_Wheel2d.SetInvertR(false);

    cs.GetColorant(0) = 0.75;
    cs.GetColorant(1) = 1.0;
    cs.GetColorant(2) = 0.5;
    m_Wheel2d.SetSelectedColor(cs);

    //m_t.Create(*this);
    //m_t.ShowWindow(SW_NORMAL);
    return 0;
  }

  void OnEnterSizeMove()
  {
    m_Wheel2d.OnEnterSizeMove();
  }

  void OnExitSizeMove()
  {
    m_Wheel2d.OnExitSizeMove();
  }

  void OnDestroy()
  {
    PostQuitMessage(0);
    SetMsgHandled(true);
  }
		
  void OnSize(UINT wParam, const CSize& s)
  {
    m_offscreen.SetSize(s.cx, s.cy);
    if(m_Wheel2d.IsWindow())
    {
      m_Wheel2d.MoveWindow(5, 100, s.cx - 10, s.cy - 105);
    }
    //if(m_Picker2d.IsWindow())
    //{
    //  m_Picker2d.MoveWindow(5, 5, s.cx - 10, s.cy - 10);
    //}
  }

private:
  ColorPicker2D m_Picker2d;
  ColorWheel2D m_Wheel2d;
  ColorManager m_cmgr;
  TempControl m_t;
  AnimBitmap m_offscreen;
};

