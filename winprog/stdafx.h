// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define  _CRT_SECURE_NO_DEPRECATE

#define _WTL_NO_AUTOMATIC_NAMESPACE

#define WINVER		0x0501
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT	0x0500
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0100

#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlwin.h>
#include <atlcrack.h>

#include <atltypes.h>
#include <atlctrls.h>
#include <atlscrl.h>
#include <atlmisc.h>


#define CCSTR_OPTION_AUTOCAST 1
#include "..\libcc\libcc\stringutil.hpp"
#include "..\libcc\libcc\log.hpp"


// a=w/h
void SizeRectMaintainAspect(const CSize& canvas, const CSize& object, CSize& scaled)
{
  // greater aspect ratio means greater width
  float aObject = (float)object.cx / object.cy;
  float aCanvas = (float)canvas.cx / canvas.cy;
  if(aObject > aCanvas)
  {
    // object is "wider" than canvas... use canvas width for scaling
    scaled.cx = canvas.cx;
    scaled.cy = (long)((float)canvas.cx / aObject);
  }
  else if(aObject == aCanvas)
  {
    // same aspect ratio
    scaled.cx = canvas.cx;
    scaled.cy = canvas.cy;
  }
  else
  {
    // object is "taller" than canvas - use canvas height for scaling
    scaled.cy = canvas.cy;
    scaled.cx = (long)(canvas.cy * aObject);
  }
}

//inline bool IsValidHandle(HANDLE h)
//{
//  return (h != 0) && (h != INVALID_HANDLE_VALUE);
//}


template<typename Tto, typename Tfrom>
inline Tto scast(const Tfrom& x)
{
  return static_cast<Tto>(x);
}

#include <string>

typedef std::basic_string<TCHAR> _tstring;
typedef std::basic_stringstream<TCHAR> _tstringstream;
typedef std::basic_istringstream<TCHAR> _tistringstream;
typedef std::basic_ostringstream<TCHAR> _tostringstream;
typedef std::basic_iostream<TCHAR> _tiostream;
typedef std::basic_istream<TCHAR> _tistream;
typedef std::basic_ostream<TCHAR> _tostream;
typedef std::basic_streambuf<TCHAR> _tstreambuf;
typedef std::basic_stringbuf<TCHAR> _tstringbuf;
typedef std::basic_filebuf<TCHAR> _tfilebuf;
typedef std::basic_ifstream<TCHAR> _tifstream;
typedef std::basic_ofstream<TCHAR> _tofstream;
typedef std::basic_fstream<TCHAR> _tfstream;
typedef std::basic_ios<TCHAR> _tios;
#ifdef _UNICODE
#   define _tcin                   std::wcin
#   define _tcout                  std::wcout
#   define _tcerr                  std::wcerr
#   define _tclog                  std::wclog
#else
#   define _tcin                   std::cin
#   define _tcout                  std::cout
#   define _tcerr                  std::cerr
#   define _tclog                  std::clog
#endif
