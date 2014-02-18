

#pragma once


const WORD CPN_MOUSEMOVE = 100;
const WORD CPN_SELCHANGED = 101;

struct CPNMouseMoveStruct
{
  NMHDR hdr;
  CPoint p;
};

typedef CWinTraits<WS_CHILD, 0> ColorPickerTraits;


using namespace Colors;


template<typename Tderived, long Taxes>
class ColorPickerBase :
  public CWindowImpl<Tderived, CWindow, ColorPickerTraits>
{
public:
  //DECLARE_WND_CLASS("ColorPicker1D");

  static const long DefaultPadding = 6;
  static const long DefaultBorderWidth = 4;
  inline static RgbPixel DefaultBorderColor() { return MakeRgbPixel(255,220,190); }
  inline static RgbPixel DefaultBackColor() { return MakeRgbPixel(96,90,130); }

  ColorPickerBase(ColorManager* pmgr) :
    m_col(pmgr),
    m_pmgr(pmgr),
    m_sel(pmgr),
    m_bordersize(DefaultBorderWidth),
    m_padding(DefaultPadding),
    m_bLowRes(false),
    m_bSpectrumDirty(true),
    m_bOffscreenDirty(true),
    m_hNotify(0),
    m_havecapture(false)
  {
    m_bordercolor = DefaultBorderColor();
    m_backcolor = DefaultBackColor();
    m_selhandlecolor = MakeRgbPixel(0,0,0);
  }

  virtual ~ColorPickerBase()
  {
  }

  BEGIN_MSG_MAP_EX(Tderived)
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

  void OnLButtonDown(UINT wParam, CPoint& p)
  {
    SetCapture();
    m_havecapture = true;
    ColorSpec s;
    ColorFromPosition(s, p.x, p.y);
    SetSelectedColor(s);
  }

  void OnLButtonUp(UINT wParam, CPoint& p)
  {
    if(m_havecapture)
    {
      ReleaseCapture();
    }
  }

  void OnMouseMove(UINT wParam, CPoint& p)
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

  void OnSize(UINT wParam, const CSize& s)
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

  virtual bool ColorIsInSpectrum(const ColorSpec& c) const = 0;

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

  void SetAxis(long axisID, long p)
  {
    m_Colorant[axisID] = p;
  }

  void SetInvertAxis(long axisID, bool v)
  {
    if(m_Invert[axisID] != v)
    {
      m_Invert[axisID] = v;
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

  bool IsManagedColorant(long iColorant) const
  {
    for(long i = 0; i < Taxes; ++ i)
    {
      if(m_Colorant[i] == iColorant)
      {
        return true;
      }
    }
    return false;
  }

  void SetColorant(long i, const Colorant& v)
  {
    if(!IsManagedColorant(i))
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
  virtual bool ColorFromPosition(ColorSpec& ret, long x, long y) const = 0;

  // p is client coords
  //virtual void PositionFromColor(CPoint& p, const Colorant& cx, const Colorant& cy) const = 0;

  // p is client coords
  // returns false if there is an error.  c must be the same colorspace as m_col
  virtual bool PositionFromColor(CPoint& p, const ColorSpec& c) const = 0;


protected:

  virtual void _DrawSelectionHandle() = 0;

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

  virtual void _GenerateSpectrum() = 0;

  void _Draw(HDC hdc)
  {
    bool bError = false;

    if(!m_col.UsesColorants())
    {
      bError = true;
    }
    else
    {
      for(int i = 0; i < Taxes; ++ i)
      {
        if(i >= m_col.GetColorantCount())
        {
          bError = true;
          break;
        }
      }
    }

    if(!bError)
    {
      // make sure we can actually draw the thing.
      long minsize = 2 + (m_bordersize * 2) + (m_padding * 2);
      if((m_size.cx < minsize) || (m_size.cy < minsize))
      {
        bError = true;
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

  void _SetOffscreenDirty()
  {
    m_bOffscreenDirty = true;
  }

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

  long m_Colorant[Taxes];
  bool m_Invert[Taxes];// invert the meaning of the R axis

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
