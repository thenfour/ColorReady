/*
  This is a control for picking a color from a 2-d CIRCULAR window.  We will support
  all color spaces.  The circle is always displayed in full.

  Interface is similar to colorpicker2d

  todo:
  -------------------------------------------
  done. -) catch WM_BEGINSIZING and WM_ENDSIZING or whatever - and just stretch the bitmap
  done. -) dont recalc the window on every WM_PAINT, if its not dirty just blit
  done. -) use DIBSections.
  done. -) change m_lowres to m_offscreen so we can handle painting the selection marks
  done. -) support CW2DN_MOUSEMOVE
  done. -) support CW2DN_SELCHANGED
  done. -) support antialiasing on the border
  done. -) support invert R axis
  done. -) support rotation
  done. -) draw selection handle.
  done. -) use lookup tables
        -) optimize selection handle drawing
*/


#include "stdafx.h"
#include "colorwheel2d.h"


using namespace Colors;


ColorWheel2D::ColorWheel2D(ColorManager* pmgr) :
  m_col(pmgr),
  m_pmgr(pmgr),
  m_sel(pmgr),
  m_aColorant(0),
  m_rColorant(1),
  m_borderwidth(DefaultBorderWidth),
  m_padding(DefaultPadding),
  m_bLowRes(false),
  m_bSpectrumDirty(true),
  m_bOffscreenDirty(true),
  m_InvertR(true),
  m_InvertA(false),
  m_Rotation(0),
  m_hNotify(0),
  m_havecapture(false)
{
  m_bordercolor = DefaultBorderColor();
  m_backcolor = DefaultBackColor();
  m_selhandlecolor = MakeRgbPixel(0,0,0);
}

void ColorWheel2D::OnCaptureChanged(HWND h)
{
  if(m_havecapture)
  {
    ReleaseCapture();
    m_havecapture = false;
  }
}

void ColorWheel2D::OnLButtonDown(UINT wParam, WTL::CPoint& p)
{
  SetCapture();
  m_havecapture = true;
  ColorSpec s;
  ColorFromPosition(s, p.x, p.y);
  SetSelectedColor(s);
}

void ColorWheel2D::OnLButtonUp(UINT wParam, WTL::CPoint& p)
{
  if(m_havecapture)
  {
    ReleaseCapture();
  }
}

void ColorWheel2D::OnMouseMove(UINT wParam, WTL::CPoint& p)
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

LRESULT ColorWheel2D::OnEraseBkgnd(HDC dc)
{
  return 1;
}

LRESULT ColorWheel2D::OnCreate(LPCREATESTRUCT p)
{
  m_col.InitNew(CS_RGB);
  m_hNotify = GetParent();
  return 0;
}

void ColorWheel2D::SetNotify(HWND h)
{
  m_hNotify = h;
}

void ColorWheel2D::OnPaint(HDC)
{
  PAINTSTRUCT ps;
  BeginPaint(&ps);
  HDC h = GetDC();
  _Draw(h);
  ReleaseDC(h);
  EndPaint(&ps);
}

void ColorWheel2D::OnEnterSizeMove()
{
  m_bLowRes = true;
}

void ColorWheel2D::OnExitSizeMove()
{
  m_bLowRes = false;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::OnSize(UINT wParam, const WTL::CSize& s)
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
    float delta = (float)(_CalcSpectrumDiameterWithBorder() - m_spectrum.GetHeight());
    // determine delta as % of original
    float dp = abs(delta / m_spectrum.GetHeight());
    if(dp > 0.1)
    {
      _SetSpectrumDirty();
    }
  }
}

bool ColorWheel2D::ColorIsInSpectrum(const ColorSpec& c) const
{
  bool r = false;
  if(c.GetColorSpaceID() == m_col.GetColorSpaceID())
  {
    // all the colorants that we do not use - make sure they are equal
    long i;
    r = true;
    for(i = 0; i < c.GetColorantCount(); i ++)
    {
      if((i != m_rColorant) && (i != m_aColorant))
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

ColorSpec ColorWheel2D::GetSelectedColor() const
{
  return m_sel;
}

bool ColorWheel2D::SetSelectedColor(ColorSpec& s, bool redraw)
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

void ColorWheel2D::SetAxes(long a, long r)
{
  m_aColorant = a;
  m_rColorant = r;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetInvertR(bool x)
{
  if(m_InvertR != x)
  {
    m_InvertR = x;
    _SetSpectrumDirty();
    Invalidate(0);
  }
}

void ColorWheel2D::SetInvertA(bool x)
{
  if(m_InvertA != x)
  {
    m_InvertA = x;
    _SetSpectrumDirty();
    Invalidate(0);
  }
}

// x is from 0-1.0, which is the whole circle.
void ColorWheel2D::SetRotation(float x)
{
  m_Rotation = x;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetColor(const ColorSpec& val)
{
  m_sel = val;
  m_col = val;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetColorant(long i, const Colorant& v)
{
  if((i != m_rColorant) && (i != m_aColorant))
  {
    m_col.GetColorant(i) = v;
    m_sel.GetColorant(i) = v;
    _SetSpectrumDirty();
    Invalidate(0);
  }
}

void ColorWheel2D::SetBackground(RgbPixel c)
{
  m_backcolor = c;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetBorder(long width)
{
  m_borderwidth = width;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetBorder(RgbPixel c, long width)
{
  m_bordercolor = c;
  m_borderwidth = width;
  _SetSpectrumDirty();
  Invalidate(0);
}

void ColorWheel2D::SetPadding(long width)
{
  m_padding = width;
  _SetSpectrumDirty();
  Invalidate(0);
}

// color from client coords
bool ColorWheel2D::ColorFromPosition(ColorSpec& ret, long x, long y) const
{
  bool r = false;
  long radius = _CalcSpectrumRadius();
  if(radius < 1)
  {
    ret = m_col;
    r = true;
  }
  else
  {
    x -= m_size.cx / 2;
    y -= m_size.cy / 2;

    ret = m_col;

    Colorant cr = m_rlut.GetAdHocRadius(x, y);
    clamp(cr, 0, radius);
    cr /= radius;
    if(m_InvertR) cr = 1.0f - cr;
    ret.GetColorant(m_rColorant) = cr;

    Colorant ca = m_alut.GetAdHocAngle(x, y, 1, m_Rotation);
    if(m_InvertA) ca = 1.0f - ca;
    ret.GetColorant(m_aColorant) = ca;

    r = true;
  }

  return r;
}

// p is client coords
void ColorWheel2D::PositionFromColor(CPoint& p, const Colorant& ca, const Colorant& cr) const
{
  // determine a and r.
  Colorant catemp = ca;
  if(m_InvertA) catemp = 1.0f - catemp;

  // get the radius first
  Colorant crtemp = cr;
  if(m_InvertR) crtemp = 1.0f - crtemp;
  crtemp = crtemp * _CalcSpectrumRadius();

  // now get X and Y from that
  m_alut.GetAdHocPosition(p.x, p.y, catemp, crtemp, 1.0f, m_Rotation);

  p.x += m_size.cx / 2;
  p.y += m_size.cy / 2;
}

// p is client coords
// returns false if there is an error.  c must be the same colorspace as m_col
bool ColorWheel2D::PositionFromColor(CPoint& p, const ColorSpec& c) const
{
  bool r = false;
  if(ColorIsInSpectrum(c))
  {
    PositionFromColor(p, c.GetColorant(m_aColorant), c.GetColorant(m_rColorant));
    r = true;
  }
  return r;
}

void ColorWheel2D::__DrawSelectionHandleHLine(long x1, long x2, long y)
{
  m_offscreen.HLine(x1, x2+1, y, m_selhandlecolor);
}

void ColorWheel2D::__DrawSelectionHandleAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  RgbPixel back;
  if(m_offscreen.GetPixelSafe(back, cx+x, cy+y)) m_offscreen.SetPixelSafe(cx+x, cy+y, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
  if(m_offscreen.GetPixelSafe(back, cx+x, cy-y-1)) m_offscreen.SetPixelSafe(cx+x, cy-y-1, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
  if(m_offscreen.GetPixelSafe(back, cx-x-1, cy+y)) m_offscreen.SetPixelSafe(cx-x-1, cy+y, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
  if(m_offscreen.GetPixelSafe(back, cx-x-1, cy-y-1)) m_offscreen.SetPixelSafe(cx-x-1, cy-y-1, Colors::MixColorsInt(f, fmax, m_selhandlecolor, back));
}

void ColorWheel2D::__DrawBorderHLine(long x1, long x2, long y)
{
  m_spectrum.HLine(x1, x2+1, y, m_bordercolor);
}

void ColorWheel2D::__DrawBorderAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  m_spectrum.SetPixel(cx+x, cy+y, Colors::MixColorsInt(f, fmax, m_bordercolor, m_backcolor));
  m_spectrum.SetPixel(cx+x, cy-y-1, Colors::MixColorsInt(f, fmax, m_bordercolor, m_backcolor));
  m_spectrum.SetPixel(cx-x-1, cy+y, Colors::MixColorsInt(f, fmax, m_bordercolor, m_backcolor));
  m_spectrum.SetPixel(cx-x-1, cy-y-1, Colors::MixColorsInt(f, fmax, m_bordercolor, m_backcolor));
}

void ColorWheel2D::__DrawSpectrumHLine(long x1, long x2, long y)
{
  for(long x = x1; x <= x2; ++x)
  {
    Colorant& cr(m_col.GetColorant(m_rColorant));
    cr = m_rlut.GetRadius(x-__m_cx, y-__m_cy) / __m_r;
    if(m_InvertR) cr = 1.0f - cr;
    clamp(cr, 0, 1.0);

    Colorant& ca(m_col.GetColorant(m_aColorant));
    Colorant a = m_alut.GetAngle(x-__m_cx, y-__m_cy);

    ca = a;
    if(ca > 1) ca -= 1;
    if(ca < 0) ca = 1.00f - ca;
    if(m_InvertA) ca = 1.0f - ca;
    m_spectrum.SetPixel(x, y, m_col.GetRGBFast());
  }
}

void ColorWheel2D::__DrawSpectrumAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  Colorant& ca(m_col.GetColorant(m_aColorant));
  Colorant& cr(m_col.GetColorant(m_rColorant));

  // set the radius colorant to this current radius
  cr = m_rlut.GetRadius(x, y) / __m_r;
  if(m_InvertR) cr = 1.0f - cr;
  ca = m_alut.GetAngle(x, y);
  if(m_InvertA) ca = 1.0f - ca;
  m_spectrum.SetPixel(cx+x, cy+y, Colors::MixColorsInt(f, fmax, m_col.GetRGBFast(), m_spectrum.GetPixel(cx+x, cy+y)));
  ca = m_alut.GetAngle(-x-1, y);
  if(m_InvertA) ca = 1.0f - ca;
  m_spectrum.SetPixel(cx-x-1, cy+y, Colors::MixColorsInt(f, fmax, m_col.GetRGBFast(), m_spectrum.GetPixel(cx-x-1, cy+y)));
  ca = m_alut.GetAngle(-x-1, -y-1);
  if(m_InvertA) ca = 1.0f - ca;
  m_spectrum.SetPixel(cx-x-1, cy-y-1, Colors::MixColorsInt(f, fmax, m_col.GetRGBFast(), m_spectrum.GetPixel(cx-x-1, cy-y-1)));
  ca = m_alut.GetAngle(x, -y-1);
  if(m_InvertA) ca = 1.0f - ca;
  m_spectrum.SetPixel(cx+x, cy-y-1, Colors::MixColorsInt(f, fmax, m_col.GetRGBFast(), m_spectrum.GetPixel(cx+x, cy-y-1)));
}

void ColorWheel2D::_DrawSelectionHandle()
{
  CPoint p;// coords of the selection.
  PositionFromColor(p, m_sel.GetColorant(m_aColorant), m_sel.GetColorant(m_rColorant));
  DonutAAG(p.x, p.y, 3, 2, this, &ColorWheel2D::__DrawSelectionHandleHLine, this, &ColorWheel2D::__DrawSelectionHandleAlphaPixel);

  // draw the ticks
  //long i;
  //for(i = 1; i < 6; i ++)
  //{
  //  m_offscreen.SetPixelSafe(p.x - 1 - i, p.y, m_selhandlecolor);
  //  m_offscreen.SetPixelSafe(p.x + 1 + i, p.y, m_selhandlecolor);
  //  m_offscreen.SetPixelSafe(p.x, p.y - 1 - i, m_selhandlecolor);
  //  m_offscreen.SetPixelSafe(p.x, p.y + 1 + i, m_selhandlecolor);
  //}
}

void ColorWheel2D::_DrawError(HDC hdc)
{
  WTL::CBrush i;
  CRect rc;
  GetClientRect(&rc);
  i.CreateSolidBrush(m_backcolor);
  TextOut(hdc, 0, 0, "Error", 5);
  FillRect(hdc, &rc, i);
  i.DeleteObject();
}

// includes the border, but no padding.
long ColorWheel2D::_CalcSpectrumDiameterWithBorder() const
{
  long r = (min(m_size.cx, m_size.cy) - m_padding) - m_padding;
  r &= ~1;// remove bit 1 to make it even number.
  return r;
}

// does not include border or padding
long ColorWheel2D::_CalcSpectrumRadius() const
{
  long r = ((min(m_size.cx, m_size.cy) / 2) - m_padding) - m_borderwidth;
  r &= ~1;// remove bit 1 to make it even number.
  return r;
}

void ColorWheel2D::_GenerateSpectrum()
{
  long sd = _CalcSpectrumDiameterWithBorder();
  m_spectrum.SetSize(sd + 4, sd + 4);
  m_spectrum.BeginDraw();
  m_spectrum.Fill(m_backcolor);
  long c = m_spectrum.GetHeight() / 2;// center point

  __m_cx = c;
  __m_cy = c;

  // draw the border
  if(m_borderwidth)
  {
    FilledCircleAAG(c, c, sd/2, this, &ColorWheel2D::__DrawBorderHLine, this, &ColorWheel2D::__DrawBorderAlphaPixel);
  }

  __m_r = _CalcSpectrumRadius();
  if(__m_r > 0)
  {
    m_alut.Resize(__m_r+1, 1, m_Rotation);
    m_rlut.Resize(__m_r+1, __m_r+1);
    // draw the spectrum
    FilledCircleAAG(c, c, __m_r, this, &ColorWheel2D::__DrawSpectrumHLine, this, &ColorWheel2D::__DrawSpectrumAlphaPixel);
  }
  m_spectrum.Commit();
}

void ColorWheel2D::_Draw(HDC hdc)
{
  bool bError = true;
  // make sure we are good to go.
  if(m_aColorant < m_col.GetColorantCount())
  {
    if(m_rColorant < m_col.GetColorantCount())
    {
      if(m_col.UsesColorants())
      {
        // make sure we can actually draw the thing.
        long minsize = _CalcSpectrumDiameterWithBorder() + m_padding + m_padding;
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
      long destx = (m_size.cx - m_spectrum.GetHeight()) / 2;
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
      long destside = _CalcSpectrumDiameterWithBorder() + 4;
      long destx = (m_size.cx - destside) / 2;
      long desty = (m_size.cy - destside) / 2;
      m_spectrum.StretchBlit(m_offscreen, destx, desty, destside, destside);
    }

    m_offscreen.Blit(hdc, 0, 0);
  }
}

// call this when selection changes, or any other non-spectrum stuff
void ColorWheel2D::_SetOffscreenDirty()
{
  m_bOffscreenDirty = true;
}

// call this when spectrum needs to be redrawn.
void ColorWheel2D::_SetSpectrumDirty()
{
  m_bSpectrumDirty = true;
  _SetOffscreenDirty();
}
