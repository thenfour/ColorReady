// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=bc584b4d.0110122010.1df94eb5%40posting.google.com
// all functions for converting colors.
// a few guidelines:
// 1) all colorants are from 0-1.
// 2) all functions must be capable of accepting the same pointer for both input and output, e.g. RGBToHWB(&mydat, &mydat) to do an in-place conversion
// 3) clamp values to 0-1 when a value is out of gamut, and return false.  conversions where this cant happen can be void

#include "colorframework.h"

namespace Colors
{
  /*
    Integer math color mixing function
  */
  inline RgbPixel MixColorsInt(long fa, long fmax, RgbPixel ca, RgbPixel cb)
  {
    BYTE r, g, b;
    long fmaxminusfa = fmax - fa;
    r = static_cast<BYTE>(((fa * R(ca)) + (fmaxminusfa * R(cb))) / fmax);
    g = static_cast<BYTE>(((fa * G(ca)) + (fmaxminusfa * G(cb))) / fmax);
    b = static_cast<BYTE>(((fa * B(ca)) + (fmaxminusfa * B(cb))) / fmax);
    return MakeRgbPixel(r,g,b);
  }

  template<typename T>
  inline T min3(const T& a, const T& b, const T& c)
  {
    return min(a,min(b,c));
  }

  template<typename T>
  inline T max3(const T& a, const T& b, const T& c)
  {
    return max(a,min(b,c));
  }

  template<typename T, typename Tmin, typename Tmax>
  inline bool clamp(T& l, const Tmin& minval, const Tmax& maxval)
  {
    if(l < static_cast<T>(minval))
    {
      l = static_cast<T>(minval);
      return true;
    }
    if(l > static_cast<T>(maxval))
    {
      l = static_cast<T>(maxval);
      return true;
    }
    return false;
  }

  // compare 2 floating point numbers to see if they are in a certain range.
  template<typename Tl, typename Tr, typename Terr>
  inline bool xequals(const Tl& l, const Tr& r, const Terr& e)
  {
    Tl d = l - r;
    if(d < 0) d = -d;
    if(d > e) return false;
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToHWB
  inline void RGBToHWB(const Colorant* rgb, Colorant* hwb)
  {
    Colorant R = rgb[0];
    Colorant G = rgb[1];
    Colorant B = rgb[2];
    Colorant w, v, b, f;
    long i;

    w = min3(R, G, B);
    v = max3(R, G, B);
    b = 1 - v;
    hwb[1] = w;
    hwb[2] = b;
    if (v == w)
    {
      hwb[0] = 0;
    }
    else
    {
      f = (R == w) ? G - B : ((G == w) ? B - R : R - G);  
      i = (R == w) ? 3 : ((G == w) ? 5 : 1);  
      hwb[0] = (i - f / (v - w)) / 6;
    }

    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // HWBToRGB
  inline void HWBToRGB(const Colorant* hwb, Colorant* rgb)
  {
    Colorant h = hwb[0]*6;
    Colorant w = hwb[1];
    Colorant b = hwb[2];
    Colorant v, n, f;  
    long i;  

    v = 1 - b;  
    i = (long)(h);
    f = h - i;  
    if (i & 1) f = 1 - f; // if i is odd  
    n = w + f * (v - w); // linear interpolation between w and v  
    switch (i)
    {  
    case 6:  
    case 0:
      rgb[0] = v;
      rgb[1] = n;
      rgb[2] = w;
      return;
    case 1:
      rgb[0] = n;
      rgb[1] = v;
      rgb[2] = w;
      return;
    case 2:
      rgb[0] = w;
      rgb[1] = v;
      rgb[2] = n;
      return;
    case 3:
      rgb[0] = w;
      rgb[1] = n;
      rgb[2] = v;
      return;
    case 4:
      rgb[0] = n;
      rgb[1] = w;
      rgb[2] = v;
      return;
    case 5:
      rgb[0] = v;
      rgb[1] = w;
      rgb[2] = n;
      return;
    }

    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToHSV
  inline void RGBToHSV(const Colorant* rgb, Colorant* hsv)
  {
    Colorant R = rgb[0];
    Colorant G = rgb[1];
    Colorant B = rgb[2];

	  Colorant v, x, f;
	  long i;
  	
	  x = min3(R, G, B);
	  v = max3(R, G, B);
    hsv[2] = v;
	  if(v == x)
    {
      hsv[0] = 0;
      hsv[1] = 0;
    }
    else
    {
	    f = (R == x) ? G - B : ((G == x) ? B - R : R - G);
	    i = (R == x) ? 3 : ((G == x) ? 5 : 1);
      hsv[0] = (i - f / (v - x)) / 6;
      hsv[1] = (v - x) / v;
    }

    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // HSVToRGB
  inline void HSVToRGB(const Colorant* hsv, Colorant* rgb)
  {
    Colorant h = hsv[0] * 6;
    Colorant s = hsv[1];
    Colorant v = hsv[2];
	  Colorant m, n, f;
	  long i;
	  i = (long)(h);
	  f = h - i;
	  if(!(i & 1))
    {
      f = 1 - f; // if i is even
    }
	  m = v * (1 - s);
	  n = v * (1 - s * f);

	  switch(i)
    {
		case 6:
		case 0:
      rgb[0] = v;
      rgb[1] = n;
      rgb[2] = m;
      return;
		case 1:
      rgb[0] = n;
      rgb[1] = v;
      rgb[2] = m;
      return;
		case 2:
      rgb[0] = m;
      rgb[1] = v;
      rgb[2] = n;
      return;
    case 3:
      rgb[0] = m;
      rgb[1] = n;
      rgb[2] = v;
      return;
		case 4:
      rgb[0] = n;
      rgb[1] = m;
      rgb[2] = v;
      return;
		case 5:
      rgb[0] = v;
      rgb[1] = m;
      rgb[2] = n;
      return;
    }

    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // YIQToRGB
  //inline bool YIQToRGB(const Colorant* yiq, Colorant* rgb)
  //{
  //  Colorant y = yiq[0];
  //  Colorant i = yiq[1];
  //  Colorant q = yiq[2];
  //  rgb[0] = y + 0.956f * i + 0.621f * q;
  //  rgb[1] = y - 0.272f * i - 0.647f * q;
  //  rgb[2] = y - 1.105f * i + 1.702f * q;
  //  return clamp(rgb[0], 0, 1) | clamp(rgb[1], 0, 1) | clamp(rgb[2], 0, 1);
  //}

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToYIQ
  //inline bool RGBToYIQ(const Colorant* rgb, Colorant* yiq)
  //{
  //  Colorant r = rgb[0];
  //  Colorant g = rgb[1];
  //  Colorant b = rgb[2];
  //  yiq[0] = 0.299f * r + 0.587f * g + 0.114f * b;
  //  yiq[1] = 0.299f * r + 0.587f * g + 0.114f * b;
  //  yiq[2] = 0.299f * r + 0.587f * g + 0.114f * b;
  //  return clamp(yiq[0], 0, 1) | clamp(yiq[1], 0, 1) | clamp(yiq[2], 0, 1);
  //}

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToGray
  inline void RGBToGray(const Colorant* rgb, Colorant* gray)
  {
    Colorant r = rgb[0];
    Colorant g = rgb[1];
    Colorant b = rgb[2];
    gray[0] = 0.299f * r + 0.587f * g + 0.114f * b;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // GrayToRGB
  inline void GrayToRGB(const Colorant* gray, Colorant* rgb)
  {
    Colorant gr = gray[0];
    rgb[0] = gr;
    rgb[1] = gr;
    rgb[2] = gr;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // YUVToRGB
  //inline bool YUVToRGB(const Colorant* yuv, Colorant* rgb)
  //{
  //  Colorant y = yuv[0];
  //  Colorant u = yuv[1];
  //  Colorant v = yuv[2];
  //  rgb[0] = y             + 1.140f * v;
  //  rgb[1] = y - 0.394f * u - 0.581f * v;
  //  rgb[2] = y + 2.028f * u;
  //  return clamp(rgb[0], 0, 1) | clamp(rgb[1], 0, 1) | clamp(rgb[2], 0, 1);
  //}

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToYUV
  //inline bool RGBToYUV(const Colorant* rgb, Colorant* yuv)
  //{
  //  Colorant r = rgb[0];
  //  Colorant g = rgb[1];
  //  Colorant b = rgb[2];
  //  yuv[0] =  0.299f * r + 0.587f * g + 0.114f * b;
  //  yuv[1] = -0.147f * r - 0.289f * g + 0.437f * b;
  //  yuv[2] =  0.615f * r - 0.515f * g - 0.100f * b;
  //  return clamp(yuv[0], 0, 1) | clamp(yuv[1], 0, 1) | clamp(yuv[2], 0, 1);
  //}

  //////////////////////////////////////////////////////////////////////////////////////////
  // CIE1931ToRGB
  inline bool CIE1931ToRGB(const Colorant* xyz, Colorant* rgb)
  {
    Colorant x = xyz[0];
    Colorant y = xyz[1];
    Colorant z = xyz[2];
    rgb[0] =  3.240479f * x - 1.537150f * y - 0.498535f * z;
    rgb[1] = -0.969256f * x + 1.875992f * y + 0.041556f * z;
    rgb[2] =  0.055648f * x - 0.204043f * y + 1.057311f * z;
    return clamp(rgb[0], 0, 1) | clamp(rgb[1], 0, 1) | clamp(rgb[2], 0, 1);
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // RGBToCIE1931
  inline bool RGBToCIE1931(const Colorant* rgb, Colorant* xyz)
  {
    Colorant r = rgb[0];
    Colorant g = rgb[1];
    Colorant b = rgb[2];
    xyz[0] =  0.412453f * r + 0.357580f * g + 0.180423f * b;
    xyz[1] =  0.212671f * r + 0.715160f * g + 0.072169f * b;
    xyz[2] =  0.019334f * r + 0.119193f * g + 0.950227f * b;
    return clamp(xyz[0], 0, 1) | clamp(xyz[1], 0, 1) | clamp(xyz[2], 0, 1);
  }

}

