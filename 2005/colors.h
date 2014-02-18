// the MAIN API header.

#pragma once


#include "colorframework.h"
#include "clrconv.h"


namespace Colors
{
  // types of colors - these define the meaning of the data for a color.
  const ColorSpaceID CS_RGB = 1;             // done.
  const ColorSpaceID CS_CIE1931 = 2;             // needs correct matrix
  //const ColorSpaceID CS_xyY = 3;             // -
  //const ColorSpaceID CS_HunterLab = 4;       // -
  //const ColorSpaceID CS_Lab = 5;             // -
  //const ColorSpaceID CS_LCh = 6;             // -
  //const ColorSpaceID CS_Luv = 7;             // -
  const ColorSpaceID CS_HSL = 8;             // -
  const ColorSpaceID CS_HSV = 9;             // done.
  const ColorSpaceID CS_CMY = 10;            // -
  const ColorSpaceID CS_CMYK = 10;           // -
  //const ColorSpaceID CS_CMYKOG = 12;         // -
  //const ColorSpaceID CS_YIQ = 13;            // done.
  //const ColorSpaceID CS_YUV = 18;            // done.
  const ColorSpaceID CS_Gray = 14;           // done.
  //const ColorSpaceID CS_IndustryPalette = 15;// -      DwordData[0] = palette ID, DwordData[1] = index
  //const ColorSpaceID CS_WindowsPalette = 16; // -      reference to user's current color scheme entries
  const ColorSpaceID CS_HWB = 17;            // done.  hue whiteness blackness, from http://www.acm.org/jgt/papers/SmithLyons96/

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGB
  inline void __stdcall RGBInitNew(ColorData& c)
  {
    c.m_Colorants[0] = 0;
    c.m_Colorants[1] = 0;
    c.m_Colorants[2] = 0;
  }

  inline ConversionResult __stdcall RGBConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    switch(destid)
    {
    case CS_HWB:
      RGBToHWB(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    case CS_HSV:
      RGBToHSV(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    //case CS_YIQ:
    //  return RGBToYIQ(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
    case CS_Gray:
      RGBToGray(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    //case CS_YUV:
    //  return RGBToYUV(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
    case CS_CIE1931:
      return RGBToCIE1931(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
    }
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall RGBToRGBFast(const ColorData& dat)
  {
    return MakeRgbPixel(dat.m_Colorants[0] * 255, dat.m_Colorants[1] * 255, dat.m_Colorants[2] * 255);
  }

  inline ColorSpaceInfo RGBGetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_RGB;
    r.nColorants = 3;
    r.bUsesColorants = true;
    r.Name = "RGB";
    r.Description = "RGB Colorspace, used by computer images and monitors";
    r.Colorants.push_back(ColorantInfo("R", "Red", "Red component"));
    r.Colorants.push_back(ColorantInfo("G", "Green", "Green component"));
    r.Colorants.push_back(ColorantInfo("B", "Blue", "Blue component"));
    r.pConvertTo = RGBConvertTo;
    r.pToRGBFast = RGBToRGBFast;
    r.pInitNew = RGBInitNew;
    return r;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // HWB
  inline void __stdcall HWBInitNew(ColorData& c)
  {
    c.m_Colorants[0] = 0;
    c.m_Colorants[1] = 0;
    c.m_Colorants[2] = 0;
  }

  inline ConversionResult __stdcall HWBConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    switch(destid)
    {
    case CS_RGB:
      HWBToRGB(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    }
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall HWBToRGBFast(const ColorData& dat)
  {
    Colorant rgb[3];
    HWBToRGB(dat.m_Colorants, rgb);
    return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  }

  inline ColorSpaceInfo HWBGetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_HWB;
    r.nColorants = 3;
    r.bUsesColorants = true;
    r.Name = "HWB";
    r.Description = "HWB Colorspace, 'Hue, Whiteness, Blackness', by Alvy Ray Smith and Eric Ray Lyons.";
    r.Colorants.push_back(ColorantInfo("H", "Hue", "Hue component"));
    r.Colorants.push_back(ColorantInfo("W", "Whiteness", "Whiteness component"));
    r.Colorants.push_back(ColorantInfo("B", "Blackness", "Blackness component"));
    r.pConvertTo = HWBConvertTo;
    r.pToRGBFast = HWBToRGBFast;
    r.pInitNew = HWBInitNew;
    return r;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // HSV
  inline void __stdcall HSVInitNew(ColorData& c)
  {
    c.m_Colorants[0] = 0;
    c.m_Colorants[1] = 0;
    c.m_Colorants[2] = 0;
  }

  inline ConversionResult __stdcall HSVConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    switch(destid)
    {
    case CS_RGB:
      HSVToRGB(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    case CS_Gray:
      dat.m_Colorants[0] = dat.m_Colorants[2];
      return CR_InGamut;
    }
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall HSVToRGBFast(const ColorData& dat)
  {
    Colorant rgb[3];
    HSVToRGB(dat.m_Colorants, rgb);
    return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  }

  inline ColorSpaceInfo HSVGetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_HSV;
    r.nColorants = 3;
    r.bUsesColorants = true;
    r.Name = "HSV";
    r.Description = "HSV Colorspace, 'Hue, Saturation, Value (brightness)'";
    r.Colorants.push_back(ColorantInfo("H", "Hue", "Hue component"));
    r.Colorants.push_back(ColorantInfo("S", "Saturation", "Saturation component"));
    r.Colorants.push_back(ColorantInfo("V", "Value (brightness)", "Value (brightness) component"));
    r.pConvertTo = HSVConvertTo;
    r.pToRGBFast = HSVToRGBFast;
    r.pInitNew = HSVInitNew;
    return r;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // YIQ
  //inline void __stdcall YIQInitNew(ColorData& c)
  //{
  //  c.m_Colorants[0] = 0;
  //  c.m_Colorants[1] = 0;
  //  c.m_Colorants[2] = 0;
  //}

  //inline ConversionResult __stdcall YIQConvertTo(ColorSpaceID destid, ColorData& dat)
  //{
  //  switch(destid)
  //  {
  //  case CS_RGB:
  //    return YIQToRGB(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
  //  }
  //  return CR_ConversionFailed;
  //}

  //inline RgbPixel __stdcall YIQToRGBFast(const ColorData& dat)
  //{
  //  Colorant rgb[3];
  //  YIQToRGB(dat.m_Colorants, rgb);
  //  return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  //}

  //inline ColorSpaceInfo YIQGetInfo()
  //{
  //  ColorSpaceInfo r;
  //  r.id = CS_YIQ;
  //  r.nColorants = 3;
  //  r.bUsesColorants = true;
  //  r.Name = "YIQ";
  //  r.Description = "YIQ Colorspace";
  //  r.Colorants.push_back(ColorantInfo("Y", "Y (intensity)", "?"));
  //  r.Colorants.push_back(ColorantInfo("I", "In-phase", "?"));
  //  r.Colorants.push_back(ColorantInfo("Q", "Quadrature", "?"));
  //  r.pConvertTo = YIQConvertTo;
  //  r.pToRGBFast = YIQToRGBFast;
  //  r.pInitNew = YIQInitNew;
  //  return r;
  //}

  //////////////////////////////////////////////////////////////////////////////////////////
  // Gray
  inline void __stdcall GrayInitNew(ColorData& c)
  {
    c.m_Colorants[0] = 0;
  }

  inline ConversionResult __stdcall GrayConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    switch(destid)
    {
    case CS_RGB:
      GrayToRGB(dat.m_Colorants, dat.m_Colorants);
      return CR_InGamut;
    }
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall GrayToRGBFast(const ColorData& dat)
  {
    Colorant rgb[3];
    GrayToRGB(dat.m_Colorants, rgb);
    return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  }

  inline ColorSpaceInfo GrayGetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_Gray;
    r.nColorants = 1;
    r.bUsesColorants = true;
    r.Name = "Gray";
    r.Description = "Gray Colorspace";
    r.Colorants.push_back(ColorantInfo("B", "Brightness", "?"));
    r.pConvertTo = GrayConvertTo;
    r.pToRGBFast = GrayToRGBFast;
    r.pInitNew = GrayInitNew;
    return r;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // YUV
  //inline void __stdcall YUVInitNew(ColorData& c)
  //{
  //  c.m_Colorants[0] = 0;
  //  c.m_Colorants[1] = 0;
  //  c.m_Colorants[2] = 0;
  //}

  //inline ConversionResult __stdcall YUVConvertTo(ColorSpaceID destid, ColorData& dat)
  //{
  //  switch(destid)
  //  {
  //  case CS_RGB:
  //    return YUVToRGB(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
  //  }
  //  return CR_ConversionFailed;
  //}

  //inline RgbPixel __stdcall YUVToRGBFast(const ColorData& dat)
  //{
  //  Colorant rgb[3];
  //  YUVToRGB(dat.m_Colorants, rgb);
  //  return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  //}

  //inline ColorSpaceInfo YUVGetInfo()
  //{
  //  ColorSpaceInfo r;
  //  r.id = CS_YUV;
  //  r.nColorants = 3;
  //  r.bUsesColorants = true;
  //  r.Name = "YUV";
  //  r.Description = "YUV Colorspace";
  //  r.Colorants.push_back(ColorantInfo("B", "Brightness", "?"));
  //  r.Colorants.push_back(ColorantInfo("B", "Brightness", "?"));
  //  r.Colorants.push_back(ColorantInfo("B", "Brightness", "?"));
  //  r.pConvertTo = YUVConvertTo;
  //  r.pToRGBFast = YUVToRGBFast;
  //  r.pInitNew = YUVInitNew;
  //  return r;
  //}

  ////////////////////////////////////////////////////////////////////////////////////////
  // CIE 1931
  inline void __stdcall CIE1931InitNew(ColorData& c)
  {
    c.m_Colorants[0] = 0;
    c.m_Colorants[1] = 0;
    c.m_Colorants[2] = 0;
  }

  inline ConversionResult __stdcall CIE1931ConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    switch(destid)
    {
    case CS_RGB:
      return CIE1931ToRGB(dat.m_Colorants, dat.m_Colorants) ? CR_OutOfGamut : CR_InGamut;
    }
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall CIE1931ToRGBFast(const ColorData& dat)
  {
    Colorant rgb[3];
    CIE1931ToRGB(dat.m_Colorants, rgb);
    return MakeRgbPixel(rgb[0]*255, rgb[1]*255, rgb[2]*255);
  }

  inline ColorSpaceInfo CIE1931GetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_CIE1931;
    r.nColorants = 3;
    r.bUsesColorants = true;
    r.Name = "CIE 1931";
    r.Description = "Cie-XYZ 1931 Colorspace";
    r.Colorants.push_back(ColorantInfo("X", "?", "?"));
    r.Colorants.push_back(ColorantInfo("Y", "?", "?"));
    r.Colorants.push_back(ColorantInfo("Z", "?", "?"));
    r.pConvertTo = CIE1931ConvertTo;
    r.pToRGBFast = CIE1931ToRGBFast;
    r.pInitNew = CIE1931InitNew;
    return r;
  }
}
