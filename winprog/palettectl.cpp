

#include "stdafx.h"
#include "palettectl.h"
#include <iostream>


// PALETTE ITEM //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PaletteItem::PaletteItem() :
  m_Type(Blank),
  m_Flagged(false),
	m_Selected(false)
{
}

PaletteItem::PaletteItem(const ColorSpec& r) :
  m_Type(Color),
  m_Color(r),
  m_Flagged(false),
	m_Selected(false)
{
}

PaletteItem::PaletteItem(const ColorSpec& c, const std::string& t, bool flagged, bool selected) :
  m_Type(Color),
  m_Text(t),
  m_Color(c),
  m_Flagged(flagged),
	m_Selected(selected)
{
}

// PALETTE CONTROL ITERATOR /////////////////////////////////////////////////////////////////////////////////////////////////////////////
PaletteCtl::iterator::iterator() :
  m_plist(0)
{
}

PaletteCtl::iterator::iterator(const PaletteCtl::iterator& r) :
  m_plist(r.m_plist),
  m_it(r.m_it)
{
}

PaletteCtl::iterator::iterator(std::vector<PaletteItem>* pv, std::vector<PaletteItem>::iterator& it) :
  m_plist(pv),
  m_it(it)
{
}

PaletteCtl::iterator& PaletteCtl::iterator::operator = (const PaletteCtl::iterator& r)
{
  m_plist = r.m_plist;
  m_it = r.m_it;
  return *this;
}

bool PaletteCtl::iterator::operator == (const PaletteCtl::iterator& r) const
{
  return (r.m_plist == m_plist) && (r.m_it == m_it);
}


bool PaletteCtl::iterator::operator != (const PaletteCtl::iterator& r) const
{
  return !((*this) == r);
}

PaletteCtl::iterator& PaletteCtl::iterator::operator ++()
{
  ++ m_it;
  return *this;
}

PaletteCtl::iterator PaletteCtl::iterator::operator +(int n)
{
	PaletteCtl::iterator ret(*this);
	ret += n;
	return ret;
}

PaletteCtl::iterator& PaletteCtl::iterator::operator +=(int n)
{
	m_it += n;
	return *this;
}

// PALETTE CONTROL //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PaletteCtl::PaletteCtl(ColorManager* pmgr, IPaletteViewEvents* pNotify) :
  m_pmgr(pmgr),
	m_currentMouseOver(0),
	m_pNotify(pNotify)
{
	m_viewOrigin.x = 0;
	m_viewOrigin.y = 0;
}


LRESULT PaletteCtl::OnEraseBkgnd(HDC dc)
{
  return 1;
}

void PaletteCtl::OnPaint(HDC)
{
  PAINTSTRUCT ps;
  BeginPaint(&ps);
  HDC h = GetDC();
  _Draw(h);
  ReleaseDC(h);
  EndPaint(&ps);
}

void PaletteCtl::OnVScroll(UINT request, UINT, HWND h)
{
  SCROLLINFO si = {0};
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
  GetScrollInfo(SB_VERT, &si);

  switch(request)
  {
  case SB_PAGEDOWN:
      m_viewOrigin.y += GetVScrollPageSize();
      break;
  case SB_PAGEUP:
      m_viewOrigin.y -= GetVScrollPageSize();
      break;
  case SB_LINEDOWN:
      m_viewOrigin.y += GetVScrollLineSize();
      break;
  case SB_LINEUP:
      m_viewOrigin.y -= GetVScrollLineSize();
      break;
  case SB_TOP:
      m_viewOrigin.y = 0;
      break;
  case SB_BOTTOM:
      m_viewOrigin.y = GetVScrollMax();
      break;
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
      m_viewOrigin.y = si.nTrackPos;
      break;
  }

	int tempy = m_viewOrigin.y;
  clamp(m_viewOrigin.y, 0, GetVirtualMaxTop());
  UpdateScrollInfo();
  Invalidate();
  return;
}


void PaletteCtl::OnHScroll(UINT request, UINT, HWND h)
{
  SCROLLINFO si = {0};
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
  GetScrollInfo(SB_HORZ, &si);

  switch(request)
  {
  case SB_PAGEDOWN:
      m_viewOrigin.x += GetHScrollPageSize();
      break;
  case SB_PAGEUP:
      m_viewOrigin.x -= GetHScrollPageSize();
      break;
  case SB_LINEDOWN:
      m_viewOrigin.x += GetHScrollLineSize();
      break;
  case SB_LINEUP:
      m_viewOrigin.x -= GetHScrollLineSize();
      break;
  case SB_TOP:
      m_viewOrigin.x = 0;
      break;
  case SB_BOTTOM:
      m_viewOrigin.x = GetHScrollMax();
      break;
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
      m_viewOrigin.x = si.nTrackPos;
      break;
  }

  clamp(m_viewOrigin.x, 0, GetHScrollMax());
  UpdateScrollInfo();
  Invalidate();

  return;
}

void PaletteCtl::OnFocus(HWND)
{
	Invalidate();
}

void PaletteCtl::OnKillFocus(HWND)
{
	Invalidate();
}

void PaletteCtl::OnMouseMove(UINT keys, const WTL::CPoint& p)
{
	PaletteItem* pNew = 0;
	HitTestResult r = ClientHitTest(p);
	if(r.hit)
	{
		pNew = &*(r.it);
	}
	if(m_currentMouseOver != pNew)
	{
		m_currentMouseOver = pNew;
		if(m_pNotify)
		{
			m_pNotify->OnPaletteItemMouseOver(pNew);
		}
	}
}

void PaletteCtl::OnLButtonDown(UINT keys, const WTL::CPoint& p)
{
	SetFocus();
}

LRESULT PaletteCtl::OnMouseWheel(UINT keys, short distance, const WTL::CPoint& p)
{
	long Delta = -(distance / WHEEL_DELTA);
	int newVal = 0;

  if(GetPotentiallyVisibleRowCount() < GetRowCount())
	{
		// vertical scrolling
		long OldPos = m_viewOrigin.y;
		m_viewOrigin.y += Delta * GetVScrollLineSize();
		clamp(m_viewOrigin.y, 0, GetVirtualMaxTop());
		newVal = (m_viewOrigin.y - OldPos);
	}
  else if(GetPotentiallyVisibleColumnCount() < GetColumnCount())
	{
		long OldPos = m_viewOrigin.x;
		m_viewOrigin.x += Delta * GetHScrollLineSize();
		clamp(m_viewOrigin.x, 0, GetVirtualMaxLeft());
		newVal = (m_viewOrigin.x - OldPos);
	}
	ScrollWindowEx(0, newVal, SW_INVALIDATE, 0, 0, 0, 0);
	UpdateScrollInfo();
  return 0;
}

void PaletteCtl::OnSize(UINT wParam, const CSize& s)
{
	RECT rc;
  GetClientRect(&rc);
	m_clientSize.cx = rc.right;
	m_clientSize.cy = rc.bottom;
  m_offscreen.SetSize(s.cx, s.cy);
  UpdateScrollInfo();
}

bool PaletteCtl::PushBack(const PaletteItem& v)
{
  m_items.push_back(v);
  return true;
}

bool PaletteCtl::PushFront(const PaletteItem& v)
{
  m_items.insert(m_items.begin(), v);
  return true;
}

bool PaletteCtl::InsertBefore(const iterator& it, const PaletteItem& v)
{
  m_items.insert(it.m_it, v);
  return true;
}

bool PaletteCtl::Erase(const iterator& it)
{
  m_items.erase(it.m_it);
  return true;
}

long PaletteCtl::Count() const
{
  return static_cast<long>(m_items.size());
}

const PaletteItem& PaletteCtl::Back() const
{
  return m_items.back();
}

const PaletteItem& PaletteCtl::Front() const
{
  return m_items.front();
}

PaletteCtl::iterator PaletteCtl::ItemToIterator(long i)
{
  return PaletteCtl::iterator(&m_items, m_items.begin() + i);
}

PaletteCtl::iterator PaletteCtl::begin()
{
  return PaletteCtl::iterator(&m_items, m_items.begin());
}

PaletteCtl::iterator PaletteCtl::end()
{
  return PaletteCtl::iterator(&m_items, m_items.end());
}

int PaletteCtl::GetCompleteItemWidth() const
{
	return (m_options.m_ItemBodySize.cx + m_options.m_ItemSpacing.left + m_options.m_ItemSpacing.right + m_options.m_TotalBorder.left + m_options.m_TotalBorder.right);
}
int PaletteCtl::GetCompleteItemHeight() const
{
	return (m_options.m_ItemBodySize.cy + m_options.m_ItemSpacing.top + m_options.m_ItemSpacing.bottom + m_options.m_TotalBorder.top + m_options.m_TotalBorder.bottom);
}

// an item's dimensions have these segments:
// [spacing] [border extra] [border] [body] [border] [border extra] [spacing]
// border extra is only there if m_SelectedBorderWidth and m_BorderSize are different values.
// if selected border width is 5, and normal border width is 1, then 4 extra pixels are added as spacing for this item.
void PaletteCtl::_Draw(HDC dc)
{
	bool hasFocus = GetFocus() == (HWND)(*this);
	m_offscreen.Fill(hasFocus ? m_options.m_BackColorFocused : m_options.m_BackColorNotFocused);

  int firstColumn = m_viewOrigin.x / GetCompleteItemWidth();
  int y = m_viewOrigin.y / GetCompleteItemHeight();
	int x;
	int xmax = min(GetColumnCount(), 1 + firstColumn + min(GetColumnCount(), GetPotentiallyVisibleColumnCount()));
	int rows = min(GetRowCount(), GetPotentiallyVisibleRowCount());

  bool bBail = false;
  CRect rc;

	RgbPixel itemBorderColor = hasFocus ? m_options.m_ItemBorderColorFocused : m_options.m_ItemBorderColorNotFocused;
	RgbPixel flagBorderColor = hasFocus ? m_options.m_FlagBorderColorFocused : m_options.m_FlagBorderColorNotFocused;
	RgbPixel flagColor = hasFocus ? m_options.m_FlagColorFocused : m_options.m_FlagColorNotFocused;

  while(1)
  {
		// x and y are row/column coords
    for(x = firstColumn; x < xmax; ++x)
    {
      int index = y * GetColumnCount() + x;
      if(index >= Count())
      {
        bBail = true;
        break;
      }
			const PaletteItem& i = m_items[index];

			if(i.m_Type == PaletteItem::Color)
			{
				// this may change based on selected state
				// the actual space given to the border will be the MAX of m_SelectedBorderWidth or m_BorderSize
				RgbPixel thisItemBorderColor;
				CRect borderSize;
				CRect borderExtra;

				if(i.m_Selected)
				{
					thisItemBorderColor = hasFocus ? m_options.m_SelectedItemBorderColorFocused : m_options.m_SelectedItemBorderColorNotFocused;
					borderSize = m_options.m_SelectedItemBorderSize;
				}
				else
				{
					thisItemBorderColor = hasFocus ? m_options.m_ItemBorderColorFocused : m_options.m_ItemBorderColorNotFocused;
					borderSize = m_options.m_ItemBorderSize;
				}
				borderExtra = CRect(
					m_options.m_TotalBorder.left - borderSize.left,
					m_options.m_TotalBorder.top - borderSize.top,
					m_options.m_TotalBorder.right - borderSize.right,
					m_options.m_TotalBorder.bottom - borderSize.bottom
					);

				// calculate where to start drawing. this excludes spacing + border extra.
				CPoint origin(
					x * GetCompleteItemWidth() - m_viewOrigin.x + m_options.m_BorderSize.left + borderExtra.left + m_options.m_ItemSpacing.left,
					y * GetCompleteItemHeight() - m_viewOrigin.y + m_options.m_BorderSize.top + borderExtra.top + m_options.m_ItemSpacing.top);

				// draw the item border.
				m_offscreen.SafeRect2(origin.x, origin.y,
					borderSize.left + borderSize.right + m_options.m_ItemBodySize.cx,
					borderSize.top + borderSize.bottom + m_options.m_ItemBodySize.cy,
					thisItemBorderColor);

				// draw the item body
				origin.x += borderSize.left;
				origin.y += borderSize.right;
				m_offscreen.SafeRect2(origin.x, origin.y,
					m_options.m_ItemBodySize.cx,
					m_options.m_ItemBodySize.cy,
					i.m_Color.GetRGBFast());

				if(i.m_Flagged)
				{
					origin.x += m_options.m_FlagPos.x - m_options.m_FlagBorder.left;
					origin.y += m_options.m_FlagPos.y - m_options.m_FlagBorder.top;
					m_offscreen.SafeRect2(origin.x, origin.y,
						m_options.m_FlagSize.cx + m_options.m_FlagBorder.left + m_options.m_FlagBorder.right,
						m_options.m_FlagSize.cy + m_options.m_FlagBorder.top + m_options.m_FlagBorder.bottom,
						flagBorderColor);

					origin.x += m_options.m_FlagBorder.left;
					origin.y += m_options.m_FlagBorder.top;
					m_offscreen.SafeRect2(origin.x, origin.y,
						m_options.m_FlagSize.cx,
						m_options.m_FlagSize.cy,
						flagColor);
				}
			}
    }

    if(bBail)
			break;

    ++ y;
  }

	// draw border
	RgbPixel borderColor = hasFocus ? m_options.m_BorderColorFocused : m_options.m_BorderColorNotFocused;
	m_offscreen.SafeRect(0, 0, m_options.m_BorderSize.left, m_clientSize.cy, borderColor);// left
	m_offscreen.SafeRect(m_clientSize.cx - m_options.m_BorderSize.right, 0, m_clientSize.cx, m_clientSize.cy, borderColor);// right
	m_offscreen.SafeRect(m_options.m_BorderSize.left, 0, m_clientSize.cx - m_options.m_BorderSize.right, m_options.m_BorderSize.top, borderColor);// top
	m_offscreen.SafeRect(m_options.m_BorderSize.left, m_clientSize.cy - m_options.m_BorderSize.bottom, m_clientSize.cx - m_options.m_BorderSize.right, m_clientSize.cy, borderColor);// bottom

  m_offscreen.Blit(dc, 0, 0);
}

long PaletteCtl::GetVScrollPageSize() const
{
	return GetNonBorderClientHeight();
}

long PaletteCtl::GetVScrollLineSize() const
{
  return GetCompleteItemHeight();
}

long PaletteCtl::GetVScrollMax() const
{
	return GetVirtualSize().cy;
}

long PaletteCtl::GetHScrollPageSize() const
{
	return GetNonBorderClientWidth();
}

long PaletteCtl::GetHScrollLineSize() const
{
  return GetCompleteItemWidth();
}

long PaletteCtl::GetHScrollMax() const
{
	return GetVirtualSize().cx;
}

// sets the scroll boundaries for the window's scrollbars.
void PaletteCtl::UpdateScrollInfo()
{
  SCROLLINFO si = {0};
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
  si.nMin = 0;

  // determine if we need to need to add/remove scroll bars.
  if(GetPotentiallyVisibleColumnCount() < GetColumnCount())
  {
    ShowScrollBar(SB_HORZ);
    EnableScrollBar(SB_HORZ);
    si.nMax = GetHScrollMax();
    si.nPage = GetHScrollPageSize();// for some lame reason, nMax must be a multiple of nPage or the scrollbar looks like shit.
		clamp(m_viewOrigin.x, 0, GetVirtualMaxLeft());
    si.nPos = m_viewOrigin.x;
    SetScrollInfo(SB_HORZ, &si, TRUE);
  }
  else
  {
		m_viewOrigin.x = 0;
    ShowScrollBar(SB_HORZ, FALSE);
  }

  if(GetPotentiallyVisibleRowCount() < GetRowCount())
  {
    ShowScrollBar(SB_VERT);
    EnableScrollBar(SB_VERT);
    si.nMax = GetVScrollMax();
    si.nPage = GetVScrollPageSize();
		clamp(m_viewOrigin.y, 0, GetVirtualMaxTop());
    si.nPos = m_viewOrigin.y;
    SetScrollInfo(SB_VERT, &si, TRUE);
  }
  else
  {
		m_viewOrigin.y = 0;
    ShowScrollBar(SB_VERT, FALSE);
  }
}



