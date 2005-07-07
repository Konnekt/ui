
cUIAction * UIPopupMenu(cUIGroup & act , UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT *prcRect , int start) {
	unsigned int cntId = act.cnt;
	act.setCnt(cntId, true);
    HMENU hmenu = makeActionsMenu(act , start);
//     hActiveMenu = hmenu;
//                  MenuAction.parent = IMIG_CNT;
    SetForegroundWindow(hWnd);
	RECT rc;
	GetWindowRect(GetDesktopWindow() , &rc);
    cUIAction * pAct = Act.getIndex(TrackPopupMenu(hmenu,uFlags | TPM_RETURNCMD,
                       x,y,nReserved,hWnd , prcRect));
//                 MenuAction = pAct?pAct->act() : NOACTION;
    PostMessage(hWnd, WM_NULL, 0, 0); 
	act.setCnt(cntId, true);
    destroyActionsMenu(act);
    DestroyMenu(hmenu);
	if (pAct) {
		pAct->call(ACTN_ACTION,0,0, cntId);
	}
	
    return pAct;
}


int UIMessageProc(int curCnt , HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam) {
    cUIAction * pAct;
    int i , a;
    HWND hwnd;
    RECT rc;
    switch (message) {
        case WM_INITMENUPOPUP:
             /* TODO : Rozsy³anie akcji przy otwieraniu MENU */
             return 0;
/*             if (hActiveMenu != (HMENU)wParam) {
               ZeroMemory(&mii, sizeof(MENUITEMINFO));
               mii.cbSize=sizeof(MENUITEMINFO);
               mii.fMask=MIIM_ID;
               mii.wID=0;
               GetMenuItemInfo(hActiveMenu , LOWORD(lParam) , 1 , &mii);
//               if (!mii.dwItemData) {return 0;}
               a = mii.wID;
             } else {
               a = MenuAction.parent;
             }

             hActiveMenu=(HMENU)wParam;
             MenuAction.parent = 0;
             if (!Act.exists(a)) return 0;
             MenuAction.parent = a;
//             IMLOG("INITMENUPOPUP [%x] %x %x" , a , wParam , lParam);
             Act[a].call(ACTN_GROUP , curCnt);
             for (i=0;i<Act[a].size();i++) {
               if (Act[a][i].status & (ACTR_SHOW))
                 Act[a][i].call(ACTN_SHOW , curCnt);
             }
             break;
*/
        case WM_MEASUREITEM:
             MEASUREITEMSTRUCT *mis ; mis=(MEASUREITEMSTRUCT *)lParam;
             if (!wParam) { // MENU
                if (!mis->itemData) return 0;
                pAct=(cUIAction*)mis->itemData;
				//SIZE sz = GetTextSize(hwndMain, 0, pAct->txt.c_str());
				mis->itemWidth = 0;
				mis->itemHeight = 0;
                if (!Ico.find(pAct->p1)) return 1;
				int icoWidth = 0;
				int icoHeight = 0;
                Ico[pAct->p1].measure((int*)&icoWidth , (int*)&icoHeight);
				mis->itemWidth += icoWidth;
				mis->itemHeight = max(icoHeight, mis->itemHeight);
				//mis->itemHeight=16;
                mis->itemWidth+=2;
                if ((pAct->status & ACTM_TYPE) == ACTT_BAND) mis->itemWidth-=100;
				else { 
				//	mis->itemHeight=0;
				}
                return 1;
             }
             return 0;
        case WM_DRAWITEM:
             DRAWITEMSTRUCT * dis;
             dis=(DRAWITEMSTRUCT *)lParam;
             if (!wParam) { // MENU
                if (!dis->itemData) return 0;
                pAct=(cUIAction*)dis->itemData;
                if (pAct->status & ACTS_HIDDEN) {return 0;}
                if (!Ico.find(pAct->p1)) return 0;
				Ico[pAct->p1].draw(dis->hDC , (pAct->pparent->status & ACTSMENU_CHECKSPACE) ? 16 : 1 , 1 , 0, ILS_SHADOW);
                return 1;
             }
             return 0;
		case WM_ENTERIDLE:
//			IMLOG("IDLE");
			while (MsgWaitForMultipleObjectsEx(0 , 0 , INFINITE , QS_ALLINPUT | QS_ALLPOSTMESSAGE , MWMO_ALERTABLE | MWMO_INPUTAVAILABLE) == WAIT_IO_COMPLETION) {
				/* lata tu dopóki nie dojdzie jakaœ wiadomoœæ jednoczeœnie wywo³uj¹c wszystkie APC */
			}
			return 0;

        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             switch (nm->code) {
               case TBN_DROPDOWN: {
				   tooltip.reset();
                   pAct=(cUIAction*)GetProp(nm->hwndFrom , "ACTION");
                   if (!pAct) return TBDDRET_NODEFAULT;
                   cUIGroup & grp=pAct->getGroup();
                   grp.handle = nm->hwndFrom;
                   grp.cnt=(unsigned int)GetProp(nm->hwndFrom , "CNTID");
                   NMTOOLBAR * nmtb;
                   nmtb = (NMTOOLBAR *)lParam;
                   i = grp.findIndex(nmtb->iItem);
                   if (i<0 && !grp[i].handle) return TBDDRET_NODEFAULT;
                   RECT rc2;
//                   IMLOG("DROPDOWN: act %d , han %x , %d type" , nmtb->hdr.idFrom , Act[nmtb->hdr.idFrom].handle , Act[nmtb->hdr.idFrom].type);
                   GetWindowRect((HWND)grp.handle , &rc2);
                   SendMessage((HWND)grp.handle , TB_GETITEMRECT , i , (LPARAM)&rc);
				   grp[i].setCnt(grp.cnt);
                   UIPopupMenu(grp[i].getGroup() , TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD,
                                  rc2.left+rc.right,rc2.top+rc.top,0,hWnd , 0);
                   return TBDDRET_DEFAULT;}
               case RBN_CHEVRONPUSHED:
                   pAct=(cUIAction*)GetProp(nm->hwndFrom , "ACTION");
                   if (!pAct) return 0;
                   pAct->handle = nm->hwndFrom;
                   pAct->cnt=(unsigned int)GetProp(nm->hwndFrom , "CNTID");

                   curCnt = pAct->cnt;
                   NMREBARCHEVRON * nmrc;
                   nmrc = (NMREBARCHEVRON*)lParam;
                   pAct = (cUIAction *)nmrc->lParam;
				   pAct->setCnt(curCnt);
                   hwnd = (HWND)pAct->handle;
                   GetClientRect(hwnd , &rc);
                   a = SendMessage(hwnd , TB_BUTTONCOUNT , 0 , 0);
                   RECT rcItem;
                   for (i=0; i < a; i++) {
                      SendMessage(hwnd , TB_GETITEMRECT , i , (LPARAM)&rcItem);
                      if (!InsideRect(rc , rcItem)) break;
                   }
                   GetWindowRect(nmrc->hdr.hwndFrom , &rc);
                   UIPopupMenu(pAct->getGroup(),TPM_RIGHTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON
                                  ,rc.left + nmrc->rc.right
                                  ,rc.top + nmrc->rc.bottom-4
                                  ,0,hWnd , 0 , i);
                   return 0;
			   case EN_LINK: case EN_SELCHANGE:
				   return cRichEditHtml::ParentWindowProc(hWnd , message , wParam , lParam);

             }
             return 0;      
    }
    return 0;
}






// -----------------------------------
void clipPropSize(HWND hwndParent, int edge, RECT & rc, int minW, int minH, int maxW, int maxH) {
	int configProp = (int)GetProp(hwndParent , "configProp");
	if (!configProp) return;
	clipPropSize(Act[configProp], edge, rc, minW, minH, maxW, maxH);
}
void clipPropSize(cUIGroup & g, int edge, RECT & rc, int minW, int minH, int maxW, int maxH) {
	HWND hwnd = (HWND)g.ghandle;
	HWND hwndParent = (HWND)getCfgHandle(g);
	RECT parentRc;
	GetClientRect(hwndParent, &parentRc);
	RECT propRc;
	GetClientRect(hwnd, &propRc);
	// szerokoœæ propa to nowa szerokosc minus szerokosc nie-propa w zwyklym oknie...
	int npW = (parentRc.right - propRc.right);
	int npH = (parentRc.bottom - propRc.bottom);
	int w = (rc.right-rc.left) - npW;
	int h = (rc.bottom-rc.top) - npH;

	w = std::max<int>(w , atoi(g.getParam(AP_MINWIDTH).c_str()));
	if (g.paramExists(AP_MAXWIDTH))
		w = std::min<int>(w , atoi(g.getParam(AP_MAXWIDTH).c_str()));
	h = std::max<int>(h , atoi(g.getParam(AP_MINHEIGHT).c_str()));
	if (g.paramExists(AP_MAXHEIGHT))
		h = std::min<int>(h , atoi(g.getParam(AP_MAXHEIGHT).c_str()));
	setRectSizeByEdge(edge, &rc, w + npW, h + npH);
	clipSize(edge , &rc , minW , minH, maxW, maxH);
}

void resizeProp(HWND hwndParent) {
 int configProp = (int)GetProp(hwndParent , "configProp");
 if (!configProp) return;
 RECT rc;
 cUIGroup & g = Act[configProp];
 HWND hwnd = (HWND)g.ghandle;
 int ymin = 0x7FFFFFFF , ymax = 0;
 GetChildRect(GetDlgItem(hwndParent , IDG_GROUP),&rc);
 int w = rc.right - rc.left - 10;
 int h = rc.bottom - rc.top;
 SetWindowPos((HWND)g.ghandle ,0
             ,rc.left+5,rc.top+15, w ,rc.bottom-rc.top- 20
             ,SWP_NOZORDER|SWP_NOACTIVATE);
 // Sprawdzam czy scrollbar jest widoczny
 SCROLLINFO si;
 si.cbSize = sizeof(si);
 si.fMask = SIF_RANGE;
 GetScrollInfo((HWND)g.ghandle , SB_VERT , &si);

 w-=(si.nMax?GetSystemMetrics(SM_CXVSCROLL):0)+2;
 HDWP hDwp;
 hDwp=BeginDeferWindowPos(0);
 int indent=0; /* TODO : Jakieœ sensowniejsze rozwi¹zanie grupowania */
 int rw = w;
 for (int i = 0 ; i < g.size() ; i++) {
   int t = (g[i].status & ACTM_TYPE);
   if (g[i].status & (ACTS_GROUP) || (!g[i].handle && t!=ACTT_GROUPEND)) continue;
   if (g[i].status & ACTR_RESIZE) {
         sUIActionNotify an = g[i].notify(ACTN_RESIZE , MAKELONG(rw , h) , (int)g.ghandle);
         IMessageDirect(IM_UIACTION , g[i].owner , (int)&an);
   }
   if (t==ACTT_GROUPEND) indent-=INDENT_GROUP;
   if (t != ACTT_HOLDER && g[i].handle && GetWindowRect((HWND)g[i].handle , &rc)) {
      if (rc.top < ymin) ymin = rc.top;
      if (rc.bottom > ymax) ymax = rc.bottom;
   } else continue;
   if (t!=ACTT_HOLDER)   {
		HWND set = (HWND)g[i].handle;
		if (GetProp(set , "AlignRight")) {
			GetChildRect(set,&rc);
			hDwp=DeferWindowPos(hDwp , set ,0,rw - (rc.right - rc.left)-indent,rc.top, 0 ,0,SWP_NOZORDER|SWP_NOSIZE);
			rw -= 5 + (rc.right - rc.left);
		}
		while (set) {
			if (GetProp(set , "FullWidth")) {
				GetChildRect(set,&rc);
				hDwp=DeferWindowPos(hDwp , set ,0,0,0, rw-rc.left-indent ,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
			}
			set = (HWND)GetProp(set, "connectedWnd");
			if (set == (HWND)g[i].handle) break; // pêtla?
 		}
   }
   if (!(g[i].status & ACTSC_INLINE))
	   rw = w;
   if (t==ACTT_GROUP) indent+=INDENT_GROUP;
 }
 EndDeferWindowPos(hDwp);
 // ScrollBar
 h -= 20;
// IMLOG("VERT h %d min %d max %d  -  %d" , h , ymin , ymax , ymax - ymin);
 si.cbSize = sizeof(si);
 si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
 GetScrollInfo(hwnd , SB_VERT , &si);
 GetWindowRect(hwnd , &rc);
 int dif = (rc.top>ymin?rc.top-ymin : 0);
// if (si.nPos + si.nPage >= si.nMax)
//   SendMessage(hwnd , WM_VSCROLL , SB_BOTTOM , 0);
// IMLOG("SRX %d %d" , si.nPos , si.nMax);

// EnableScrollBar(hwnd , SB_VERT , (ymax-ymin > h || rc.top>ymin)?ESB_ENABLE_BOTH:ESB_DISABLE_BOTH);
 ShowScrollBar(hwnd , SB_VERT , (ymax-ymin > h || rc.top>ymin));

 bool recall =  (ymax-ymin > h || rc.top>ymin)!=(si.nMax!=0); // scrollbar sie zmienil, trzeba zrobic resize...

 si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
 si.nMin = 0;
 if ((ymax-ymin > h || rc.top>ymin)) {
   si.nMax = ymax - ymin;
   if (rc.top>ymin) {si.nPos = rc.top-ymin;
   } else    si.nPage = h;

 } else {
     si.nMax = 0;
 }
 SetScrollInfo(hwnd , SB_VERT , &si , 1);

 if (recall) resizeProp(hwndParent);

}

void PropClassCanApply(HWND hwnd) {
    SendMessage(GetParent(hwnd),MYWM_CANAPPLY,0,0);
}

// --------------------------------------------------
int CALLBACK PropClassProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
   int a;
   cUIGroup * grp;
   cUIAction * act;
   char * str;
   RECT rc;
   switch (message)
   {
		case WM_SETCURSOR: {
             //SetCfgTip(GetDlgItem(hwndConfig , IDC_TIP) , (const char*)GetProp((HWND)wParam , "TIP") , (HWND)wParam);
/*			 CStdString className;
			 GetClassName((HWND)wParam, className.GetBuffer(20), 20);
			 className.ReleaseBuffer();
			 if (className == "PROP") {
			 */
			 if (wParam == (WPARAM)hWnd) {
				SetCursor(LoadCursor(0,IDC_ARROW));
			 }	  

			 tooltip.mouseMovement(hWnd);
			 return 0;}
		case WM_MOUSEMOVE:{
			return 0;}
		case WM_ACTIVATE: case WM_LBUTTONUP: {
			if ((HWND)lParam != tooltip.getTipWindow())
				tooltip.reset();
			}
            break;
        case WM_CREATE:
                SetProp(hWnd , "GROUP" , ((CREATESTRUCT*)lParam)->lpCreateParams);
//                grp=(cUIGroup*)lParam;
                break;
		case WM_DESTROY:
			tooltip.clearTargets(hWnd);
			break;
        case WM_MOUSEWHEEL:{
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
//            for (int i=0; i<3; i++)
                SendMessage(hWnd , WM_VSCROLL , (delta>0?SB_PAGEUP:SB_PAGEDOWN) , 0);
            break;}

        case WM_HSCROLL:
			if (GetProp((HWND)lParam, "ACTION"))
				PropClassCanApply(hWnd);
			break;
        case WM_VSCROLL:

                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
                GetScrollInfo(hWnd , SB_VERT , &si);
                a = si.nPos;
                if (!si.nMax) return 0;
//                a = b = GetScrollPos(hWnd , SB_VERT);
//                GetScrollRange(hWnd , SB_VERT , &c , &d);
                switch (LOWORD(wParam)) {
                   case SB_THUMBTRACK:
                     si.nPos = HIWORD(wParam);
                     break;
                   case SB_LINEDOWN:
                     si.nPos += 10;
                     break;
                   case SB_LINEUP:
                     si.nPos -= 10;
                     break;
                   case SB_PAGEDOWN:
                     si.nPos += 40;
                     break;
                   case SB_PAGEUP:
                     si.nPos -= 40;
                     break;
                   case SB_TOP:
                     si.nPos = si.nMin;
                     break;
                   case SB_BOTTOM:
                     si.nPos = si.nMax - si.nPage;
                     break;
                }
//                if (b < si.nMin) b = si.nMin;
//                if (b + si.nPage > si.nMax) b = si.nMax - si.nPage;
                si.fMask = SIF_POS;
                SetScrollInfo(hWnd , SB_VERT , &si , 1);
                GetScrollInfo(hWnd , SB_VERT , &si);
//                SetScrollPos(hWnd , SB_VERT , b , 1);
/* TODO : Problem w przewijaniu! */
                GetClientRect(hWnd , &rc);
                if (si.nPos != a) {
                  ScrollWindow(hWnd , 0 , a - si.nPos , 0 , 0);
                  InvalidateRect(hWnd , &rc , 0);
                  UpdateWindow(hWnd);
                }
//                IMLOG("CL - %d %d" , rc.top , rc.bottom);
                return 0;
        case WM_COMMAND: {
                grp=(cUIGroup*)GetProp(hWnd , "GROUP");
                act=(cUIAction*)GetProp((HWND)lParam , "ACTION");
                if (!grp || !act) return 0;
                int type = act->status & ACTM_TYPE;
                grp->ghandle = hWnd;
                //act->handle = (HANDLE)lParam;
                act->setCnt((unsigned int)GetProp(hWnd , "CNTID"));
                grp->cnt=act->cnt;//=(unsigned int)GetProp(hWnd , "CNTID");
                switch (HIWORD(wParam)) {

                 case EN_CHANGE:
 		 			 tooltip.hideAuto();

                     if (GetProp((HWND)lParam , "PART")) return 0;
                     PropClassCanApply(hWnd);
                     if (!(act->status & ACTR_CHECK)) return 0;
                     SendMessage((HWND)lParam , WM_GETTEXT , MAX_STRING , (LPARAM)TLS().buff);
                     str=(char *)checkCfgValue(*act , TLS().buff);
                     /*TODO: Z³e porównanie! */
                     if (strcmp(TLS().buff , str))
                       SendMessage((HWND)lParam , WM_SETTEXT , 0 , (LPARAM)str);
                     break;
                 case CBN_SELCHANGE:
 		 			 tooltip.hideAuto();
                     PropClassCanApply(hWnd);
                     act->call(ACTN_ACTION , 0 , 0);
                     return 1;
                 case CBN_DROPDOWN:
//                     if (!(act->status & ACTR_DROP)) return 0;
					 tooltip.hideAuto();
                     act->call(ACTN_DROP,0,0);
                     return 1;
                 case BN_CLICKED:
 		 			 tooltip.hideAuto();
                     switch (type) {
					   case ACTT_RADIO:
						   ((cUIAction_cfg*)act) ->setRadioValue(0);
                       case ACTT_LINK: 
                       case ACTT_CHECK:
                       case ACTT_BUTTON: 
                           PropClassCanApply(hWnd);
                           return act->call(ACTN_ACTION , 0 , 0);
					   case ACTT_TIPBUTTON: {
						   if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							   tooltip.reset();
							   return 0;
						   }
						   act->call(ACTN_ACTION , 0 , 0);
						   ToolTipX::iTip * tip = act->getTipObject();
						   if (tip) {
							   RECT rc;
							   GetWindowRect((HWND)lParam, &rc);
							   tooltip.setTip(0, ToolTipX::TargetPtr(new TargetActionTipButton(ToolTipX::TipPtr(tip), (HWND)lParam)), false);
							   POINT pt;
							   ToolTipX::enPlacement place;
							   if (act->status & ACTSBUTTON_ALIGNRIGHT) {
								   place = ToolTipX::pLeftBottom;
                                   pt = Point(rc.right, rc.bottom);
							   } else {
								   place = ToolTipX::pRightBottom;
                                   pt = Point(rc.left, rc.bottom);
							   }
							   tooltip.attach();
							   tooltip.setPos(pt, false, ToolTipX::positionFixed, place);
							   tooltip.show();
							   SetActiveWindow(tooltip.getTipWindow());
						   }
						   return 0;}
						    

                       case ACTT_FILE: {
                            PropClassCanApply(hWnd);
                            OPENFILENAME of;
                            memset(&of , 0 , sizeof(of));
							// Zapamiêtywanie œcie¿ek...
							sMRU mru;
							CStdString mru_name = "FILE_" + act->txt;
							CStdString mru_result;
							mru.name = mru_name;
							mru.flags = MRU_GET_ONEBUFF;
							mru.buffer = mru_result.GetBuffer(255);
							mru.buffSize = 255;
							mru.count = 1;
							Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_GET , &mru));
							mru_result.ReleaseBuffer();
							of.lpstrInitialDir = mru_result;

                            of.lpstrTitle = (char*)act->txt.c_str();
                            of.hwndOwner = hWnd;
                            of.lStructSize=sizeof(of)-12;
                            of.lpstrFilter="*.*\0*.*\0";
                            GetWindowText(GetNextWindow((HWND)lParam , GW_HWNDPREV) , TLS().buff , MAX_STRING);
                            of.lpstrFile=TLS().buff;
                            of.nMaxFile=MAX_STRING;
//                          of.nFilterIndex=1;
                            of.Flags=OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
                            of.lpstrDefExt="";
                            act->call(ACTN_FILEOPEN , (int)&of , 0);
                            if (GetOpenFileName(&of)) {
								if (!act->call(ACTN_FILEOPENED , (int)&of , 0)) {
									SetWindowText(GetNextWindow((HWND)lParam , GW_HWNDPREV) , of.lpstrFile);
									CStdString dir = of.lpstrFile;
									if (dir.find('\\')!=-1) dir.erase(dir.find_last_of('\\'));
									mru.current = dir;
									Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_SET , &mru));
								}
                            }
                            break;}
                       case ACTT_DIR : {
                            PropClassCanApply(hWnd);
                            CDirDialog dd;
                            dd.m_strTitle = act->txt;
                            GetWindowText(GetNextWindow((HWND)lParam , GW_HWNDPREV) , dd.m_strSelDir.GetBuffer(MAX_PATH) , MAX_PATH);
                            dd.m_strSelDir.ReleaseBuffer(-1);
                            if (dd.DoBrowse()) {
                                SetWindowText(GetNextWindow((HWND)lParam , GW_HWNDPREV) , (char*)dd.m_strPath.c_str());
                            }
                            break;}
                     }

//                     IMessageDirect(IM_UIACTION , (*grp)[i].owner , (int)&(*grp)[i].act() , ACTN_ACTION);
                     break;
                }
                break;}
        case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;

		case WM_ERASEBKGND: {
			HDC hdc = (HDC)wParam;
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
			return 1;
		}
		case WM_NOTIFY:  {
             NMHDR * nm;nm=(NMHDR*)lParam;
             switch (nm->code) {
		     case EN_LINK: case EN_SELCHANGE:
				return cRichEditHtml::ParentWindowProc(hWnd , message , wParam , lParam);
			 }
			 return 0;
		}
		case WM_CLOSE: {
			SendMessage(GetParent(hWnd) , WM_CLOSE , 0 , 0);
			return 0;
		}
	default:
	        return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// ------------------------------------------------------------


INT_PTR CALLBACK ConfigProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam) {
   switch (message)
   {
        NMHDR * nmhdr;
        NMTREEVIEW * nmtv;
		case WM_INITDIALOG: {
                Garbage.addCollection(hwndDlg);

				//GroupBoxSubClass::SubClass(GetDlgItem(hwndDlg , IDG_GROUP));
				break;}
/*        case WM_SIZE:
             return 1;*/
        case WM_CLOSE:
                DestroyWindow(hwndDlg);
                break;
        case WM_DESTROY:
                Garbage.dispose(hwndDlg);
                break;
        case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                if (nmhdr->idFrom==IDC_TREE) {
                     switch (nmhdr->code) {
                       case TVN_SELCHANGED:
                         nmtv=(NMTREEVIEW *)lParam;
                         if (nmtv->itemNew.lParam)
                           openCfgAction(*(cUIGroup *)nmtv->itemNew.lParam);
                         break;
/*                       case TVN_GETINFOTIP: {
                         cUIAction * act = (cUIAction*)((NMTVGETINFOTIP*)lParam)->lParam;
                         SetCfgTip(GetDlgItem(hwndDlg , IDC_TIP) , (act && !act->tip.empty())?act->tip.c_str() : "" , act);
                         break;}*/
                       }
                     }
                break;
        case WM_COMMAND:
                switch (HIWORD(wParam)) {
                 case BN_CLICKED: // Control
		 			 tooltip.reset();
                     switch(LOWORD(wParam)) {
/*                       case IDOK :
                          saveCfg();
                          DestroyWindow(hwndConfig);
                          break;
                       case IDAPPLY:
                          saveCfg();
                          break;
                       case IDCANCEL:
                          DestroyWindow(hwndConfig);
                          break;
                          */
                     }
                     return 1;
                }     
                break;     
        case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;
        case WM_MOUSEWHEEL: {
			tooltip.reset();
            POINT pt = { LOWORD(lParam) , HIWORD(lParam)};
            ScreenToClient(hwndDlg , &pt);
            HWND item = ChildWindowFromPointEx(hwndDlg , pt , CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
            if (item) SendMessage(item , message , wParam , lParam);
            return 0;
            }
		case WM_SETCURSOR:
			tooltip.mouseMovement(hwndDlg);
			break;
		case WM_MOUSELEAVE:
			tooltip.reset();
			break;
		case WM_ACTIVATE: {
			if ((HWND)lParam != tooltip.getTipWindow())
				tooltip.reset();
			break;
			}
		   
   }
   return 0;
}

// --------------------
#define BARCTRMENU_LOCK 1
#define BARCTRMENU_TBLIST 2
#define BARCTRMENU_TBTEXT 3
#define BARCTRMENU_TBDARROWS 4

void ReBarControlMenu(HWND hwnd , int x , int y) {
   cUIGroup * grp = (cUIGroup *)GetProp(hwnd , "ACTION");
   if (!grp) return;
   POINT pt = {x , y};
   ScreenToClient(hwnd , &pt);
   HMENU menu = CreatePopupMenu();
   MENUINFO mif;
   mif.cbSize=sizeof(MENUINFO);
   mif.fMask=MIM_STYLE;
   mif.dwStyle=/*MNS_AUTODISMISS|*/MNS_CHECKORBMP;//|MNS_NOTIFYBYPOS;
//   SetMenuInfo(menu , &mif);
   MENUITEMINFO mi;
   memset(&mi , 0 , sizeof(mi));
   mi.cbSize=sizeof(mi);
   mi.fMask=MIIM_STATE|MIIM_ID|MIIM_FTYPE|MIIM_STRING;
   mi.fType=MFT_STRING;

   mi.wID = BARCTRMENU_LOCK;
   mi.fState = (grp->status & ACTSBAR_LOCKED? MFS_CHECKED : MFS_ENABLED);
   mi.dwTypeData = "Zablokowany";
   InsertMenuItem(menu , -1 ,1,&mi);
   // Pobieramy toolbar ktory tutaj siedzi
   cUIAction * act = 0;
   RBHITTESTINFO rhi;
   rhi.pt = pt;
   SendMessage(hwnd , RB_HITTEST , 0 , (LPARAM)&rhi);
   if (rhi.iBand >= 0) {
       REBARBANDINFO rbbi;
       rbbi.cbSize = sizeof(rbbi);
       rbbi.fMask = RBBIM_LPARAM;
       rbbi.lParam = 0;
       SendMessage(hwnd , RB_GETBANDINFO , rhi.iBand , (LPARAM)&rbbi);
       act = (cUIAction*)rbbi.lParam;
       if (act && act->getType() == ACT_TB) {
           mi.fType = MFT_SEPARATOR;
           InsertMenuItem(menu , -1 ,1,&mi);
           mi.fType = MFT_STRING;
           mi.wID = BARCTRMENU_TBLIST;
           mi.dwTypeData = "Lista";
           mi.fState = (act->status & ACTSTB_LIST? MFS_CHECKED : 0);
           InsertMenuItem(menu , -1 ,1,&mi);
           mi.wID = BARCTRMENU_TBTEXT;
           mi.dwTypeData = "Opisy";
           mi.fState = (act->status & ACTSTB_MIXED? 0 : MFS_CHECKED);
           InsertMenuItem(menu , -1 ,1,&mi);
           mi.wID = BARCTRMENU_TBDARROWS;
           mi.dwTypeData = "Strza³ki";
           mi.fState = (act->status & ACTSTB_DARROWS? MFS_CHECKED : 0);
           InsertMenuItem(menu , -1 ,1,&mi);
       }
   }


   // jedziem
   int ret = TrackPopupMenu(menu,TPM_NONOTIFY | TPM_RETURNCMD,
                       x,y,0,hwnd , 0);
   switch (ret) {
       case BARCTRMENU_LOCK: {
           Act.Mod.changeStatus(*grp , ACTSBAR_LOCKED , ~grp->status);
           // i juz ok...
           break;}
       case BARCTRMENU_TBLIST:
           if (act) 
               Act.Mod.changeStatus(*act , ACTSTB_LIST , ~act->status);
           break;
       case BARCTRMENU_TBTEXT:
           if (act) 
               Act.Mod.changeStatus(*act , ACTSTB_MIXED , ~act->status);
           break;
       case BARCTRMENU_TBDARROWS:
           if (act) 
               Act.Mod.changeStatus(*act , ACTSTB_DARROWS , ~act->status);
           break;
   }
   // robim

   DestroyMenu(menu);
}

int CALLBACK ReBarControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
   switch (message)
   {
   case WM_SETCURSOR: {
	   tooltip.mouseMovement(hwnd);
	   return 0;}
   case WM_LBUTTONDOWN:
	   tooltip.hide();
	   break;
   case WM_CONTEXTMENU: {
	   tooltip.hide();
       // Pokazujemy Menu!
       ReBarControlMenu(hwnd , LOWORD(lParam) , HIWORD(lParam));
       return 0;}
   case WM_DESTROY: {
       cUIGroup * grp = (cUIGroup *)GetProp(hwnd , "ACTION");
       if (grp)
           Act.Mod.saveBar(*grp);
       break;}
   }
   return CallWindowProc((WNDPROC)GetProp(hwnd , "oldWNDPROC") , hwnd , message , wParam, lParam);
}
