/*
  supports
  -) drag and drop
  -) adjustable border
  -) adjustable spacing
  -) adjustable item width
  -) adjustable item height
  -) random access to items
  -) item text supported
  -) scrollable (via scrollbars and mousewheel)
  -) support special colors like industry palette colors with a mark

  this is broken into 3 categories:

  item

  collection

  view

  The main issue is the distinction between collection and view.  Does the palette item collection
  store information about the placement of items?  I dont think it should... should the view just
  dumbly display them always just like characters in an edit box - left to right and down?  There may
  be many times when you really want it to look like a 2-d array - like a spectrum of colors.

  But how will that work in the UI?  Are users really going to mess with that setting?  I guess it
  would just be a check-box to make it fixed-width.

  Ok - there is now only a single setting for it - 

  Terminology:
  The "VIRTUAL" palette is the entire thing.  The actual window only shows a portion of the "virtual palette".
  The virtual palette does NOT include the 'extra' space at the end of the palette that we show where you can
  like scroll past the end of the actual palette.

*/

#pragma once

#include "colorframework.h"
#include "animbitmap.h"
#include <atltypes.h>
#include <vector>
#include "colorcontrols.h"


using namespace Colors;


const UINT PCM_SELCHANGED = 0;
const UINT PCM_MOUSEMOVE = 0;
const UINT PCM_HOVERCHANGED = 0;


// A single item - mostly this just contains a ColorSpec, but also has palette-specific stuff.
struct PaletteItem
{
  PaletteItem();
  PaletteItem(const ColorSpec& r);
  PaletteItem(const ColorSpec& c, const std::string& t);
  PaletteItem(const ColorSpec& c, const std::string& t, bool Flagged);
  PaletteItem(const PaletteItem& r);
  PaletteItem& operator = (const PaletteItem& r);

  enum ItemType
  {
    Blank,
    Color
  };

  ItemType m_Type;
  ColorSpec m_Color;
  std::string m_Text;// what's this text for?  i dunno...
  bool m_Flagged;
};


struct PaletteViewOptions
{
  static const long DefaultItemWidth = 16;
  static const long DefaultItemHeight = 16;
  static const bool DefaultFixedWidth =  true;
  static const long DefaultWidth = 19;
  static const long DefaultMinWidth = 10;// minimum width of an item before it gets wrapped to the next line in non-fixed width mode.
  inline static RgbPixel DefaultItemBorderColor() { return MakeRgbPixel(0,0,0); }
  inline static RgbPixel DefaultBackColor() { return MakeRgbPixel(96,90,130); }
  inline static CRect DefaultSpacing() { return CRect(1,1,1,1); }
  inline static CRect DefaultItemBorderWidth() { return CRect(1,1,1,1); }

  // for specifying the flag
  inline static CRect DefaultFlagBorder() { return CRect(1,1,1,1); };
  inline static CPoint DefaultFlagPos() { return CPoint(3, 3); };
  inline static CSize DefaultFlagSize() { return CSize(4, 4); };
  inline static RgbPixel DefaultFlagBorderColor() { return MakeRgbPixel(0,0,0); }
  inline static RgbPixel DefaultFlagColor() { return MakeRgbPixel(255,255,255); }

  PaletteViewOptions() :
    m_ItemWidth(DefaultItemWidth),
    m_ItemHeight(DefaultItemHeight),
    m_FixedWidth(DefaultFixedWidth),
    m_Width(DefaultWidth),
    m_MinWidth(DefaultMinWidth)
  {
    m_Spacing = DefaultSpacing();
    m_ItemBorderWidth = DefaultItemBorderWidth();
    m_BackColor = DefaultBackColor();
    m_ItemBorderColor = DefaultItemBorderColor();

    m_FlagBorder = DefaultFlagBorder();
    m_FlagPos = DefaultFlagPos();
    m_FlagSize = DefaultFlagSize();
    m_FlagColor = DefaultFlagColor();
    m_FlagBorderColor = DefaultFlagBorderColor();
  }

  long m_ItemWidth;
  long m_ItemHeight;

  bool m_FixedWidth;
  long m_Width;// not used if m_FixedWidth is false.  number of elements on each row for fixed width mode

  RgbPixel m_BackColor;
  CRect m_Spacing;// spacing between items

  long m_MinWidth;

  RgbPixel m_ItemBorderColor;
  CRect m_ItemBorderWidth;// small black border for each item

  CRect m_FlagBorder;
  CPoint m_FlagPos;
  CSize m_FlagSize;
  RgbPixel m_FlagColor;
  RgbPixel m_FlagBorderColor;
};


typedef CWinTraits<WS_CHILD, 0> ColorPaletteTraits;


class PaletteCtl :
  public CWindowImpl<PaletteCtl, CWindow, ColorPaletteTraits>
{
public:
  DECLARE_WND_CLASS("PaletteCtl");

  BEGIN_MSG_MAP_EX(PaletteCtl)
    MSG_WM_MOUSEWHEEL(OnMouseWheel)
    MSG_WM_VSCROLL(OnVScroll)
    MSG_WM_HSCROLL(OnHScroll)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_SIZE(OnSize)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    //CHAIN_MSG_MAP(CScrollWindowImpl<PaletteCtl>)
  END_MSG_MAP()

  // Constructors, operators, etc
  PaletteCtl(ColorManager* pmgr);

  // Message Handlers
  void OnCaptureChanged(HWND h);
  void OnLButtonDown(UINT wParam, WTL::CPoint& p);
  void OnLButtonUp(UINT wParam, WTL::CPoint& p);
  void OnMouseMove(UINT wParam, WTL::CPoint& p);
  LRESULT OnMouseWheel(UINT keys, short distance, const CPoint& p);
  LRESULT OnEraseBkgnd(HDC dc);
  LRESULT OnCreate(LPCREATESTRUCT p);
  void OnPaint(HDC);
  void OnSize(UINT wParam, const CSize& s);
  void OnVScroll(UINT request, UINT pos, HWND h);
  void OnHScroll(UINT request, UINT pos, HWND h);

  // Stuff for external use
  void SetNotify(HWND h);

  // palette management
  struct iterator
  {
    friend class PaletteCtl;
  public:
    iterator();
    iterator(const iterator& r);
    iterator(std::vector<PaletteItem>* pv, std::vector<PaletteItem>::iterator& it);
    iterator& operator = (const iterator& r);
    bool operator == (const iterator& r) const;
    bool operator != (const iterator& r) const;
    iterator& operator ++();

  private:
    std::vector<PaletteItem>* m_plist;
    std::vector<PaletteItem>::iterator m_it;
  };

  bool PushBack(const PaletteItem& v);
  bool PushFront(const PaletteItem& v);
  bool InsertBefore(const iterator& it, const PaletteItem& v);
  bool Erase(const iterator& it);
  long Count() const;
  const PaletteItem& Back() const;
  const PaletteItem& Front() const;

  iterator begin();
  iterator end();
  iterator ItemToIterator(long i);

private:

  void _RenderItem(PaletteItem& i, const CRect& rc);
  void _Draw(HDC dc);

  typedef std::vector<PaletteItem> ListType;

  void UpdateScrollInfo();
  long GetVScrollPageSize() const;
  long GetVScrollLineSize() const;
  long GetVScrollMax() const;
  long GetHScrollPageSize() const;
  long GetHScrollLineSize() const;
  long GetHScrollMax() const;

  // Item, Column, Row, Pixel, ColumnRow
  long VirtualRowColumnToIndex(long x, long y) const;// coordinates of the virtual palette INDICES to palette index
  long VirtualXToVirtualColumn(long x) const;
  long VirtualYToVirtualRow(long y) const;
  long GetVirtualColumnCount() const;
  long GetVirtualRowCount() const;
  long GetVirtualHeight() const;// in pixels
  long GetVirtualWidth() const;// in pixels

  void VirtualRowColumnToRect(long x, long y, CRect& rc) const;

  long GetVisibleColumnCount() const;
  long GetVisibleRowCount() const;

  long GetCompleteItemWidth() const;// if the viewing area is 100 pixels wide and it shows EXACTLY 10 items, this function returns 10.  So it includes everything to do with an item - padding, etc.
  long GetCompleteItemHeight() const;

  struct HitTestResult
  {
    enum WhatEnum
    {
      HT_None,
      HT_Item
    }
    What;
    long iItem;
    long VirtualPosX;
    long VirtualPosY;
    iterator it;
  };

  bool HitTest(const CPoint& p, HitTestResult& r) const;

  void RefreshScroll();

  ListType m_items;

  ColorManager* m_pmgr;

  PaletteViewOptions m_view;

  CSize m_size;
  bool m_bHaveCapture;
  enum CaptureType { CT_None, CT_Drag } m_CaptureType;// we may have capture for a number of reasons...

  long m_ViewX;// the x coord of the actual view of the virtual palette.
  long m_ViewY;
  HWND m_hNotify;
  AnimBitmap m_offscreen;

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
};

