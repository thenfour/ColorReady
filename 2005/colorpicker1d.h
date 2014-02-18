/*
  AXES NAMES:
  Since this can be vertical or horizontal orientation, I can't refer to most axes as v or h.
  I will refer to them as primary and adjascent.
*/

#pragma once


#include "animbitmap.h"
#include "colors.h"
#include "geom.h"
#include "colorcontrols.h"


using namespace Colors;


class ColorPicker1D : public ColorPickerBase<ColorPicker1D, 1>
{
public:
  DECLARE_WND_CLASS("ColorPicker1D");

  // used in SetOrientation()
  static const long Vertical = 1;
  static const long Horizontal= 2;

  // axis id's (indexes of m_Colorant)
  static const long AxisPrimary = 0;

  ColorPicker1D(ColorManager* pmgr) :
    ColorPickerBase<ColorPicker1D, 1>(pmgr),
    m_pColorant(ColorPickerBase<ColorPicker1D, 1>::m_Colorant[0])
  {
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
        if((i != m_pColorant))
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

  // p is client coords
  void PositionFromColor(CPoint& ret, const Colorant& p) const
  {
    CSize s = _CalcSpectrumSizeNoBorder();

    if(m_Orientation == Vertical)
    {
      ret.x = m_size.cx / 2;
      ret.y = static_cast<long>(p * s.cy);
      ret.y += m_padding + m_bordersize;
    }
    else
    {
      ret.y = m_size.cy / 2;
      ret.x = static_cast<long>(p * s.cx);
      ret.x += m_padding + m_bordersize;
    }
  }

  // p is client coords
  // returns false if there is an error.  c must be the same colorspace as m_col
  bool PositionFromColor(CPoint& p, const ColorSpec& c) const
  {
    bool r = false;
    if(ColorIsInSpectrum(c))
    {
      PositionFromColor(p, c.GetColorant(m_pColorant));
      r = true;
    }
    return r;
  }

  void SetOrientation(long o)
  {
    m_Orientation = o;
  }

  // uses only the primary axis
  bool ColorFromPosition(ColorSpec& ret, long p) const
  {
    ret = m_col;
    CSize s = _CalcSpectrumSizeNoBorder();
    // adjust x and y so they are in the spectrum (clamp out of bounds too
    p -= m_padding + m_bordersize;

    if(m_Orientation == Vertical)
    {
      clamp(p, 0, s.cy);
      ret.GetColorant(m_pColorant) = static_cast<float>(p) / s.cy;
    }
    else
    {
      clamp(p, 0, s.cx);
      ret.GetColorant(m_pColorant) = static_cast<float>(p) / s.cx;
    }

    return true;
  }

  // color from client coords
  bool ColorFromPosition(ColorSpec& ret, long x, long y) const
  {
    if(m_Orientation == Vertical)
    {
      return ColorFromPosition(ret, y);
    }

    return ColorFromPosition(ret, x);
  }

protected:
  void _GenerateSpectrum()
  {
    CSize wholething = _CalcSpectrumSizeWithBorder();
    m_spectrum.SetSize(wholething.cx, wholething.cy);
    m_spectrum.BeginDraw();
    m_spectrum.Fill(m_bordercolor);

    // draw the spectrum
    CSize SpectrumSize = _CalcSpectrumSizeNoBorder();
    ColorSpec c = m_col;
    if(m_Orientation == Vertical)
    {
      for(long y = 0; y < SpectrumSize.cy; ++ y)
      {
        c.GetColorant(m_pColorant) = static_cast<float>(y) / SpectrumSize.cy;
        m_spectrum.HLine(m_bordersize, m_bordersize + SpectrumSize.cx, m_bordersize + y, c.GetRGBFast());
      }
    }
    else
    {
      for(long x = 0; x < SpectrumSize.cx; ++ x)
      {
        c.GetColorant(m_pColorant) = static_cast<float>(x) / SpectrumSize.cx;
        m_spectrum.VLine(m_bordersize + x, m_bordersize, m_bordersize + SpectrumSize.cy, c.GetRGBFast());
      }
    }

    m_spectrum.Commit();
  }

  void _DrawSelectionHandle()
  {
  }

  long m_Orientation;
  long& m_pColorant;
};