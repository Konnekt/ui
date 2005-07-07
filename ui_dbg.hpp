
#ifdef __DEBUG
HWND hwndDbgActTree = 0;
HWND hwndDbgActBar = 0;
INT_PTR CALLBACK DbgActionsProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);


/* showup 0 - actual. 1 - insert/nothing */
void DbgActionsSet(cUIAction * act , bool showup) {
  if (!act || (!act->dbg && !showup) || (act->dbg && showup)) return;
  TVITEMEX item;
  item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_PARAM;
  item.cChildren = (act->type & ACT_ISGROUP)?1:0;
  item.state = TVIS_BOLD;
  item.stateMask = item.state;
  item.lParam = (long)act;
  cUIGroup * grp = (cUIGroup*) act;
  if (act->type & ACT_ISGROUP)
	  item.pszText = _saprintf("%d [%x] P1=%x , CNT=%x , HDL=%x , GHN=%x , OWN=%x , %s (%s)" , act->index , act->id , act->p1 , act->cnt , act->handle , grp->ghandle , act->owner,  act->txt.c_str(), act->params.c_str());
  else
    item.pszText = _saprintf("%d [%x] P1=%x , CNT=%x , HDL=%x , STATUS=%x , OWN=%x , %s (%s)" , act->index , act->id , act->p1 , act->cnt , act->handle , act->status , act->owner, act->txt.c_str(), act->params.c_str());

  if (!act->dbg) {
    TVINSERTSTRUCT tvs;
    tvs.hParent = (act->pparent)?act->pparent->dbg : 0;
    tvs.hInsertAfter = TVI_LAST;
    tvs.itemex = item;
    act->dbg = TreeView_InsertItem(hwndDbgActTree , &tvs);
  } else { // act->dbg
//    item.mask |= TVIF_HANDLE;
    item.hItem = act->dbg;
    TreeView_SetItem(hwndDbgActTree , &item);
  }
  free(item.pszText);
}

void DbgActionsFill(cUIGroup * start = 0) {
  if (!start) start = Act.getRoot();
  for (cUIGroup::t_Act_it Act_it = start->Act.begin(); Act_it!=start->Act.end(); Act_it++) {
    DbgActionsSet((*Act_it), true);
  }
}

void DbgActionsRefresh(cUIGroup * start) {
  if (!start) return;
  DbgActionsSet(start, false);
  for (cUIGroup::t_Act_it Act_it = start->Act.begin(); Act_it!=start->Act.end(); Act_it++) {
    if ((*Act_it)->type & ACT_ISGROUP) DbgActionsRefresh(&(*Act_it)->getGroup());
    else DbgActionsSet((*Act_it), false);
  }
  
}

void DbgActionsClear() {
  for (t_Grp_it Grp_it = Act.Grp.begin(); Grp_it!=Act.Grp.end(); Grp_it++) {
    for (cUIGroup::t_Act_it Act_it = (Grp_it->second)->Act.begin(); Act_it!=(Grp_it->second)->Act.end(); Act_it++) {
      (*Act_it)->dbg = 0;
    }
    (Grp_it->second)->dbg = 0;
  }
}

void onSizeDbgAct(int w , int h) {
  RECT rc,rc2;
  GetClientRect(hwndDbgAct , &rc);
  GetClientRect(hwndDbgActBar , &rc2);
  int barHeight = rc2.bottom - rc.top;
  rc.top = rc2.bottom + 2;
  w = rc.right - rc.left;
  h = rc.bottom - rc.top;

  HDWP hDwp;
  hDwp=BeginDeferWindowPos(2);
  hDwp=DeferWindowPos(hDwp , hwndDbgActBar ,0,
         2,2, w ,barHeight,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndDbgAct , IDC_TREE) ,0,
         0,rc.top, w ,h,SWP_NOZORDER);
  EndDeferWindowPos(hDwp);
}
void DbgActionsWindows(HWND hwnd) {
   hwndDbgActBar = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL,
        WS_CHILD | WS_CLIPCHILDREN |WS_CLIPSIBLINGS | WS_VISIBLE
        |TBSTYLE_TRANSPARENT
        |CCS_NODIVIDER
        | TBSTYLE_FLAT
        | TBSTYLE_LIST
//        | CCS_NOPARENTALIGN
//        | CCS_NORESIZE
        | TBSTYLE_TOOLTIPS
        //| CCS_NOPARENTALIGN
        | CCS_NORESIZE
        , 0, 0, 200, 30, hwnd,
        (HMENU)IDC_TOOLBAR, hInst, 0);
     // Get the height of the toolbar.
     SendMessage(hwndDbgActBar , TB_SETEXTENDEDSTYLE , 0 ,
                       TBSTYLE_EX_MIXEDBUTTONS
                       | TBSTYLE_EX_HIDECLIPPEDBUTTONS
                       );
     SendMessage(hwndDbgActBar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
     TBBUTTON bb;
     bb.dwData=0;
     #define INSERT(cmd , str , bmp , state , style)\
       bb.iString=(int)(str);\
       bb.idCommand=(cmd);\
       bb.iBitmap=(bmp);\
       bb.fsState=(state)| TBSTATE_ENABLED;\
       bb.fsStyle=(style)| BTNS_AUTOSIZE | (bmp==I_IMAGENONE?BTNS_SHOWTEXT:0);\
       SendMessage(hwndDbgActBar, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb)
     INSERT(IDB_REFRESH , "Odœwie¿" , I_IMAGENONE , 0 , BTNS_BUTTON);
     #undef INSERT


}
INT_PTR CALLBACK DbgActionsProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
   switch (message)
   {
        case WM_INITDIALOG: {
                hwndDbgAct = hwnd;
                hwndDbgActTree = GetDlgItem(hwnd , IDC_TREE);
                DbgActionsWindows(hwnd);
                DbgActionsFill();
                onSizeDbgAct(0 , 0);
                break;
                }
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_NCDESTROY:
                DbgActionsClear();
                hwndDbgAct = 0;
                break;
        case WM_COMMAND:
                if (HIWORD(wParam)==BN_CLICKED)
                {
                    switch (LOWORD(wParam)) {
                        case IDB_REFRESH:
                            DbgActionsRefresh(Act.getRoot());
                            return 1;
                    }
                }
              return 1;

        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             switch (nm->code) {
               case TVN_ITEMEXPANDING: {
                    NMTREEVIEW * nmtv = (NMTREEVIEW*)lParam;
//                    IMLOG("EXPAND! %x" , nmtv->itemNew.lParam);
                    cUIAction * act = (cUIAction*)nmtv->itemNew.lParam;
                    if (act->type & ACT_ISGROUP)
                      DbgActionsFill((cUIGroup*)act);
                    return false;}
             }
             return 0;
        case WM_SIZING:
             clipSize(wParam , (RECT *)lParam , 200 , 100);
             return 1;
        case WM_SIZE:
//             IMessage(IMC_LOG , 0,0,(int)"WM_SIZE" , wParam);
             onSizeDbgAct(LOWORD(lParam) , HIWORD(lParam));
             return 1;
   }
   return 0;
}

#endif __DEBUG