

#include "StdAfx.h"
#include ".\colorspot.h"


ColorSpot::ColorSpot(void)
{
}

ColorSpot::~ColorSpot(void)
{
}

void ColorSpot::OnPaint(HDC)
{
  PAINTSTRUCT ps;
  BeginPaint(&ps);
  FillRect(ps.hdc, &ps.rcPaint, m_brush);
  EndPaint(&ps);
}

void ColorSpot::SetColor(const ColorSpec& c)
{
  m_color = c;
  m_brush.CreateSolidBrush(RgbPixelToCOLORREF(c.GetRGBFast()));
}

