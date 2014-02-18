

#include "StdAfx.h"
#include "selectionframe.h"


/*
group
static
frame
frame
static, multiline
*/


SelectionFrame::SelectionFrame()
{
}

SelectionFrame::~SelectionFrame()
{
}

LRESULT SelectionFrame::OnCreate(LPCREATESTRUCT p)
{
  m_group.Create(*this, 0, _T("m_group"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0);
  m_OK.Create(*this, 0, _T("Ok"), WS_CHILD | WS_VISIBLE, 0);
  m_Exit.Create(*this, 0, _T("Exit"), WS_CHILD | WS_VISIBLE, 0);

  m_autoPlacement.RegisterSymbol(_T("m_group"), m_group);
  m_autoPlacement.RegisterSymbol(_T("m_ok"), m_OK);
  m_autoPlacement.RegisterSymbol(_T("m_EXIT"), m_Exit);
  m_autoPlacement.RunFile(_T("selectionframe.placement"));
  m_autoPlacement.DumpOutput();
  return 0;
}

void SelectionFrame::OnSize(UINT wParam, const CSize& s)
{
  m_autoPlacement.OnSize();
}

