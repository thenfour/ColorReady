/*
	TODO: support selection - multiple selection optional.
*/

#pragma once

#include "colorframework.h"
#include "animbitmap.h"
#include <atltypes.h>
#include <vector>
#include "colorcontrols.h"


using namespace Colors;


// A single item - mostly this just contains a ColorSpec, but also has palette-specific stuff.
struct PaletteItem
{
  PaletteItem();
  PaletteItem(const ColorSpec& r);
  PaletteItem(const ColorSpec& c, const std::string& t, bool Flagged = false, bool Selected = false);

  enum ItemType
  {
    Blank,
    Color
  };

  ItemType m_Type;
  ColorSpec m_Color;
  std::string m_Text;// what's this text for?  i dunno...
  bool m_Flagged;
	bool m_Selected;
};

struct IPaletteViewEvents
{
	//virtual void OnPaletteSelectionChanged() = 0;
	virtual void OnPaletteItemMouseOver(PaletteItem* p) = 0;// p may be 0
};

struct PaletteViewOptions
{
	// behaviors
  static const bool DefaultFixedWidth = true;
  static const long DefaultWidth = 15;
	static const bool DefaultAllowMultipleSelection = true;

	// border / background
	inline static CRect DefaultBorderSize() { return CRect(2, 2, 2, 2); }
  inline static RgbPixel DefaultBorderColorFocused() { return MakeRgbPixel(255,0,0); }
  inline static RgbPixel DefaultBorderColorNotFocused() { return MakeRgbPixel(0,0,255); }
  inline static RgbPixel DefaultBackColorFocused() { return MakeRgbPixel(96,90,130); }
  inline static RgbPixel DefaultBackColorNotFocused() { return MakeRgbPixel(88,88,88); }

	// basic item rendering
	inline static CSize DefaultItemBodySize() { return CSize(16, 16); }
  inline static RgbPixel DefaultItemBorderColorFocused() { return MakeRgbPixel(0,0,0); }
  inline static RgbPixel DefaultItemBorderColorNotFocused() { return MakeRgbPixel(0,0,0); }
  inline static CRect DefaultItemSpacing() { return CRect(1,1,1,1); }
  inline static CRect DefaultItemBorderSize() { return CRect(1,1,1,1); }

  // rendering the flag
  inline static CRect DefaultFlagBorder() { return CRect(1,1,1,1); };
  inline static CPoint DefaultFlagPos() { return CPoint(4, 4); };
  inline static CSize DefaultFlagSize() { return CSize(2, 2); };
  inline static RgbPixel DefaultFlagBorderColorFocused() { return MakeRgbPixel(80,80,80); }
  inline static RgbPixel DefaultFlagBorderColorNotFocused() { return MakeRgbPixel(80,80,80); }
  inline static RgbPixel DefaultFlagColorFocused() { return MakeRgbPixel(240,240,240); }
  inline static RgbPixel DefaultFlagColorNotFocused() { return MakeRgbPixel(240,240,240); }

	// selected item
	inline static CRect DefaultSelectedBorderSize() { return CRect(2,2,2,2); }
	inline static RgbPixel DefaultSelectedBorderColorFocused() { return MakeRgbPixel(225,160,35); }
	inline static RgbPixel DefaultSelectedBorderColorNotFocused() { return MakeRgbPixel(35,160,0225); }

  PaletteViewOptions() :
		m_AllowMultipleSelection(DefaultAllowMultipleSelection),
    m_FixedWidth(DefaultFixedWidth),
    m_Width(DefaultWidth)
  {
		m_ItemBodySize = DefaultItemBodySize();
		m_ItemSpacing = DefaultItemSpacing();
    m_ItemBorderSize = DefaultItemBorderSize();
    m_BackColorNotFocused = DefaultBackColorNotFocused();
    m_BackColorFocused = DefaultBackColorFocused();
    m_ItemBorderColorNotFocused = DefaultItemBorderColorNotFocused();
    m_ItemBorderColorFocused = DefaultItemBorderColorFocused();

    m_FlagBorder = DefaultFlagBorder();
    m_FlagPos = DefaultFlagPos();
    m_FlagSize = DefaultFlagSize();
    m_FlagColorNotFocused = DefaultFlagColorNotFocused();
    m_FlagColorFocused = DefaultFlagColorFocused();
    m_FlagBorderColorNotFocused = DefaultFlagBorderColorNotFocused();
    m_FlagBorderColorFocused = DefaultFlagBorderColorFocused();

		m_BorderColorFocused = DefaultBorderColorFocused();
		m_BorderColorNotFocused = DefaultBorderColorNotFocused();
		m_BorderSize = DefaultBorderSize();

		m_SelectedItemBorderSize = DefaultSelectedBorderSize();
		m_SelectedItemBorderColorFocused = DefaultSelectedBorderColorFocused();
		m_SelectedItemBorderColorNotFocused = DefaultSelectedBorderColorNotFocused();

		// calculated...
		m_TotalBorder = CRect(
			max(m_SelectedItemBorderSize.left, m_ItemBorderSize.left),
			max(m_SelectedItemBorderSize.top, m_ItemBorderSize.top),
			max(m_SelectedItemBorderSize.right, m_ItemBorderSize.right),
			max(m_SelectedItemBorderSize.bottom, m_ItemBorderSize.bottom)
			);
  }

	CSize m_ItemBodySize;

	CRect m_BorderSize;
  RgbPixel m_BorderColorFocused;
  RgbPixel m_BorderColorNotFocused;

  bool m_FixedWidth;
  long m_Width;// not used if m_FixedWidth is false.  number of elements on each row for fixed width mode

	bool m_AllowMultipleSelection;

  RgbPixel m_BackColorNotFocused;
  RgbPixel m_BackColorFocused;
  CRect m_ItemSpacing;

  RgbPixel m_ItemBorderColorFocused;
  RgbPixel m_ItemBorderColorNotFocused;
  CRect m_ItemBorderSize;// small black border for each item

  CRect m_FlagBorder;
  CPoint m_FlagPos;
  CSize m_FlagSize;

  RgbPixel m_FlagColorFocused;
  RgbPixel m_FlagColorNotFocused;

	RgbPixel m_FlagBorderColorFocused;
  RgbPixel m_FlagBorderColorNotFocused;

	RgbPixel m_SelectedItemBorderColorFocused;
	RgbPixel m_SelectedItemBorderColorNotFocused;
	CRect m_SelectedItemBorderSize;

	CRect m_TotalBorder;// calculated as the max of selectedbordersize & bordersize
};


typedef CWinTraits<WS_CHILD, 0> ColorPaletteTraits;


class PaletteCtl :
  public CWindowImpl<PaletteCtl, CWindow, ColorPaletteTraits>
{
	IPaletteViewEvents* m_pNotify;
public:
  DECLARE_WND_CLASS("PaletteCtl");

  BEGIN_MSG_MAP_EX(PaletteCtl)
    MSG_WM_SETFOCUS(OnFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_RBUTTONDOWN(OnLButtonDown)// for focus
    MSG_WM_MOUSEWHEEL(OnMouseWheel)
    MSG_WM_VSCROLL(OnVScroll)
    MSG_WM_HSCROLL(OnHScroll)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_SIZE(OnSize)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
  END_MSG_MAP()

  // Constructors, operators, etc
  PaletteCtl(ColorManager* pmgr, IPaletteViewEvents* pNotify = 0);

  // Message Handlers
	void OnMouseMove(UINT keys, const WTL::CPoint& p);
	void OnLButtonDown(UINT keys, const WTL::CPoint& p);
  LRESULT OnMouseWheel(UINT keys, short distance, const WTL::CPoint& p);
  LRESULT OnEraseBkgnd(HDC dc);
  void OnPaint(HDC);
  void OnSize(UINT wParam, const CSize& s);
  void OnVScroll(UINT request, UINT pos, HWND h);
  void OnHScroll(UINT request, UINT pos, HWND h);
  void OnFocus(HWND h);
  void OnKillFocus(HWND h);

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
		iterator operator +(int n);
		iterator& operator +=(int n);

		PaletteItem& operator *()
		{
			return *m_it;
		}

		PaletteItem* operator ->()
		{
			return &*m_it;
		}

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

  //void _RenderItem(PaletteItem& i, const CRect& rc, const CRect& borderSize, RgbPixel itemBorderColor, RgbPixel flagBorderColor, RgbPixel flagColor);
  void _Draw(HDC dc);

  typedef std::vector<PaletteItem> ListType;

  void UpdateScrollInfo();
  long GetVScrollPageSize() const;
  long GetVScrollLineSize() const;
  long GetVScrollMax() const;
  long GetHScrollPageSize() const;
  long GetHScrollLineSize() const;
  long GetHScrollMax() const;

  void RefreshScroll();

  ListType m_items;

  ColorManager* m_pmgr;

  PaletteViewOptions m_options;


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

	// view operations - converting the visible pixel-based view to palette entries
	CPoint m_viewOrigin;// where on the virtual space is the top-left of the NONBORDER client window area
  CSize m_clientSize;

	int GetNonBorderClientWidth() const
	{
		return (m_clientSize.cx - m_options.m_BorderSize.Width());
	}
	int GetNonBorderClientHeight() const
	{
		return (m_clientSize.cy - m_options.m_BorderSize.Height());
	}

	int GetCompleteItemWidth() const;
	int GetCompleteItemHeight() const;
	int GetPotentiallyVisibleColumnCount() const// only FULL columns.
	{
		return GetNonBorderClientWidth() / GetCompleteItemWidth();
	}

	int GetPotentiallyVisibleRowCount() const// only FULL rows.
	{
		return GetNonBorderClientHeight() / GetCompleteItemHeight();
	}

	int GetColumnCount() const// not visible. this is TOTAL in the array.
	{
		if(m_options.m_FixedWidth)
			return min(m_options.m_Width, (int)m_items.size());

		return min((int)m_items.size(), GetPotentiallyVisibleColumnCount());
	}
	int GetRowCount() const
	{
		int columns = GetColumnCount();
		if(columns == 0)
			return 0;
		return ((int)m_items.size() + columns - 1) / columns;// same as clamp((float)m_items.size() / columns);
	}

	// gets the height in pixels where the top of the lowest vscrolled area is (if you have paged down to the bottom of the palette, this is the position of the TOP of the window)
	int GetVirtualMaxTop() const
	{
		return (GetRowCount() - 1) * GetCompleteItemHeight();
	}
	int GetVirtualMaxLeft() const
	{
		return (GetColumnCount() - 1) * GetCompleteItemWidth();
	}

	// gets the height in pixels where the BOTTOM of the scrolled area is.
	// and the width in pixels
	WTL::CSize GetVirtualSize() const
	{
		WTL::CSize ret;
		// takes the typical dimensions (count*size) and adds the client size - item size, so you can scroll into blank space but 1 row/column is always visible.
		ret.cx = GetVirtualMaxLeft() + GetNonBorderClientWidth();
		ret.cy = GetVirtualMaxTop() + GetNonBorderClientHeight();
		return ret;
	}

	WTL::CPoint ClientToVirtual(const WTL::CPoint& p) const
	{
		return WTL::CPoint(
			p.x + m_viewOrigin.x - m_options.m_BorderSize.left,
			p.y + m_viewOrigin.y - m_options.m_BorderSize.top);
	}

	struct HitTestResult
	{
		iterator it;
		bool hit;
	};
	HitTestResult VirtualHitTest(const WTL::CPoint& /* virtual coords */ p)
	{
		HitTestResult ret;
		ret.hit = false;
		ret.it = end();

		int columns = GetColumnCount();
		int rows = GetRowCount();

		int col = p.x / GetCompleteItemWidth();
		if(col < 0 || col >= columns)
			return ret;

		int row = p.y / GetCompleteItemHeight();
		if(row < 0 || row >= rows)
			return ret;

		int index = (row * columns) + col;
		if(index < 0 || index >= (int)m_items.size())
			return ret;

		ret.hit = true;
		ret.it = begin() + index;

		return ret;
	}
	HitTestResult ClientHitTest(const WTL::CPoint& p)
	{
		return VirtualHitTest(ClientToVirtual(p));
	}

	PaletteItem* m_currentMouseOver;
};

