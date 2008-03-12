#include "stdafx.h"
#include <Stamina/Object.h>
#include "Window.h"
#include "Kronos.h"
#include "HistoryWnd.h"

void HistoryWindow::_create(WndMessage& m) {
  SendMessage(getHwnd(), WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(Ctrl->hDll(), MAKEINTRESOURCE(IDI_MAIN)));
  SendMessage(getHwnd(), WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(Ctrl->hDll(), MAKEINTRESOURCE(IDI_MAIN)));

  _hImageList = (HIMAGELIST) ICMessage(IMI_GETICONLIST, (int) IML_16);
  _hImageList32 = (HIMAGELIST) ICMessage(IMI_GETICONLIST, (int) IML_32);

  _hTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, "Tree", 
    WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | 
    TVS_SHOWSELALWAYS | TVS_TRACKSELECT | WS_TABSTOP | 0x800, 
    5, 30, 300, 200, getHwnd(), (HMENU)1001, 0, NULL);

  _hStatus = CreateStatusWindow(SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, "Historia", getHwnd(), 100);

  int sbw [3]={/*200 , 300 ,*/ -1};
  SendMessage(_hStatus, SB_SETPARTS, 1, (LPARAM) sbw);

  _hTool = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
    WS_CHILD | WS_CLIPCHILDREN |WS_CLIPSIBLINGS | WS_VISIBLE | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | 
    TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, 0, 500, 30, getHwnd(), (HMENU)1023, 0, 0);
    // | TBSTYLE_LIST | CCS_NOPARENTALIGN | CCS_NORESIZE

  SendMessage(_hTool, TB_SETEXTENDEDSTYLE, 0 ,TBSTYLE_EX_MIXEDBUTTONS /*| TBSTYLE_EX_HIDECLIPPEDBUTTONS*/);
  SendMessage(_hTool, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
  SendMessage(_hTool, TB_SETIMAGELIST, 0, (LPARAM) _hImageList32);

  // Set values unique to the band with the toolbar.
  TBBUTTON bb;
  bb.fsState = TBSTATE_ENABLED;
  bb.fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
  bb.dwData = 0;
  //TBSTYLE_SEP

  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::refresh, IML_32);
  bb.idCommand = IDI_HISTB_REFRESH;
  bb.iString = (int)"Odœwie¿";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.iBitmap=ICMessage(IMI_GETICONINDEX, Kronos::ico::shred, IML_32);
  bb.idCommand=IDI_HISTB_SHRED;
  bb.iString=(int) "Kasuj wszystko";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.fsStyle = BTNS_AUTOSIZE | BTNS_SEP;
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.fsState = 0;
  bb.fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::delete_, IML_32);
  bb.idCommand = IDI_HISTB_DEL;
  bb.iString = (int) "Usuñ";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  // ukryte
  bb.fsState = 0;

  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::save, IML_32);
  bb.idCommand = IDI_HISTB_SAVE;
  bb.iString=(int) "Zapisz";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::print, IML_32);
  bb.idCommand = IDI_HISTB_PRINT;
  bb.iString=(int) "Drukuj";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.fsStyle = BTNS_AUTOSIZE | BTNS_SEP;
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  // bb.fsState=TBSTATE_ENABLED;
  bb.fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::search, IML_32);
  bb.fsState = TBSTATE_ENABLED;
  bb.idCommand = IDI_HISTB_SEARCH;
  bb.iString = (int) "Szukaj";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  bb.fsState = TBSTATE_ENABLED;
  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::resend, IML_32);
  bb.idCommand = IDI_HISTB_RESEND;
  bb.iString = (int) "Przeœlij";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);

  bb.iBitmap = ICMessage(IMI_GETICONINDEX, Kronos::ico::compact, IML_32);
  bb.idCommand = IDI_HISTB_COMPACT;
  bb.iString = (int) "Kompaktuj";
  SendMessage(_hTool, TB_INSERTBUTTON, 0x3FFF, (LPARAM) &bb);

  SendMessage(_hTool, TB_AUTOSIZE, 0, 0);

  _hList = CreateWindow(WC_LISTVIEW, "List1", LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | 
    LVS_OWNERDATA | WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 88,21,230,67, getHwnd(), (HMENU)0, 0, 0);

   _hProgress = CreateWindowEx(0 , PROGRESS_CLASS , "" , WS_CHILD | WS_VISIBLE|PBS_SMOOTH
                   , 210 , 270 , 80 , 16 ,getHwnd() , (HMENU)0 ,0 , 0);
   SendMessage(_hProgress , PBM_SETSTEP , 1 , 0);

   sUIAction act(Kronos::ui::historyWindow, Konnekt::UI::ACT::msg_ctrlview, 0);
   sUIActionNotify_createWindow cw;
   cw.act = act;
   cw.hwndParent = getHwnd();

   IMessageDirect(IM_UIACTION ,tPluginId::pluginUI, (int)&cw);

   _hView = cw.hwnd;

    //SetWindowLong(this->hwnd, GWL_ID, IDC_MSG);


   SetWindowPos(getHwnd(), 0, 0, 0, 400, 300, SWP_NOMOVE);

   ShowWindow(getHwnd(), SW_SHOW);
}

void HistoryWindow::_destroy(WndMessage& m) {
  DestroyWindow(_hTree);
  DestroyWindow(_hStatus);
  DestroyWindow(_hTool);
  DestroyWindow(_hList);
  DestroyWindow(_hProgress);

  sUIAction act(Kronos::ui::historyWindow, Konnekt::UI::ACT::msg_ctrlview, 0);
  sUIActionNotify_destroyWindow dw;
  dw.act = act;
  dw.hwnd = _hView;

  IMessageDirect(IM_UIACTION ,tPluginId::pluginUI, (int)&dw);
}

void HistoryWindow::_size(WndMessage& m) {
  SendMessage(_hStatus, WM_SIZE, 0, m.getParam());

  int w = LOWORD(m.getParam());
  int h = HIWORD(m.getParam());

  HDWP hDwp;
  RECT rc, rc2;
  GetClientRect(getHwnd(), &rc);
  GetClientRect(_hTool, &rc2);

  int barHeight = rc2.bottom;

  rc.top += rc2.bottom + 2;
  rc.left+=2;
  rc.right-=2;
  GetClientRect(_hStatus, &rc2);
  rc.bottom -= rc2.bottom + 2;

  w = rc.right - rc.left;
  h = rc.bottom - rc.top;

  hDwp = BeginDeferWindowPos(4);

  #define H_SPACE 0
  int W_TREE = (int) min(w / 4, 200);
  int H_LIST = (int) min(h / 2.5, 250);

  hDwp = DeferWindowPos(hDwp, _hTree, 0, rc.left,rc.top, W_TREE, h, SWP_NOZORDER);
  hDwp = DeferWindowPos(hDwp, _hList, 0,rc.left + W_TREE + 2,rc.top, w - W_TREE - 2 , H_LIST, SWP_NOZORDER);

  hDwp=DeferWindowPos(hDwp , _hView ,0,
		    rc.left + W_TREE + 2, rc.top + H_LIST + H_SPACE  , w - W_TREE - 2 , h - H_LIST - H_SPACE  ,SWP_NOZORDER);


  POINT pt;
  GetWindowRect(_hStatus , &rc2);
  pt.x = rc2.left;
  pt.y = rc2.top;

  ScreenToClient(getHwnd(), &pt);
  rc2.left = pt.x;
  rc2.top = pt.y;
  rc2.right = pt.x + rc2.right - rc2.left;
  rc2.bottom = pt.y + rc2.bottom - rc2.top;

  hDwp = DeferWindowPos(hDwp, _hProgress ,HWND_TOP, rc2.left + 205,rc2.top+4, 90 , 16 , SWP_NOZORDER);

  #undef H_SPACE
  EndDeferWindowPos(hDwp);

  RedrawWindow(_hProgress, 0, 0, /*RDW_UPDATENOW | */RDW_VALIDATE | RDW_INVALIDATE);

  m.setResult(1);
}
void HistoryWindow::_maxmininfo(WndMessage& m) {
  MINMAXINFO* mmi = m.getParam<MINMAXINFO*>();

  mmi->ptMinTrackSize.x = max(mmi->ptMinTrackSize.x, 400);
  mmi->ptMinTrackSize.y = max(mmi->ptMinTrackSize.y, 300);

  m.setResult(0);
}

void HistoryWindow::_command(WndMessage& m) {
  if (HIWORD(m.getParam<DWORD>(WndMessage::twParam)) == BN_CLICKED) {
    switch (LOWORD(m.getParam<DWORD>(WndMessage::twParam))) {
      case IDI_HISTB_REFRESH:
        break;
      case IDI_HISTB_SHRED:
        break;
      case IDI_HISTB_DEL:
        break;
      case IDI_HISTB_SAVE:
        break;
      case IDI_HISTB_PRINT:
        break;
      case IDCANCEL:
        //DestroyWindow(hwnd);
        break;
      case IDI_HISTB_RESEND: {
        MessageSelect ms(Net::broadcast,(char*) 0 ,Message::typeAll, Message::flagSend);
        ms.runQueue();
        break; }
      case IDI_HISTB_COMPACT:
        break;
    }
  }

  m.setResult(0);
}

void HistoryWindow::_notify(WndMessage& m) {
}
