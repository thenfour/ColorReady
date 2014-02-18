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

#pragma once


#include "animbitmap.h"
#include "colors.h"
#include <math.h>
#include "geom.h"
#include "polarlut.h"
#include "colorcontrols.h"


using namespace Colors;


class ColorWheel2D :
  public CWindowImpl<ColorWheel2D, CWindow, ColorPickerTraits>
{
public:
  DECLARE_WND_CLASS("ColorWheel2D");

  static const long DefaultPadding = 6;
  static const long DefaultBorderWidth = 3;
  inline static RgbPixel DefaultBorderColor() { return MakeRgbPixel(255,220,190); }
  inline static RgbPixel DefaultBackColor() { return MakeRgbPixel(96,90,130); }

  ColorWheel2D(ColorManager* pmgr);

  BEGIN_MSG_MAP_EX(ColorWheel2D)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_SIZE(OnSize)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_CAPTURECHANGED(OnCaptureChanged)
  END_MSG_MAP()

  void OnCaptureChanged(HWND h);
  void OnLButtonDown(UINT wParam, WTL::CPoint& p);
  void OnLButtonUp(UINT wParam, WTL::CPoint& p);
  void OnMouseMove(UINT wParam, WTL::CPoint& p);
  LRESULT OnEraseBkgnd(HDC dc);
  LRESULT OnCreate(LPCREATESTRUCT p);
  void SetNotify(HWND h);
  void OnPaint(HDC);
  void OnEnterSizeMove();
  void OnExitSizeMove();
  void OnSize(UINT wParam, const WTL::CSize& s);
  bool ColorIsInSpectrum(const ColorSpec& c) const;
  ColorSpec GetSelectedColor() const;
  bool SetSelectedColor(ColorSpec& s, bool redraw = true);
  void SetAxes(long a, long r);
  void SetInvertR(bool x);
  void SetInvertA(bool x);
  void SetRotation(float x);
  void SetColor(const ColorSpec& val);
  void SetColorant(long i, const Colorant& v);
  void SetBackground(RgbPixel c);
  void SetBorder(long width);
  void SetBorder(RgbPixel c, long width);
  void SetPadding(long width);
  bool ColorFromPosition(ColorSpec& ret, long x, long y) const;
  void PositionFromColor(CPoint& p, const Colorant& ca, const Colorant& cr) const;
  bool PositionFromColor(CPoint& p, const ColorSpec& c) const;
  void __DrawSelectionHandleHLine(long x1, long x2, long y);
  void __DrawSelectionHandleAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);
  void __DrawBorderHLine(long x1, long x2, long y);
  void __DrawBorderAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);
  void __DrawSpectrumHLine(long x1, long x2, long y);
  void __DrawSpectrumAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);

private:

  void _DrawSelectionHandle();
  void _DrawError(HDC hdc);
  long _CalcSpectrumDiameterWithBorder() const;
  long _CalcSpectrumRadius() const;
  void _GenerateSpectrum();
  void _Draw(HDC hdc);
  void _SetOffscreenDirty();
  void _SetSpectrumDirty();

  ColorManager* m_pmgr;

  bool m_bLowRes;// for when the user is sizing or something and we dont want to recalculate the spectrum every time

  CSize m_size;// window size
  long m_borderwidth;
  RgbPixel m_backcolor;
  RgbPixel m_bordercolor;
  RgbPixel m_selhandlecolor;
  long m_padding;// empty space on either side of the circle
  long m_aColorant;// axis
  long m_rColorant;// radius
  bool m_InvertR;// invert the meaning of the R axis
  bool m_InvertA;// invert the meaning of the A axis
  float m_Rotation;

  AnimBitmap m_spectrum;// just the spectrum
  bool m_bSpectrumDirty;// if true, we need to recalc.
  // lookup tables
  AngleLut<float> m_alut;
  RadiusLut<float> m_rlut;

  AnimBitmap m_offscreen;// always same size as the window (unless we are in low-res mode)
  bool m_bOffscreenDirty;// this will be true when we need to redraw non-spectrum stuff like selection.

  // the color we are basing the spectrum on
  ColorSpec m_col;

  HWND m_hNotify;// the window that we notify

  // selected color
  ColorSpec m_sel;
  bool m_AllowSelection;

  // misc stuff for passing around the spectrum drawing callbacks
  long __m_r;
  long __m_cx;
  long __m_cy;

  bool m_havecapture;
};
