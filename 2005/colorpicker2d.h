/*
  This is a control for picking a color from a 2-d rectangular window.  We will support
  all color spaces.

  Since different color spaces have different # of colorants, we need to have a way to
  set the "unused" dimensions, and control the 2 dimensions the UI will draw.  For instance
  with CMYK, lets say the user has chosen C and M for the control's X and Y dimensions.
  There is still Y and K, so the user needs to set that explicitly.

  To tackle this, the user will pass in a UIColor (variant-style class) which describes
  both the colorspace to use and the "default color" to use in rendering.  After that,
  the user can set the axes to render with SetAxes().

  We have a spectrum bitmap and an offscreen bitmap.  The spectrum bitmap is always just the
  size of the spectrum plus border, minus the padding.  Offscreen is the size of the window.
*/

#pragma once


#include "animbitmap.h"
#include "colors.h"
#include "geom.h"
#include "colorcontrols.h"


using namespace Colors;


class ColorPicker2D :
  public CWindowImpl<ColorPicker2D, CWindow, ColorPickerTraits>
{
public:
  DECLARE_WND_CLASS("ColorPicker2D");

  static const long DefaultPadding = 6;
  static const long DefaultBorderWidth = 4;
  inline static RgbPixel DefaultBorderColor() { return MakeRgbPixel(255,220,190); }
  inline static RgbPixel DefaultBackColor() { return MakeRgbPixel(96,90,130); }

  static const long SelHandleRadius = 3;

  ColorPicker2D(ColorManager* pmgr) :
    m_col(pmgr),
    m_pmgr(pmgr),
    m_sel(pmgr),
    m_xColorant(0),
    m_yColorant(1),
    m_bordersize(DefaultBorderWidth),
    m_padding(DefaultPadding),
    m_bLowRes(false),
    m_bSpectrumDirty(true),
    m_bOffscreenDirty(true),
    m_InvertX(false),
    m_InvertY(false),
    m_hNotify(0),
    m_havecapture(false)
  {
    m_bordercolor = DefaultBorderColor();
    m_backcolor = DefaultBackColor();
    m_selhandlecolor = MakeRgbPixel(0,0,0);
  }

  BEGIN_MSG_MAP_EX(ColorPicker2D)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_SIZE(OnSize)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_CAPTURECHANGED(OnCaptureChanged)
  END_MSG_MAP()

  void OnCaptureChanged(HWND h)
  {
    if(m_havecapture)
    {
      ReleaseCapture();
      m_havecapture = false;
    }
  }

	void OnLButtonDown(UINT wParam, WTL::CPoint& p)
  {
    SetCapture();
    m_havecapture = true;
    ColorSpec s;
    ColorFromPosition(s, p.x, p.y);
    SetSelectedColor(s);
  }

  void OnLButtonUp(UINT wParam, WTL::CPoint& p)
  {
    if(m_havecapture)
    {
      ReleaseCapture();
    }
  }

  void OnMouseMove(UINT wParam, WTL::CPoint& p)
  {
    CPNMouseMoveStruct data;
    data.hdr.code = CPN_MOUSEMOVE;
    data.hdr.hwndFrom = *this;
    data.hdr.idFrom = 0;
    data.p = p;
    SendMessage(m_hNotify, WM_NOTIFY, 0, (LPARAM)&data);

    if(m_havecapture)
    {
      ColorSpec s;
      ColorFromPosition(s, p.x, p.y);
      SetSelectedColor(s);
    }
  }

  LRESULT OnEraseBkgnd(HDC dc)
  {
    return 1;
  }

  LRESULT OnCreate(LPCREATESTRUCT p)
  {
    m_col.InitNew(CS_RGB);
    m_hNotify = GetParent();
    return 0;
  }

  void SetNotify(HWND h)
  {
    m_hNotify = h;
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

  void OnEnterSizeMove()
  {
    m_bLowRes = true;
  }

  void OnExitSizeMove()
  {
    m_bLowRes = false;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void OnSize(UINT wParam, const WTL::CSize& s)
  {
    m_size = s;

    // always adjust the offscreen buffer
    m_offscreen.SetSize(m_size.cx, m_size.cy);
    m_offscreen.BeginDraw();
    m_offscreen.Fill(m_backcolor);

    if(!m_bLowRes)
    {
      _SetSpectrumDirty();
    }
    else
    {
      // see if we are out of tolerance for scaling.  we have to use the size of the
      // spectrum for this, so if you are just making the window wider or something it wont
      // change the values.
      CSize s = _CalcSpectrumSizeWithBorder();
      long real_pixels = s.cx * s.cy;
      long stored_pixels = m_spectrum.GetHeight() * m_spectrum.GetWidth();
      // determine delta as % of original
      float dp = static_cast<float>(abs(real_pixels - stored_pixels)) / stored_pixels;
      if(dp > 0.1)
      {
        _SetSpectrumDirty();
      }
    }
  }

  bool ColorIsInSpectrum(const ColorSpec& c) const
  {
    bool r = false;
    if(c.GetColorSpaceID() == m_col.GetColorSpaceID())
    {
      // all the colorants that we do not use - make sure they are equal
      long i;
      r = true;
      for(i = 0; i < c.GetColorantCount(); i ++)
      {
        if((i != m_xColorant) && (i != m_yColorant))
        {
          if(!xequals(c.GetColorant(i), m_col.GetColorant(i), 0.00001))
          {
            r = false;
            break;
          }
        }
      }
    }

    return r;
  }

  ColorSpec GetSelectedColor() const
  {
    return m_sel;
  }

  bool SetSelectedColor(ColorSpec& s, bool redraw = true)
  {
    bool r = false;
    if(ColorIsInSpectrum(s))
    {
      m_sel = s;

      m_selhandlecolor = MakeRgbPixel(0,0,0);
      ColorSpec g = m_sel;
      if(CR_Error != g.ConvertToColorSpace(CS_Gray))
      {
        g.GetColorant(0) += 0.42f;
        if(g.GetColorant(0) > 1.0f)
        {
          g.GetColorant(0) -= 1.0f;
        }

        m_selhandlecolor = g.GetRGBFast();
      }

      NMHDR hdr;
      hdr.code = CPN_SELCHANGED;
      hdr.hwndFrom = *this;
      hdr.idFrom = 0;
      SendMessage(m_hNotify, WM_NOTIFY, 0, (LPARAM)&hdr);

      _SetSpectrumDirty();
      RedrawWindow();
      r = true;
    }

    return r;
  }

  void SetAxes(long x, long y)
  {
    m_xColorant = x;
    m_yColorant = y;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void SetInvertX(bool v)
  {
    if(m_InvertX != v)
    {
      m_InvertX = v;
      _SetSpectrumDirty();
      Invalidate(0);
    }
  }

  void SetInvertY(bool v)
  {
    if(m_InvertY != v)
    {
      m_InvertY = v;
      _SetSpectrumDirty();
      Invalidate(0);
    }
  }

  void SetColor(const ColorSpec& val)
  {
    m_sel = val;
    m_col = val;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void SetColorant(long i, const Colorant& v)
  {
    if((i != m_xColorant) && (i != m_yColorant))
    {
      m_col.GetColorant(i) = v;
      m_sel.GetColorant(i) = v;
      _SetSpectrumDirty();
      Invalidate(0);
    }
  }

  void SetBackground(RgbPixel c)
  {
    m_backcolor = c;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void SetBorder(long width)
  {
    m_bordersize = width;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void SetBorder(RgbPixel c, long width)
  {
    m_bordercolor = c;
    m_bordersize = width;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  void SetPadding(long width)
  {
    m_padding = width;
    _SetSpectrumDirty();
    Invalidate(0);
  }

  // color from client coords
  bool ColorFromPosition(ColorSpec& ret, long x, long y) const
  {
    ret = m_col;
    CSize s = _CalcSpectrumSizeNoBorder();
    // adjust x and y so they are in the spectrum (clamp out of bounds too
    x -= m_padding + m_bordersize;
    y -= m_padding + m_bordersize;
    clamp(x, 0, s.cx);
    clamp(y, 0, s.cy);

    ret.GetColorant(m_xColorant) = static_cast<float>(x) / s.cx;
    ret.GetColorant(m_yColorant) = static_cast<float>(y) / s.cy;
    return true;
  }

  // p is client coords
  void PositionFromColor(CPoint& p, const Colorant& cx, const Colorant& cy) const
  {
    CSize s = _CalcSpectrumSizeNoBorder();
    p.x = static_cast<long>(cx * s.cx);
    p.x += m_padding + m_bordersize;

    p.y = static_cast<long>(cy * s.cy);
    p.y += m_padding + m_bordersize;
  }

  // p is client coords
  // returns false if there is an error.  c must be the same colorspace as m_col
  bool PositionFromColor(CPoint& p, const ColorSpec& c) const
  {
    bool r = false;
    if(ColorIsInSpectrum(c))
    {
      PositionFromColor(p, c.GetColorant(m_xColorant), c.GetColorant(m_yColorant));
      r = true;
    }
    return r;
  }

  void __DrawSelectionHandleHLine(long x1, long x2, long y)
  {
    m_offscreen.HLine(x1, x2+1, y, m_selhandlecolor);
  }

  void __DrawSelectionHandleAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
  {
    RgbPixel back;
    if(m_offscreen.GetPixelSafe(back, cx+x, cy+y)) m_offscreen.SetPixelSafe(cx+x, cy+y, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
    if(m_offscreen.GetPixelSafe(back, cx+x, cy-y-1)) m_offscreen.SetPixelSafe(cx+x, cy-y-1, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
    if(m_offscreen.GetPixelSafe(back, cx-x-1, cy+y)) m_offscreen.SetPixelSafe(cx-x-1, cy+y, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
    if(m_offscreen.GetPixelSafe(back, cx-x-1, cy-y-1)) m_offscreen.SetPixelSafe(cx-x-1, cy-y-1, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
  }

private:

  void _DrawSelectionHandle()
  {
    CPoint p;// coords of the selection.
    PositionFromColor(p, m_sel.GetColorant(m_xColorant), m_sel.GetColorant(m_yColorant));
    DonutAAG(p.x, p.y, 3, 2, this, &ColorPicker2D::__DrawSelectionHandleHLine, this, &ColorPicker2D::__DrawSelectionHandleAlphaPixel);
  }

  void _DrawError(HDC hdc)
  {
    WTL::CBrush i;
    CRect rc;
    GetClientRect(&rc);
    i.CreateSolidBrush(m_backcolor);
    TextOut(hdc, 0, 0, "Error", 5);
    FillRect(hdc, &rc, i);
    i.DeleteObject();
  }

  inline CSize _CalcSpectrumSizeWithBorder() const
  {
    return CSize(m_size.cx - (m_padding * 2), m_size.cy - (m_padding * 2));
  }

  inline CSize _CalcSpectrumSizeNoBorder() const
  {
    return CSize(m_size.cx - ((m_padding + m_bordersize) * 2), m_size.cy - ((m_padding + m_bordersize) * 2));
  }

  void _GenerateSpectrum()
  {
    CSize wholething = _CalcSpectrumSizeWithBorder();
    m_spectrum.SetSize(wholething.cx, wholething.cy);
    m_spectrum.BeginDraw();
    m_spectrum.Fill(m_bordercolor);

    // draw the spectrum
    CSize SpectrumSize = _CalcSpectrumSizeNoBorder();
    ColorSpec c = m_col;
    for(long y = 0; y < SpectrumSize.cy; ++ y)
    {
      c.GetColorant(m_yColorant) = static_cast<float>(y) / SpectrumSize.cy;
      for(long x = 0; x < SpectrumSize.cx; ++ x)
      {
        c.GetColorant(m_xColorant) = static_cast<float>(x) / SpectrumSize.cx;
        m_spectrum.SetPixel(x + m_bordersize, y + m_bordersize, c.GetRGBFast());
      }
    }

    m_spectrum.Commit();
  }

  void _Draw(HDC hdc)
  {
    bool bError = true;
    // make sure we are good to go.
    if(m_xColorant < m_col.GetColorantCount())
    {
      if(m_yColorant < m_col.GetColorantCount())
      {
        if(m_col.UsesColorants())
        {
          // make sure we can actually draw the thing.
          long minsize = 2 + (m_bordersize * 2) + max(SelHandleRadius + 1, m_padding * 2);
          if((m_size.cx >= minsize) && (m_size.cy >= minsize))
          {
            bError = false;
          }
        }
      }
    }

    if(bError)
    {
      _DrawError(hdc);
    }
    else
    {
      // we will calculate the bitmap even if we are in lowres mode in case the color changes
      // or some other drastic thing (other than just size)
      if(m_bSpectrumDirty)
      {
        // spectrum needs to be redrawn. resize it, and draw.
        _GenerateSpectrum();
        m_bSpectrumDirty = false;
      }

      if(m_bOffscreenDirty)
      {
        m_offscreen.BeginDraw();
        m_offscreen.Fill(m_backcolor);
        m_offscreen.Commit();
        long destx = (m_size.cx - m_spectrum.GetWidth()) / 2;
        long desty = (m_size.cy - m_spectrum.GetHeight()) / 2;
        m_spectrum.Blit(m_offscreen, destx, desty);

        _DrawSelectionHandle();

        m_bOffscreenDirty = false;
      }

      if(m_bLowRes)
      {
        m_offscreen.BeginDraw();
        m_offscreen.Fill(m_backcolor);
        m_offscreen.Commit();
        CSize s = _CalcSpectrumSizeWithBorder();
        long destx = (m_size.cx - s.cx) / 2;
        long desty = (m_size.cy - s.cy) / 2;
        m_spectrum.StretchBlit(m_offscreen, destx, desty, s.cx, s.cy);
      }

      m_offscreen.Blit(hdc, 0, 0);
    }
  }

  // call this when selection changes, or any other non-spectrum stuff
  void _SetOffscreenDirty()
  {
    m_bOffscreenDirty = true;
  }

  // call this when spectrum needs to be redrawn.
  void _SetSpectrumDirty()
  {
    m_bSpectrumDirty = true;
    _SetOffscreenDirty();
  }

  ColorManager* m_pmgr;

  bool m_bLowRes;// for when the user is sizing or something and we dont want to recalculate the spectrum every time
  bool m_havecapture;

  CSize m_size;// window size
  long m_bordersize;
  RgbPixel m_backcolor;
  RgbPixel m_bordercolor;
  RgbPixel m_selhandlecolor;
  long m_padding;// empty space on either side of the circle
  long m_xColorant;
  long m_yColorant;
  bool m_InvertX;// invert the meaning of the R axis
  bool m_InvertY;// invert the meaning of the R axis

  AnimBitmap m_spectrum;// just the spectrum
  bool m_bSpectrumDirty;// if true, we need to recalc.

  AnimBitmap m_offscreen;// always same size as the window (unless we are in low-res mode)
  bool m_bOffscreenDirty;// this will be true when we need to redraw non-spectrum stuff like selection.

  // the color we are basing the spectrum on
  ColorSpec m_col;

  HWND m_hNotify;// the window that we notify

  // selected color
  ColorSpec m_sel;
  bool m_AllowSelection;

};
