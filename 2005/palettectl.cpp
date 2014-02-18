

#include "stdafx.h"
#include "palettectl.h"
#include <iostream>


// PALETTE ITEM //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PaletteItem::PaletteItem() :
  m_Type(Blank),
  m_Flagged(false)
{
}

PaletteItem::PaletteItem(const ColorSpec& r) :
  m_Type(Color),
  m_Color(r),
  m_Flagged(false)
{
}

PaletteItem::PaletteItem(const ColorSpec& c, const std::string& t) :
  m_Type(Color),
  m_Text(t),
  m_Color(c),
  m_Flagged(false)
{
}

PaletteItem::PaletteItem(const ColorSpec& c, const std::string& t, bool Flagged) :
  m_Type(Color),
  m_Text(t),
  m_Color(c),
  m_Flagged(Flagged)
{
}

PaletteItem::PaletteItem(const PaletteItem& r) :
  m_Type(r.m_Type),
  m_Color(r.m_Color),
  m_Text(r.m_Text),
  m_Flagged(r.m_Flagged)
{
}

PaletteItem& PaletteItem::operator = (const PaletteItem& r)
{
  m_Type = r.m_Type;
  m_Color = r.m_Color;
  m_Text = r.m_Text;
  m_Flagged = r.m_Flagged;
  return *this;
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

// PALETTE CONTROL //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PaletteCtl::PaletteCtl(ColorManager* pmgr) :
  m_pmgr(pmgr),
  m_bHaveCapture(false),
  m_hNotify(0),
  m_CaptureType(CT_None),
  m_ViewY(0),
  m_ViewX(0)
{
}


void PaletteCtl::OnCaptureChanged(HWND h)
{
  if(m_bHaveCapture)
  {
    ReleaseCapture();
    m_bHaveCapture = false;
  }
}

void PaletteCtl::OnLButtonDown(UINT wParam, WTL::CPoint& p)
{
  // set capture, and do some hit testing.
  SetFocus();
}

void PaletteCtl::OnLButtonUp(UINT wParam, WTL::CPoint& p)
{
  // select an item?  release capture?
}

void PaletteCtl::OnMouseMove(UINT wParam, WTL::CPoint& p)
{
}

LRESULT PaletteCtl::OnEraseBkgnd(HDC dc)
{
  return 1;
}

LRESULT PaletteCtl::OnCreate(LPCREATESTRUCT p)
{
  m_hNotify = GetParent();
  return 0;
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
  //char s[100];
  //sprintf(s, "GET Range:(%lu,%lu) Page(%lu) Pos(%lu) TPos(%lu)\n", si.nMin, si.nMax, si.nPage, si.nPos, si.nTrackPos);
  //OutputDebugString(s);

  switch(request)
  {
  case SB_PAGEDOWN:
      m_ViewY += GetVScrollPageSize();
      break;
  case SB_PAGEUP:
      m_ViewY -= GetVScrollPageSize();
      break;
  case SB_LINEDOWN:
      m_ViewY += GetVScrollLineSize();
      break;
  case SB_LINEUP:
      m_ViewY -= GetVScrollLineSize();
      break;
  case SB_TOP:
      m_ViewY = 0;
      break;
  case SB_BOTTOM:
      m_ViewY = GetVScrollMax();
      break;
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
      m_ViewY = si.nTrackPos;
      break;
  }

  clamp(m_ViewY, 0, GetVScrollMax());
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
      m_ViewX += GetHScrollPageSize();
      break;
  case SB_PAGEUP:
      m_ViewX -= GetHScrollPageSize();
      break;
  case SB_LINEDOWN:
      m_ViewX += GetHScrollLineSize();
      break;
  case SB_LINEUP:
      m_ViewX -= GetHScrollLineSize();
      break;
  case SB_TOP:
      m_ViewX = 0;
      break;
  case SB_BOTTOM:
      m_ViewX = GetHScrollMax();
      break;
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
      m_ViewX = si.nTrackPos;
      break;
  }

  clamp(m_ViewX, 0, GetHScrollMax());
  UpdateScrollInfo();
  Invalidate();

  return;
}

LRESULT PaletteCtl::OnMouseWheel(UINT keys, short distance, const CPoint& p)
{
  long Delta = -(distance / WHEEL_DELTA);
  long OldPos = m_ViewY;
  m_ViewY += Delta * GetVScrollLineSize();
  clamp(m_ViewY, 0, GetVScrollMax());
  ScrollWindowEx(0, (m_ViewY - OldPos), SW_INVALIDATE, 0, 0, 0, 0);
  UpdateScrollInfo();
  return 0;
}

void PaletteCtl::OnSize(UINT wParam, const CSize& s)
{
  m_size = s;
  m_offscreen.SetSize(s.cx, s.cy);
  UpdateScrollInfo();
}

void PaletteCtl::SetNotify(HWND h)
{
  m_hNotify = h;
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

void PaletteCtl::_RenderItem(PaletteItem& i, const CRect& rc)
{
  if(i.m_Type == PaletteItem::Color)
  {
    m_offscreen.SafeRect(rc.left, rc.top, rc.right, rc.bottom, m_view.m_ItemBorderColor);
    m_offscreen.SafeRect(
      rc.left + m_view.m_ItemBorderWidth.left,
      rc.top + m_view.m_ItemBorderWidth.top,
      rc.right - m_view.m_ItemBorderWidth.right,
      rc.bottom - m_view.m_ItemBorderWidth.bottom,
      i.m_Color.GetRGBFast()
      );

    if(i.m_Flagged)
    {
      m_offscreen.SafeRect(
        rc.left + m_view.m_FlagPos.x,
        rc.top + m_view.m_FlagPos.y,
        rc.left + m_view.m_FlagPos.x + m_view.m_FlagSize.cx,
        rc.top + m_view.m_FlagPos.y + m_view.m_FlagSize.cy,
        m_view.m_FlagBorderColor);

      m_offscreen.SafeRect(
        rc.left + m_view.m_FlagPos.x + m_view.m_FlagBorder.left,
        rc.top + m_view.m_FlagPos.y + m_view.m_FlagBorder.top,
        rc.left + m_view.m_FlagPos.x + m_view.m_FlagSize.cx - m_view.m_FlagBorder.right,
        rc.top + m_view.m_FlagPos.y + m_view.m_FlagSize.cy - m_view.m_FlagBorder.bottom,
        m_view.m_FlagColor);
    }
  }
}

void PaletteCtl::_Draw(HDC dc)
{
  m_offscreen.Fill(m_view.m_BackColor);
  long FirstVisibleColumn = VirtualXToVirtualColumn(m_ViewX);
  long FirstVisibleRow = VirtualYToVirtualRow(m_ViewY);
  long y = FirstVisibleRow;
  long x = FirstVisibleColumn;
  long xmax = x + GetVisibleColumnCount();
  if(xmax > GetVirtualColumnCount())
  {
    xmax = GetVirtualColumnCount();
  }
  bool bBail = false;
  CRect rc;

  while(1)// keep going down rows until bBail is true;
  {
    for(x = FirstVisibleColumn; x < xmax; ++x)
    {
      // have we drawn the last item in the collection?
      long i = VirtualRowColumnToIndex(x, y);
      if(i >= Count())
      {
        bBail = true;
        break;
      }

      // we are good to draw this one.
      VirtualRowColumnToRect(x, y, rc);
      _RenderItem(m_items[i], rc);
    }

    if(bBail) break;

    ++ y;
  }

  m_offscreen.Blit(dc, 0, 0);
}

long PaletteCtl::VirtualXToVirtualColumn(long x) const
{
  return x / GetCompleteItemWidth();
}

long PaletteCtl::VirtualYToVirtualRow(long y) const
{
  return y / GetCompleteItemHeight();
}

long PaletteCtl::VirtualRowColumnToIndex(long x, long y) const
{
  return x + (GetVirtualColumnCount() * y);
}

long PaletteCtl::GetCompleteItemWidth() const
{
  return (m_view.m_ItemWidth + m_view.m_Spacing.left + m_view.m_Spacing.right);
}

long PaletteCtl::GetCompleteItemHeight() const
{
  return (m_view.m_ItemHeight + m_view.m_Spacing.top + m_view.m_Spacing.bottom);
}

long PaletteCtl::GetVirtualColumnCount() const
{
  if(m_view.m_FixedWidth)
  {
    return min(m_view.m_Width, static_cast<long>(m_items.size()));
  }
  return min(static_cast<long>(m_items.size()), GetVisibleColumnCount());
}

long PaletteCtl::GetVirtualRowCount() const
{
  long Items = static_cast<long>(m_items.size());
  if(!Items) return 0;

  long Columns = GetVirtualColumnCount();
  if(!Columns) return 0;

  // always round up
  return Items / Columns + ((Items % Columns) ? 1 : 0);
}

long PaletteCtl::GetVisibleColumnCount() const
{
  long ItemWidth = GetCompleteItemWidth();
  // if we're variable-width and
  // the "remainder" (the item that's getting cut off) is less than minimum width
  if((!m_view.m_FixedWidth) && ((m_size.cx % ItemWidth) < m_view.m_MinWidth))
  {
    return (m_size.cx / ItemWidth);
  }
  // other wise, if we're fixed width,
  // or if we need to show that cutoff one, report the cutoff one.
  return m_size.cx / ItemWidth + 1;
}

long PaletteCtl::GetVisibleRowCount() const
{
  return m_size.cy / GetCompleteItemHeight();
}

long PaletteCtl::GetVirtualHeight() const
{
  return GetVirtualRowCount() * GetCompleteItemHeight();
}

long PaletteCtl::GetVirtualWidth() const
{
  return GetVirtualColumnCount() * GetCompleteItemWidth();
}

void PaletteCtl::VirtualRowColumnToRect(long x, long y, CRect& rc) const
{
  x *= GetCompleteItemWidth();
  y *= GetCompleteItemHeight();
  x -= m_ViewX;
  y -= m_ViewY;
  rc.top = m_view.m_Spacing.top + y;
  rc.left = m_view.m_Spacing.left + x;
  rc.right = rc.left + m_view.m_ItemWidth;
  rc.bottom = rc.top + m_view.m_ItemHeight;
}

bool PaletteCtl::HitTest(const CPoint& p, HitTestResult& result) const
{
  bool r = false;
  return r;
}

long PaletteCtl::GetVScrollPageSize() const
{
  return (GetVisibleRowCount() - 1) * GetCompleteItemHeight();
}

long PaletteCtl::GetVScrollLineSize() const
{
  return GetCompleteItemHeight();
}

long PaletteCtl::GetVScrollMax() const
{
  return GetVirtualHeight() - GetVScrollLineSize();
}

long PaletteCtl::GetHScrollPageSize() const
{
  return (GetVisibleColumnCount() - 1) * GetCompleteItemWidth();
}

long PaletteCtl::GetHScrollLineSize() const
{
  return GetCompleteItemWidth();
}

long PaletteCtl::GetHScrollMax() const
{
  return GetVirtualWidth() - GetHScrollLineSize();
}

void PaletteCtl::UpdateScrollInfo()
{
  SCROLLINFO si = {0};
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
  si.nMin = 0;

  // determine if we need to need to add/remove scroll bars.
  if(GetVisibleColumnCount() < GetVirtualColumnCount())
  {
    ShowScrollBar(SB_HORZ);
    EnableScrollBar(SB_HORZ);
    si.nMax = GetHScrollMax();
    si.nPage = 1;//GetHScrollPageSize(); for some lame reason, nMax must be a multiple of nPage or the scrollbar looks like shit.
    si.nPos = m_ViewX;
    SetScrollInfo(SB_HORZ, &si, TRUE);
  }
  else
  {
    ShowScrollBar(SB_HORZ, FALSE);
  }

  if(GetVisibleRowCount() < GetVirtualRowCount())
  {
    ShowScrollBar(SB_VERT);
    EnableScrollBar(SB_VERT);
    si.nMax = GetVScrollMax();
    si.nPage = 1;//GetVScrollPageSize();
    si.nPos = m_ViewY;
    SetScrollInfo(SB_VERT, &si, TRUE);
  }
  else
  {
    ShowScrollBar(SB_VERT, FALSE);
  }
}



