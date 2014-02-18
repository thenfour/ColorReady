

#pragma once


#include "colors.h"


using namespace Colors;


typedef CWinTraits<WS_CHILD, 0> ColorSpotTraits;


class ColorSpot : public CWindowImpl<ColorSpot, CWindow, ColorSpotTraits>
{
public:
  BEGIN_MSG_MAP_EX(ColorSpot)
    //MSG_WM_CREATE(OnCreate)
    MSG_WM_PAINT(OnPaint)
  END_MSG_MAP()

  ColorSpot(void);
  ~ColorSpot(void);

  void SetColor(const ColorSpec& c);

  //LRESULT OnCreate(LPCREATESTRUCT p);
  void OnPaint(HDC);

private:
  ColorSpec m_color;
  WTL::CBrush m_brush;
};


