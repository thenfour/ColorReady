

#pragma once


#include "ColorSpot.h"
#include "uitools.h"


typedef CWinTraits<WS_CHILD, 0> SelectionFrameTraits;

class SelectionFrame : public CWindowImpl<SelectionFrame, CWindow, SelectionFrameTraits>
{
public:
  BEGIN_MSG_MAP_EX(SelectionFrame)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_SIZE(OnSize)
  END_MSG_MAP()

  SelectionFrame();
  ~SelectionFrame();

  LRESULT OnCreate(LPCREATESTRUCT p);
  void OnSize(UINT wParam, const CSize& s);

private:
  WTL::CButton m_group;

  WTL::CStatic m_oldText;
  ColorSpot m_oldSpot;
  WTL::CStatic m_newText;
  ColorSpot m_newSpot;
  WTL::CStatic m_details;
  WTL::CButton m_addToPalette;
  WTL::CButton m_OK;
  WTL::CButton m_Exit;

  AutoPlacement::Manager m_autoPlacement;
};
