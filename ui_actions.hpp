
using Tables::tTableId;

int cfgNeedRestart = 0;

struct GroupBoxFilledSubClass {
	static WNDPROC oldProc;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{ 
		switch (msg) 
		{ 
			case WM_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				RECT rc;
				/*HRGN hrgn = CreateRectRgn(0,0,0,0);
				GetUpdateRgn(hwnd, hrgn, true);
				FillRgn(hdc, hrgn, GetSysColorBrush(COLOR_BTNFACE));
				DeleteObject(hrgn);
				*/
/*
				GetClientRect(hwnd, &rc);
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
				*/
				GetChildRect(hwnd, &rc);
				HRGN hrgn = CreateRectRgnIndirect(&rc);
				HWND sibling = GetWindow(GetParent(hwnd), GW_CHILD);
				do {
					if (sibling == hwnd) continue;
					RECT src;
					HRGN hr1, hr2;
					hr1 = hrgn;
					hrgn = CreateRectRgn(0,0,0,0);
					GetChildRect(sibling, &src);
					hr2 = CreateRectRgnIndirect(&src);
					int res = CombineRgn(hrgn, hr1, hr2, RGN_DIFF);
					DeleteObject(hr1);      
					DeleteObject(hr2);
				}while (sibling = GetWindow(sibling, GW_HWNDNEXT));
				//GetUpdateRgn(hwnd, hrgn, true);
				//GetClientRect(hwnd, &rc);
				OffsetRgn(hrgn, -rc.left, -rc.top);
				FillRgn(hdc, hrgn, GetSysColorBrush(COLOR_BTNFACE));
				//FrameRgn(hdc, hrgn, GetSysColorBrush(COLOR_3DSHADOW), 1,1);
				DeleteObject(hrgn);


				return 1;
			}
//			case WM_NCHITTEST :
//				SetCursor(LoadCursor(0,IDC_ARROW));
//				return 1;
		} 

		// Call the original window procedure for default processing. 
		return CallWindowProc(oldProc, hwnd, msg, wParam, lParam); 
	}
	static SubClass(HWND hwnd) {
		oldProc = (WNDPROC)SetWindowLong(hwnd , GWL_WNDPROC , (LONG)WndProc);
	}
};
WNDPROC GroupBoxFilledSubClass::oldProc = 0;


 int fillProfiles(int where) {
   string dir = string((char*)ICMessage(IMC_PROFILESDIR)) + "*.*";
//   IMLOG("PROFILE %s" , dir.c_str());
   WIN32_FIND_DATA fd;
   HANDLE hFile;
   BOOL found;
   found = ((hFile = FindFirstFile(dir.c_str(),&fd))!=INVALID_HANDLE_VALUE);

   int i = 0;
   string profile = (char*)ICMessage(IMC_GETPROFILE);
   while (found)
   {
      found = FindNextFile(hFile , &fd);
      if (found && strcmp(fd.cFileName , "..") && fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        Act[IMIG_MAIN_PROFILE].insert(IMIB_PROFILE | i , -1 , fd.cFileName , (profile==fd.cFileName?ACTS_CHECKED:0));
        i++;
      }
   }
   FindClose(hFile);
   return 0;
 } 



 HMENU makeActionsMenu(cUIGroup & g , int startFrom , bool create) {
   if (!g.getType()) g.setType(ACT_MENU);
   if (IsMenu((HMENU)g.ghandle))
                if (create) DestroyMenu((HMENU)g.ghandle);
                   else return (HMENU)g.ghandle;
   HMENU handle;
   g.count=startFrom;
   if (g.status & ACTR_INIT) g.call(ACTN_CREATEGROUP , 0 , 0);
   handle = (HMENU)CreatePopupMenu();
   if (g.getType() == ACT_MENU) {g.ghandle=handle;}
   MENUINFO mif;
   mif.cbSize=sizeof(MENUINFO);
   mif.fMask=MIM_STYLE|MIM_MENUDATA;
   mif.dwStyle=/*MNS_AUTODISMISS|*/ (g.status & ACTSMENU_CHECKSPACE ? 0 : MNS_CHECKORBMP);//|MNS_NOTIFYBYPOS;
   mif.dwMenuData=(unsigned long)&g;
   SetMenuInfo((HMENU)handle , &mif);
   int next_fType = 0;
   for (int i=startFrom;i<g.size();i++) {
     if (g[i].status & ACTR_INIT/* && !(g[i].status & ACTS_GROUP)*/) g[i].call(ACTN_CREATE , (int)handle , 0);
     if (g[i].status & ACTS_HIDDEN) continue;
     if (g[i].statusType() == ACTT_HWND) continue;
     g.count++;
     MENUITEMINFO mi;
     mi.cbSize=sizeof(MENUITEMINFO);
     mi.fMask=MIIM_DATA|MIIM_STATE|MIIM_ID|MIIM_FTYPE;
     mi.fType=next_fType;
     next_fType = 0;
//     g[i].status=(g[i].status & ~ACTS_T_MASK) | ACTS_T_MENU;
     if ((g[i].status & ACTM_TYPE) == ACTT_BARBREAK) next_fType|=MFT_MENUBARBREAK;
     else if ((g[i].status & ACTM_TYPE) == ACTT_BREAK) next_fType|=MFT_MENUBREAK;
     else if ((g[i].status & ACTM_TYPE) == ACTT_SEP) mi.fType|=MFT_SEPARATOR;
     if (!((g[i].status & ACTM_TYPE) == ACTT_SEP)) {
         mi.fMask|=MIIM_STRING;
         mi.dwTypeData=(char *)g[i].txt.c_str();
         mi.cch=g[i].txt.size();
       }
	 
     if (1 || g[i].p1 || mi.fType == 0 || (mi.fType & MFT_SEPARATOR)) {
         //mi.fMask |= MIIM_BITMAP;
//         mi.hbmpItem=(HBITMAP)g[i].bmp;
         //mi.hbmpItem = HBMMENU_CALLBACK;
		 mi.fType |= MFT_OWNERDRAW;
     }
	 
     mi.fState=MFS_ENABLED;
     if (g[i].status & ACTS_CHECKED)
       mi.fState |= MFS_CHECKED;
     if (g[i].status & ACTS_DISABLED) {
       mi.fState |= MFS_DISABLED;
     } 
     if (g[i].status & ACTSMENU_BOLD) {
		 mi.fState |= MFS_DEFAULT;
     } 
     mi.wID=g[i].index;
     if (g[i].status & ACTS_GROUP) {
       mi.fMask|=MIIM_SUBMENU;
       mi.hSubMenu=makeActionsMenu(g[i].getGroup(),0,create);
     }
     mi.dwItemData=(ULONG_PTR)&g[i];

     if (!InsertMenuItem((HMENU)handle , g.count ,1,&mi)) {
		 IMDEBUG(DBG_ERROR, "InsertMenuItem error = %d", GetLastError());
		 //ShowLastError();
     }
   }
   return (HMENU)handle;
 }

 void destroyActionsMenu(cUIGroup & g) {
     for (int i=0;i<g.size();i++) {
       if (g[i].status & ACTS_GROUP) destroyActionsMenu(g[i].getGroup());
         else {if (g[i].status & ACTR_INIT) g[i].call(ACTN_DESTROY, 0 , 0);
            if (g[i].getType()==ACT_MENU) g[i].handle = 0;
         }
     }
     if (g.status & ACTR_INIT) g.call(ACTN_DESTROY , 0 , 0);
     if (g.getType() == ACT_MENU && g.ghandle) DestroyMenu((HMENU)g.ghandle);
     if (g.getType()==ACT_MENU) g.ghandle = 0;

 }

//---------------------------------------------------

void makeCfgTree(cUIGroup & g , HWND tree) {
   TreeView_SetIndent(tree , 5); // Ustawia wciecie drzewka
   g.setType(ACT_CFG);
   if (g.status & ACTR_INIT) g.call(ACTN_CREATEGROUP , 0 , 0);
   for (int i=0;i<g.size();i++) {
     g[i].setType(ACT_CFG);
     g[i].cnt = g.cnt;
     if (!(g[i].status & ACTS_GROUP)) continue;
     if (g[i].status & ACTR_INIT)
       g[i].call(ACTN_CREATE , 0 , 0);
     if (g[i].status & ACTS_HIDDEN) continue;
     TVINSERTSTRUCT tvs;
     TVITEMEX tie;
//     ZeroMemory(&tie, sizeof(TVITEM));
//     ZeroMemory(&tvs, sizeof(TVINSERTSTRUCT));
     tie.mask=TVIF_PARAM|TVIF_STATE|TVIF_TEXT;
     tie.state=0;
     tie.stateMask=0;
     tie.pszText=(char*)g[i].txt.c_str()  ;
     if (g[i].p1) {
       tie.mask|=TVIF_IMAGE|TVIF_SELECTEDIMAGE;
       tie.iImage=Ico[g[i].p1].index[0];
       tie.iSelectedImage=Ico[g[i].p1].index[0];
     }
     tie.lParam=(ULONG_PTR)&g[i];
     tvs.hParent=(HTREEITEM)g.handle;
     tvs.hInsertAfter=TVI_LAST;
     tvs.itemex=tie;
     g[i].handle=(void*)SendMessage(tree , TVM_INSERTITEM , 0 , (long)&tvs);
     makeCfgTree(g[i].getGroup() , tree);
   }
}

string funkcja() {
  string str="test"; 
  return str;
}

const char * getCfgValue(cUIAction & a , char * buff , int size) {
	int type;
	const char * ch = "";
	if ((a.type & ACT_FORCNT && a.cnt == AC_NONE)) return "";
	if (a.p1 && !(a.status & ACTR_NODATASTORE)) {
		type=Ctrl->DTgetType((a.type & ACT_FORCNT)?DTCNT:DTCFG , a.p1&0x0FFFFFFF);
	//    if (!buff) {buff = TLS().buff;size = MAX_STRING;}

		tTableId db = (a.type & ACT_FORCNT)?DTCNT : DTCFG;
		int row = (a.type & ACT_FORCNT)?a.cnt : 0;
		Tables::Value v(Tables::ctypeString);
		Ctrl->DTget(db , row , a.p1&0x0FFFFFFF , &v);
		ch = v.vCChar?v.vCChar:"";
		if (buff/* != TLS().buff*/) {strncpy(buff , ch , size);ch = buff;}
	};
	if (a.id && (!a.p1 || a.status & ACTR_CONVERT)) { // Pobieramy cala wartosc, lub tylko sprawdzamy to, co juz pobralismy...
		sUIActionNotify_buff an;
		an.act = a.act();
		type=!(a.status & ACTSC_INT);
		if (!buff) {buff = TLS().buff;size = MAX_STRING;
			strncpy(buff , ch , size);
		} else if (!*ch) buff[0]=0;
  		an.buff=buff;
		an.buffSize=size;
		an.code = ACTN_GET;
		a.call(&an);
		an.code = ACTN_CONVERT_TO;
		a.call(&an);
	//    if (a.status & ACTS_CHECK) IMessageDirect(IM_UIACTION , a.owner , (int)sa , ACTF_CHECK);
		return (char*)SAFECHAR(an.buff);
  }
  return ch;
}

void setCfgValue(cUIAction & a , char * v , size_t size) {
    if ((a.type & ACT_FORCNT && a.cnt == AC_NONE)) return;
	sUIActionNotify_buff an;
	an.act = a.act();
    if (a.id && (!a.p1 || a.status & ACTR_CONVERT)) {
        an.buff=v;
		an.buffSize=size;
		an.code = ACTN_CONVERT_FROM;
		a.call(&an);
		an.code = ACTN_SET;
		a.call(&an);
        if (an.buff) v=an.buff; else return;
    }
    if (a.p1 && !(a.status & ACTR_NODATASTORE)) {
        tTableId db = (a.type & ACT_FORCNT)?DTCNT : DTCFG;
        int row = (a.type & ACT_FORCNT)?a.cnt : 0;
		Tables::Value val(Tables::ctypeString);
        if (a.status & ACTSC_NEEDRESTART) {
            Ctrl->DTget(db , row , a.p1&0x0FFFFFFF , &val);
            if (strcmp(v , val.vCChar)) {cfgNeedRestart++;}
        }
        val.vCChar = v;
        Ctrl->DTset(db , row , a.p1&0x0FFFFFFF , &val);
    }
}

const char * checkCfgValue(cUIAction & a,char * v) {
  sUIActionNotify an=sUIActionNotify(a.act() , ACTN_CHECK , (int)v , 0);
  IMessageDirect(IM_UIACTION , a.owner , (int)&an);
  return (char*)an.notify1;
}

HWND getCfgHandle(cUIAction & act) {
	if ((act.type & ACT_BRANCHMASK)!=ACT_CFGBRANCH) return 0;
    if (act.type & ACT_FORCNT) {
        if (Cnt.exists(act.cnt))
            return Cnt[act.cnt].hwndInfo;
        else return 0;
    } else {
        return hwndConfig;
    }
}


#define INDENT_GROUP 10
// ------------------------------------------------------------------
#define CFGDISABLED(g) ((g.status & ACTS_DISABLED || g.status & ACTS_HIDDEN)?WS_DISABLED:0)

void closeGroupBox(HWND lastGroupWnd, int y) {
    if (lastGroupWnd) {
		RECT rc;
		GetChildRect(lastGroupWnd , &rc);
		SetWindowPos(lastGroupWnd , 0 , 0 , 0 , rc.right-rc.left , y - rc.top +2 , SWP_NOMOVE|SWP_NOZORDER);
		lastGroupWnd=0;
    }
}


//#define WS_EX_COMPOSITED        0x02000000L
HWND makeCfgProp(cUIGroup & g ) {
   HWND hWnd, hwndConfig , item;
   hwndConfig=getCfgHandle(g);
   cGarbageCollection * GarbageCfg = Garbage.findCollection(hwndConfig);
   int b=0;
   int w , h;
   hWnd=CreateWindowEx(WS_EX_CONTROLPARENT ,"PROP" , "" ,  WS_CHILD|WS_VSCROLL|WS_CLIPCHILDREN|WS_CLIPSIBLINGS | CFGDISABLED(g)
         ,150,25,270,200,hwndConfig /*GetDlgItem(hwndConfig , IDG_GROUP)*/ , (HMENU)g.index,hInst,&g);

   SetProp(hWnd , "CNTID" , (void*)g.cnt);
   g.ghandle=hWnd;
   sUIActionNotify an;
//   if (g.status & ACTR_INIT)
//       g.call(ACTN_CREATE);
//   IMessageDirect(IM_UIACTION , g.owner , (int)g.fill() , ACTF_CREATE);
   int x=0 , y=0;
   int rh=0; // Wysokosc linijki
   HWND lastGroupWnd = 0;
   int lastType = 0;
   int indent = 0;
   int inHidden = 0;
   int minPageWidth = 0;
   for (int i = 0 ; i<g.size() ; i++) {
     if (g[i].type & ACT_ISGROUP) continue;
     h = w = 0;
     g[i].setType(ACT_CFG);
     g[i].cnt = g.cnt;
     if (g[i].status & ACTR_INIT) { // Wysyla Create
       an.act=g[i].act();
       an.code=ACTN_CREATE;
       an.notify1 = (int)hWnd;
       IMessageDirect(IM_UIACTION , g[i].owner , (int)&an);
//       if ((g[i].status & ACTS_TYPEMASK)==ACTS_TWND) { // ustawia x,y dla TWND
//         x=LOWORD(sa->p1);
//         y=HIWORD(sa->p1);
//       }
//       g[i].handle=sa->handle;
     }
     int curType = g[i].status & ACTM_TYPE;
     if (g[i].status & ACTS_HIDDEN || inHidden) {
         if (curType == ACTT_GROUP) inHidden++;
         else if (curType == ACTT_GROUPEND) inHidden--;
         g[i].handle = 0;
         y+=rh;x=indent;rh=0;
         continue;}
     bool created = true;
     bool wndCreate = true;
	 bool fontSet = true;
     bool wnd = true;
	 bool tipByHwnd = true;
	 bool offsetPosition = true;
     int minWidth = 0;
     int wndExStyle = 0 , wndStyle = WS_VISIBLE|WS_CHILD;
	 HFONT useFont = g[i].status & ACTSC_BOLD ? fontB : font;
     CStdString wndClass = "" , wndTxt="";
     x+=((cUIAction_cfg&)g[i]).x;
     y+=((cUIAction_cfg&)g[i]).y;
     w=((cUIAction_cfg&)g[i]).w;
     h=((cUIAction_cfg&)g[i]).h;
     int oy=2,ox=5;  // offsety
	 int roy = 0;
     switch (curType) {
     case ACTT_INFO:
		 if (!w) {
			  w = 250;
			  g[i].status |= ACTSC_FULLWIDTH;
			  g[i].status &= ~ACTSC_INLINE;
		  }
		 if (!h) {
			 h = GetTextSize(hWnd, useFont, g[i].txt.c_str(), w).cy + 2;
		 }
		 if (h < 0) h = abs(h) * (fontHeight) + 2;
          g[i].handle=CreateWindowEx(0 , RICHEDIT_CLASS , g[i].txt.c_str() 
              , WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL
              | (g[i].status & ACTSINFO_CENTER?ES_CENTER:0)
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          wndCreate=false;
          SendMessage((HWND)g[i].handle , EM_SETBKGNDCOLOR , 0 , GetSysColor(COLOR_BTNFACE));
           
          break;
	 case ACTT_HTMLINFO:
          //if (!h) h = -2;
		  if (!h) {
			  CStdString copy;
			  cPreg pr(false);
			  copy = pr.replace("/[\\r\\n]+/", "", g[i].txt.c_str());
			  copy = pr.replace("/<br */? *>/", "\r\n", copy);
			  copy = pr.replace("/<[^>]+>/", "", copy);
			  h = GetTextSize(hWnd, useFont, copy, w).cy + 2;
		  }

		  if (h < 0) h = abs(h) * (fontHeight) + 2;
		  IMLOG("HIX=%d x=", ((cUIAction_cfg&)g[i]).x, x);
		  g[i].handle = cRichEditHtml::Create((g[i].status & ACTSINFO_CENTER?ES_CENTER:0) , x , y , w , h , hWnd , (HMENU)g[i].index);
	      SendMessage((HWND)g[i].handle , WM_SETFONT , (WPARAM)font , 1);
          wndCreate=false;
		  fontSet = false;
		  cRichEditHtml::SetHTML((HWND)g[i].handle , g[i].txt , HTMLInfoStyleCB);
/*		  if (!w) {
			g[i].status |= ACTSC_FULLWIDTH;
			g[i].status &= ~ACTSC_INLINE;
		  }*/
		  if (!w) {
			g[i].status |= ACTSC_FULLWIDTH;
			g[i].status &= ~ACTSC_INLINE;
		  }
           
          break;


       case ACTT_COMMENT:
          wndClass="STATIC";
          wndTxt=g[i].txt;
          wndStyle|=SS_NOPREFIX|SS_CENTERIMAGE;
/*		  if (!(g[i].status & ACTSC_INLINE)) {
			  g[i].status |= ACTSC_FULLWIDTH;
		  }*/
		  if (!w) {
			 w = GetTextSize(hWnd, useFont, g[i].txt.c_str(), w).cx + 2;
		  }
          oy = 2;
          break;
	   case ACTT_SEPARATOR:
		   y+=2;
		   h = h?h : (g[i].txt.empty() ? 5 : fontHeight);
           if (!w) {
			   w = 200;g[i].status |= ACTSC_FULLWIDTH;
               g[i].status &= ~ACTSC_INLINE;
		   }
           g[i].handle=CreateWindow("STATIC","" , wndStyle | SS_ETCHEDHORZ
	            ,x,y + (g[i].txt.empty() ? 0 : fontHeight/2),w,5,hWnd,(HMENU)g[i].index,hInst,0);
		   wndCreate = false;
           oy=3;
		   if (!g[i].txt.empty()) {
			   CStdString txt = " " + g[i].txt + " ";
               HWND label = CreateWindow("STATIC",txt , wndStyle
	               ,x + 16,y,GetTextSize(hWnd, useFont, txt).cx,fontHeight,hWnd,0,hInst,0);
			   SendMessage(label , WM_SETFONT , (WPARAM)useFont , 1);
 		       SetProp((HWND)g[i].handle, "connectedWnd", (HANDLE)label);
		   }
           break;
       case ACTT_CHECK:
          //w = w?w:100;
		   if (w == 0) {
			 w = GetTextSize(hWnd, useFont, g[i].txt.c_str()).cx + 22;
		   }
          h = h?h:fontHeight + 6;
          g[i].handle=CreateWindow("BUTTON",g[i].txt.c_str() , WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX |  CFGDISABLED(g[i])
            | (curType!=lastType?WS_TABSTOP:0)
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          wndCreate=false;
          //if (!(g[i].status & ACTSC_INLINE)) g[i].status |= ACTSC_FULLWIDTH;
          SendMessage((HWND)g[i].handle , BM_SETCHECK , (getCfgValue(g[i])[0]!='0')?BST_CHECKED:BST_UNCHECKED , 0);
          break;
       case ACTT_IMAGE: {
          wndCreate=false;
		  g[i].handle=CreateWindow("STATIC","" , WS_VISIBLE|WS_CHILD|SS_CENTER|SS_REALSIZEIMAGE
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          ((cUIAction_cfg*)&g[i])->setImage(g[i].txt.c_str());
		  w = ((cUIAction_cfg&)g[i]).w; // mog³y siê zmieniæ...
		  h = ((cUIAction_cfg&)g[i]).h;
          break;}
	   case ACTT_TIPBUTTON: 
		  if (!g[i].paramExists(AP_ICO) && !g[i].paramExists(AP_IMGURL)) {
			  g[i].setParam(AP_ICO, inttoch(ICON_HELP));
		  }
		  {
			  int size = atoi(g[i].getParam(AP_ICONSIZE).c_str());
			  if (!size) size = 16;
			  if (!w) {
				  w = size + 8 + GetTextSize(hWnd, useFont, g[i].txt.c_str(), w).cx + 2;
			  }
			  if (!h) {
                  h = size+8;
			  }
		  }
		  wndStyle |= BS_PUSHLIKE | BS_CHECKBOX;
	   case ACTT_LINK:
	   case ACTT_BUTTON: {
		   bool fixedW = w > 0;
		   bool fixedH = h > 0;
		   if (h < 0) h = abs(h) * (fontHeight) + 2;
		   if (curType == ACTT_LINK) {
			h = h?h:fontHeight + 0;
		   } else {
			w = w?w:100;
			h = h?h:fontHeight + 10;
		   }
		   SIZE sz = GetTextSize(hWnd, useFont, g[i].txt.c_str());
		   w = max(w, sz.cx + 8);

		   if (g[i].paramExists(AP_ICO) || g[i].paramExists(AP_IMGURL)) {
			   int size = atoi(g[i].getParam(AP_ICONSIZE));
			   if (!size) size = 16;
			   if (!fixedW)
					w += size + 4;
			   if (!fixedH)
				   h = max(h, size + 4);
		   }

		  //h = max(h, sz.cy + 10);

		  oy+=1;
          wndCreate=false;
/*		  g[i].handle=CreateWindowEx(0,"BUTTON","" , WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON |  CFGDISABLED(g[i]) | WS_TABSTOP
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
			*/
		  ButtonX * button = new ButtonX("", wndStyle | ( curType == ACTT_LINK ?BS_OWNERDRAW : 0) | BS_MULTILINE| CFGDISABLED(g[i]) | WS_TABSTOP
			  ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
		  g[i].handle = button->getHWND();
          ((cUIAction_cfg*)&g[i])->setButton(g[i].txt.c_str());
		  if (g[i].status & ACTSBUTTON_ALIGNRIGHT) {
			  g[i].status &= ~ACTSC_FULLWIDTH | ACTSC_INLINE;
			  SetProp(button->getHWND(), "AlignRight", (HANDLE)1);
			  offsetPosition = false;
		  }
          //SendMessage((HWND)g[i].handle , BM_SETCHECK , (getCfgValue(g[i])[0]!='0')?BST_CHECKED:BST_UNCHECKED , 0);
		  break; }
       case ACTT_PASSWORD:
       case ACTT_EDIT:
		   oy+=1;
          wndClass="EDIT";
          wndExStyle = WS_EX_CLIENTEDGE;
          wndStyle |=CFGDISABLED(g[i]) | ES_AUTOHSCROLL
            | (g[i].status&ACTSC_INT?ES_NUMBER:0)  |WS_TABSTOP
            | ((g[i].status & ACTM_TYPE)==ACTT_PASSWORD ? ES_PASSWORD:0)
			| (g[i].status & ACTSEDIT_READONLY?ES_READONLY:0)
			;
          wndTxt = getCfgValue(g[i]);
          break;
       case ACTT_TEXT:
          h = h?h : -4;
		  if (h < 0) h = abs(h) * (fontHeight + 1) + 4;
          g[i].status |= ACTSC_FULLWIDTH;
          g[i].status &= ~ACTSC_INLINE;
          wndClass="EDIT";
          wndExStyle = WS_EX_CLIENTEDGE;
          wndStyle |=CFGDISABLED(g[i]) | ES_AUTOVSCROLL
            | (g[i].status&ACTSC_INT?ES_NUMBER:0)  |WS_TABSTOP | ES_MULTILINE |ES_WANTRETURN|WS_VSCROLL
			| (g[i].status & ACTSEDIT_READONLY?ES_READONLY:0)
			;
          wndTxt = getCfgValue(g[i]);
          break;
       case ACTT_GROUP:
          if (!w) {
			  w = 200;
			  g[i].status |= ACTSC_FULLWIDTH;
		  }
          if (!h) h = 2 * fontHeight + 6;
		  g[i].handle=CreateWindowEx(0 , "BUTTON",g[i].txt.c_str() , WS_VISIBLE|WS_CHILD|BS_GROUPBOX | CFGDISABLED(g[i])
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
		  GroupBoxFilledSubClass::SubClass((HWND)g[i].handle);
		  if (!lastGroupWnd) lastGroupWnd=(HWND)g[i].handle;
		  h=g[i].txt.empty()?10:fontHeight;
          wndCreate=false;
		  tipByHwnd = false;
          indent += INDENT_GROUP;
          x = indent;
          break;
       case ACTT_GROUPEND:
          g[i].handle = (void*)0/*-1*/; /*TODO: Sprawdziæ! Czy -1 tutaj nie szkodzi!!!*/
		  closeGroupBox(lastGroupWnd, y);
          indent-=INDENT_GROUP;
          indent = max(0 , indent);
		  lastGroupWnd = 0;
          x=indent;y+=6;rh=0;
          wnd=wndCreate=false;
  		  tipByHwnd = false;
          break;
	   case ACTT_FILE: case ACTT_DIR:{
          w = w?w:200;
          h = h?h:fontHeight + 6;
          g[i].handle=CreateWindowEx(WS_EX_CLIENTEDGE , "EDIT",getCfgValue(g[i]) , WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL | WS_TABSTOP | CFGDISABLED(g[i])
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
//          item = CreateWindow("BUTTON","..." , WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON |  CFGDISABLED(g[i])
//            ,x+w+2,y,2*h,h,hWnd,0,hInst,0);
		  ButtonX * button = new ButtonX("", WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON |  CFGDISABLED(g[i])
			  ,x+w+2,y,1*h,h,hWnd,0,hInst,0);
		  item = button->getHWND();
		  button->setIcon(Ico[ICON_SEARCH].ico_, false);
          w += 1*h+2;  
          SetProp(item , "ACTION" , (void*)&g[i]);
          SetProp(item , "PART" , (void*)1);
          SendMessage(item , WM_SETFONT , (WPARAM)font , 1);
          wndCreate = false;
  		  tipByHwnd = false;
          g[i].status &= ~ACTSC_FULLWIDTH;
		  break;}
       case ACTT_COLOR: {
          w = w?w:50;              
          h = h?h:fontHeight + 6;
          CColorPicker * CP = new CColorPicker(atoi(getCfgValue(g[i])) , WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])
              | (g[i].status & ACTSCOLOR_CHECKBOX?CP_CHECKBOX:0)
              ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          g[i].handle = (HWND) CP->_hwnd;
          wndCreate = false;
          g[i].status &= ~ACTSC_FULLWIDTH;
          break;
          }
       case ACTT_FONT: {
          w = w?w:100;              
          h = h?h:fontHeight + 6;
          CFontPicker * FP = new CFontPicker(g[i].txt.c_str(), getCfgValue(g[i]) 
              ,  (g[i].status & ACTSFONT_NOCOLOR?CF_NOCOLORSEL:0) 
                |(g[i].status & ACTSFONT_NOBGCOLOR?CF_NOBGCOLORSEL:0)
                |(g[i].status & ACTSFONT_NOFACE?CF_NOFACESEL|CF_NOSCRIPTSEL:0)
                |(g[i].status & ACTSFONT_NOSIZE?CF_NOSIZESEL:0)
                |(g[i].status & ACTSFONT_NOSTYLE?CF_NOSTYLESEL:0)
              ,  WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])
              | (g[i].status & ACTSFONT_CHECKBOX?FP_CHECKBOX:0)
              ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          g[i].handle = (HWND) FP->_hwnd;
          wndCreate = false;
          break;
          }
       case ACTT_COMBO: {
		   if (!w) {
			  //w = GetTextSize(hWnd, useFont, g[i].txt.c_str()).cx;
			   w = 120;
		   }
		  h = h?h:100; //< wysokoœæ listy
		  g[i].handle = CreateWindowEx(0 , /*(g[i].status & ACTSCOMBO_LIST)?*/WC_COMBOBOXEX/*:"COMBOBOX"*/ , "" , WS_CLIPCHILDREN | WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])|(g[i].status & ACTSCOMBO_LIST?CBS_DROPDOWNLIST:CBS_DROPDOWN)| CBS_AUTOHSCROLL|WS_TABSTOP
                                    ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          h = fontHeight + 10; //< wysokoœæ kontrolki
          if (/*g[i].status & (ACTSCOMBO_LIST) &&*/ !(g[i].status & ACTSCOMBO_NOICON)) {
              SendMessage((HWND)g[i].handle , CBEM_SETIMAGELIST , 0 , (LPARAM)Ico.iml[0]);
          } 
          ((cUIAction_cfg*)&g[i])->setComboItems(g[i].txt.c_str());
          setActionValue(g[i] , getCfgValue(g[i]));

/*		  // Ustawiamy TIPa
          if (!g[i].tip.empty()) {
              SetProp((HWND) GetWindow((HWND)g[i].handle,GW_CHILD) , "TIP" , (void*)g[i].tip.c_str());
              if (g[i].status & ACTSCOMBO_LIST) SetProp((HWND) GetWindow(GetWindow((HWND)g[i].handle,GW_CHILD),GW_CHILD) , "TIP" , (void*)g[i].tip.c_str());
          }
*/
          wndCreate = false;
  		  tipByHwnd = false;
          break;
          }
       case ACTT_TIME:
          w = w?w:100;
          h = h?h:fontHeight + 6;
          g[i].handle=CreateWindowEx(0,DATETIMEPICK_CLASS,"" , WS_VISIBLE|WS_CHILD |  CFGDISABLED(g[i]) | (g[i].status & ACTSTIME_SHOWNONE?DTS_SHOWNONE:0) | (g[i].status & ACTSTIME_UPDOWN?DTS_UPDOWN:0)
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          wndCreate=false;
          DateTime_SetFormat((HWND)g[i].handle , g[i].txt.c_str());
          setActionValue(g[i] , getCfgValue(g[i]));
  		  tipByHwnd = false;
          break;
       case ACTT_HWND:{
                sUIActionNotify_createWindow ancw;
                ancw.act = g[i].act();
                ancw.hwndParent = hWnd;
                ancw.status = g[i].status;
                ancw.txt = g[i].txt.c_str();
                ancw.x=x;
                ancw.y=y;
                ancw.font = font;
                ancw.fontBold = fontB;
                g[i].call(&ancw);
                g[i].handle = ancw.hwnd;
                if (!ancw.x) g[i].status&=~ACTSC_INLINE;
                w = ancw.x -x;
                h = ancw.y -y;
                wndCreate = false;
  		  tipByHwnd = false;
           break;}
       case ACTT_RADIO:
		   if (!w) {
   			  w = GetTextSize(hWnd, useFont, g[i].txt.c_str()).cx + 24;
		   }
          h = h?h:fontHeight + 6;
          g[i].handle=CreateWindow("BUTTON","" , WS_VISIBLE|WS_CHILD|BS_RADIOBUTTON|CFGDISABLED(g[i])
            | (curType!=lastType?WS_TABSTOP:0)
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          wndCreate=false;
          ((cUIAction_cfg*)&g[i])->setRadio("");
          //if (!(g[i].status & ACTSC_INLINE)) g[i].status |= ACTSC_FULLWIDTH;
		  if (g[i].status & ACTSRADIO_LAST) {
	          setActionValue(g[i] , getCfgValue(g[i]));
		  }
          break;
       case ACTT_SLIDER:
          w = w?w:200;
          h = h?h:2 * fontHeight + 6;
		  g[i].handle=CreateWindowEx(0, TRACKBAR_CLASS,"" 
			  , WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i]) | (curType!=lastType?WS_TABSTOP:0)
			    | TBS_AUTOTICKS | TBS_BOTTOM | TBS_TOOLTIPS
              ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
          wndCreate=false;
          if (!(g[i].status & ACTSC_INLINE)) g[i].status |= ACTSC_FULLWIDTH;
		  // tworzymy "buddy" windows
		  if (!g[i].txt.empty()) {
			  HWND buddy1=CreateWindowEx(0, "STATIC","" 
					, WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])
					,x,y+h,w/2,18,hWnd,0,hInst,0);
			  SendMessage(buddy1 , WM_SETFONT , (WPARAM)(g[i].status & ACTSC_BOLD?font:fontLabel) , 1);
			  HWND buddy2=CreateWindowEx(0, "STATIC","" 
				  , WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])|ES_RIGHT
					,x+(w/2),y+h,w/2,18,hWnd,0,hInst,0);
			  SendMessage(buddy2 , WM_SETFONT , (WPARAM)(g[i].status & ACTSC_BOLD?font:fontLabel) , 1);
			  SetProp((HWND)g[i].handle, "connectedWnd", (HANDLE)buddy1);
			  SetProp(buddy1, "connectedWnd", (HANDLE)buddy2);
			  //SendMessage((HWND)g[i].handle , TBM_SETBUDDY , 1, (LPARAM)buddy1);
			  //SendMessage((HWND)g[i].handle , TBM_SETBUDDY , 0, (LPARAM)buddy2);
			  if (g[i].status & ACTSC_FULLWIDTH) {
				  SetProp(buddy2 , "FullWidth" , (void*)1);
			  }
			  h+=fontHeight + 4;
		  }
          ((cUIAction_cfg*)&g[i])->setSlider("");
          setActionValue(g[i] , getCfgValue(g[i]));
          break;
	   case ACTT_SPINNER:  {
          w = w?w:100;
          h = h?h:fontHeight + 6;
          g[i].handle=CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT",getCfgValue(g[i]) 
			  , WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i])|ES_AUTOHSCROLL|ES_NUMBER
                | (curType!=lastType?WS_TABSTOP:0)
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
		  HWND spinner = CreateWindowEx(0, UPDOWN_CLASS, ""
			  , WS_VISIBLE|WS_CHILD|CFGDISABLED(g[i]) 
			    | UDS_ARROWKEYS | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_NOTHOUSANDS | UDS_SETBUDDYINT | UDS_WRAP
				| (curType!=lastType?WS_TABSTOP:0)
				, 0, 0, w, h, hWnd,(HMENU)g[i].index,hInst,0); 
				;
          wndCreate=false;
		  SetProp((HWND)g[i].handle, "connectedWnd", (HANDLE)spinner);
          ((cUIAction_cfg*)&g[i])->setSpinner("");
          setActionValue(g[i] , getCfgValue(g[i]));
  		  tipByHwnd = false;
		  break;}

       default:
          created = false;
          wnd = false;
          wndCreate=false;
     }

	 if (wnd) {
       w = w?w:100;
       h = h?h:fontHeight + 6;
	   if (offsetPosition) {
			rh=max(rh , h + oy);
	   }
//	   if (h + oy > rh + roy)
//		   roy = 
	   roy = 0;//max(roy, oy);
	   if (!minWidth) {
		   if (g[i].paramExists(AP_MINWIDTH)) {
			   minWidth = x + atoi(g[i].getParam(AP_MINWIDTH));
		   } else
				minWidth = x+w;
		   if (lastGroupWnd)
			   minWidth += 2*indent; // indent z drugiej strony...
	   }
	   if (minWidth > minPageWidth)
		   minPageWidth = minWidth;
       if (wndCreate) {
         g[i].handle=CreateWindowEx(wndExStyle , wndClass,wndTxt , wndStyle
            ,x,y,w,h,hWnd,(HMENU)g[i].index,hInst,0);
       }
	   if (curType != ACTT_GROUP && curType != ACTT_GROUPEND && curType != ACTT_TIPBUTTON && g[i].handle) {
		   RECT tipRc;
		   tipRc.left = x - 3;
		   tipRc.top = y - 3;
		   tipRc.right = x+w + 3;
		   if (g[i].status & ACTSC_FULLWIDTH) {
			   tipRc.right = 5000;
		   }
		   tipRc.bottom = y+h + 3;
		   //ToolTipX::iTip * tipObj = g[i].getTipObject();
		   //ToolTipX::iTip * tipObj = g[i].getTipObject();
		   if (g[i].hasTipObject()) {
			   if (tipByHwnd) {
					tooltip.addTarget(hWnd, ToolTipX::TargetPtr(
						new ToolTipX::TargetWindow(
							ToolTipX::TipPtr(new TipAction(g[i])), (HWND)g[i].handle)));
			   } else {
					tooltip.addTarget(hWnd, ToolTipX::TargetPtr(
							new ToolTipX::TargetRect(
							ToolTipX::TipPtr(new TipAction(g[i])), tipRc)));
			   }

		   }
/*
		   tooltip.addTarget(hWnd
			   , ToolTipX::TargetPtr(
			   new ToolTipX::TargetWindow(
			   ToolTipX::TipPtr(
			   new ToolTipX::TipOnlyTitle("To jest tip: " + g[i].txt, Ico[IDI_WND_CONFIG].ico_, true, fontB)), (HWND)g[i].handle)));
			   */
/*		   int type;
		   HBITMAP bmp = (HBITMAP) Ico.getImageFromURL("file://doc/img/logo.gif", type);
		   */
/*		   tooltip.addTarget(hWnd
			   , ToolTipX::TargetPtr(
			   new ToolTipX::TargetWindow(
			   ToolTipX::TipPtr(
			   new ToolTipX::TipOnlyImage(bmp)), (HWND)g[i].handle)));
			   */
/*		   tooltip.addTarget(hWnd
			   , ToolTipX::TargetPtr(
			   new ToolTipX::TargetWindow(
			   ToolTipX::TipPtr(
			   ToolTipX::Tip::titleTextAndImage(Ico[IDI_WND_CONFIG].ico_, "To jest tip...", g[i].txt + "</br></br><b>Tak, to jest richtext! <center><i>a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a </i></center></b>",true, bmp, true)), (HWND)g[i].handle)));
*/

	   }

	   if (g[i].status & ACTSC_FULLWIDTH) {
           SetProp((HWND)g[i].handle , "FullWidth" , (void*)1);
           g[i].status &=~ACTSC_INLINE;
       }
	   if (offsetPosition) {
		if (g[i].status & ACTSC_INLINE) {x+= w + ox;}
			else {y+=rh+roy;x=indent;rh=0;}
	   }
	   if (fontSet)
		   SendMessage((HWND)g[i].handle , WM_SETFONT , (WPARAM)(g[i].status & ACTSC_BOLD?fontB:font) , 1);
       SetProp((HWND)g[i].handle , "ACTION" , (void*)&g[i]);
       //if (!g[i].tip.empty()) SetProp((HWND) g[i].handle , "TIP" , (void*)g[i].tip.c_str());
	 }	


     lastType = g[i].status & ACTM_TYPE;
     if (created) {
        if (g[i].status & ACTR_STATUS) g[i].call(ACTN_STATUS , 0 , 0);
     }
   } // for

   if (lastGroupWnd) { // nie zamkniêta grupa
	   closeGroupBox(lastGroupWnd, y);
   }
   g.setParam(AP_MINWIDTH, inttoch(minPageWidth));

   return hWnd;
}

bool cUIAction::hasTipObject(bool useText) {
	return 	this->paramExists(AP_TIP_OBSOLETE) 
		|| this->paramExists(AP_TIP) || this->paramExists(AP_TIPRICH)
		|| (useText && !this->txt.empty());
}

ToolTipX::iTip * cUIAction::getTipObject(bool useText) {
	ToolTipX::iTip * tipObj = 0;
	if (this->paramExists(AP_TIP_OBSOLETE)) {
		tipObj = new ToolTipX::TipText(this->getParam(AP_TIP_OBSOLETE), font);
	} else if (this->paramExists(AP_TIP) || this->paramExists(AP_TIPRICH)) {
		CStdString text = this->getParam(AP_TIP);
		CStdString textRich = this->getParam(AP_TIPRICH);
		int icon = chtoint(this->getParam(AP_TIPICON));
		CStdString iconUrl = this->getParam(AP_TIPICONURL);
		CStdString imageUrl = this->getParam(AP_TIPIMAGEURL);
		CStdString title = this->getParam(AP_TIPTITLE);
		ToolTipX::Tip * tip = new ToolTipX::Tip();
		tipObj = tip;
		if (!title.empty() || icon || !iconUrl.empty()) {	
			HICON hicon = 0;
			if (icon) {
				hicon = (HICON)Ico.iconGet(icon, IML_16);
			} else if (!iconUrl.empty()) {
				int type;
				hicon = (HICON)Ico.getImageFromURL(iconUrl, type, (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , this->owner));	
				if (type != IMAGE_ICON && hicon) {
					DeleteObject(hicon);
					hicon = 0;
				}
			}
			tip->add(ToolTipX::TipPtr(new ToolTipX::TipOnlyTitle(title, hicon, false, fontB)), title.empty() ? ToolTipX::Tip::alignLeft : ToolTipX::Tip::alignAuto);
		}
		if (!text.empty()) {
			tip->add(ToolTipX::TipPtr(new ToolTipX::TipText(text, font, chtoint(this->getParam(AP_TIP_WIDTH)))));
		}
		if (!textRich.empty()) {
			tip->add(ToolTipX::TipPtr(new ToolTipX::TipTextRich(textRich, font, HTMLInfoStyleCB, chtoint(this->getParam(AP_TIPRICH_WIDTH)))));
		}
		if (!imageUrl.empty()) {
			int type;
			HBITMAP bitmap = (HBITMAP)Ico.getImageFromURL(imageUrl, type, (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , this->owner));	
			if (type != IMAGE_BITMAP && bitmap) {
				DestroyIcon((HICON)bitmap);
				bitmap = 0;
			}
			if (bitmap) 
			tip->add(ToolTipX::TipPtr(new ToolTipX::TipOnlyImage(bitmap, false)));
		}
	} else if (useText && !this->txt.empty()) {
		tipObj = new ToolTipX::TipText(this->txt, font);
	}
	return tipObj;
}


void openCfgAction(cUIGroup & g) {
	tooltip.reset();
   sUIACTION sa;
   ZeroMemory(&sa , sizeof(sUIACTION));
   int configProp = (int)GetProp(getCfgHandle(g) , "configProp");
   // Chowamy poprzednika
   if (configProp) {
     SendMessage((HWND)Act[configProp].ghandle , WM_VSCROLL , SB_TOP , 0);
     if (Act[configProp].status & ACTR_SHOW)
       Act[configProp].call(ACTN_HIDE , 0 , 0);
     for (int i=0;i<Act[configProp].size();i++) {
       if (Act[configProp][i].status & ACTR_SHOW)
         Act[configProp][i].call(ACTN_HIDE , 0 , 0);
     }
     ShowWindow((HWND)Act[configProp].ghandle,SW_HIDE);
   }
   configProp=g.id;
   SetProp(getCfgHandle(g) , "configProp" , (void*)configProp);
   SetDlgItemText(getCfgHandle(g) , IDG_GROUP , g.txt.c_str());
   if (!g.ghandle) makeCfgProp(g);
   SetScrollPos((HWND)g.ghandle , SB_VERT , 0 , 1);
     if (Act[configProp].status & ACTR_SHOW)
       Act[configProp].call(ACTN_SHOW , 0 , 0);
     for (int i=0;i<Act[configProp].size();i++) {
       if (Act[configProp][i].status & ACTR_SHOW)
         Act[configProp][i].call(ACTN_SHOW , 0 , 0);
     }
   SendMessage(GetDlgItem(hwndConfig , IDC_TREE) , TVM_SELECTITEM , TVGN_CARET , (LPARAM)Act[g.parent](g.id).handle);
   RECT size, newSize;
   GetWindowRect(getCfgHandle(g), &size);
   size.right -= size.left;
   size.bottom -= size.top;
   size.left = 0;
   size.top = 0;

   resizeProp(getCfgHandle(g));
   newSize = size;
   clipPropSize(g, 0, newSize, 0, 0);
   if (size.right != newSize.right || size.bottom != newSize.bottom) {
	   SetWindowPos((HWND)getCfgHandle(g), 0, 0, 0, newSize.right, newSize.bottom, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
   } else {
   }
   SetWindowPos((HWND)g.ghandle , HWND_TOP , 0 , 0 , 0 , 0 , SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
//   SetFocus();
   SetActiveWindow(GetParent((HWND)g.ghandle));
   //     MessageBox(0 , a->txt.c_str() , "" , 0);
}

void destroyCfgTree(cUIGroup * g) {
	destroyActionsTree(g ? *g : Act[IMIG_CFG]);
}
void destroyActionsTree(cUIGroup & g) {
	for (int i=0;i<g.size();i++) {
		if (g[i].status & ACTS_GROUP) {
			destroyActionsTree(static_cast<cUIGroup &>(g[i]));
			if (((g[i].getType() == ACT_CFG) || (g[i].getType() == ACT_BAR) || (g[i].getType() == ACT_TB)) && g[i].getGroup().ghandle)
				DestroyWindow((HWND)g[i].getGroup().ghandle);
		} else {
			if (g[i].status & ACTR_INIT)
				g[i].call(ACTN_DESTROY , 0 , 0);
			if (g[i].statusType() == ACTT_HWND) {
				sUIActionNotify_destroyWindow andw;
				andw.act = g[i].act();
				andw.hwnd = (HWND)g[i].handle;
				g[i].call(&andw);
			} else {
				// Wszystkie inne, które maj¹ handle...
				if ((g[i].getType() == ACT_CFG) && g[i].handle)
					DestroyWindow((HWND)g[i].handle);
			}
			g[i].handle = 0;
		}
	}
	if (g.status & ACTR_INIT)
		g.call(ACTN_DESTROY , 0 , 0);
	g.ghandle = 0;
	g.handle = 0;
}

const char * getActionValue(cUIAction & act , char * buff, size_t size , bool convert , size_t * newSize) {
	   bool strUsed = false;
	   if (!buff) {size = BIG_STRING; buff = TLS().str.GetBuffer(size+1);strUsed=true;}
       cUIAction_cfg * actc = (cUIAction_cfg *)&act;
       *buff = 0; 
	   if (act.handle) {
         switch (act.status & ACTM_TYPE) {
           case ACTT_COMBO:
               actc->getComboValue(buff , size);
               break;
           case ACTT_TIME:
               actc->getTimeValue(buff , size);
               break;
           case ACTT_RADIO:
               actc->getRadioValue(buff , size);
               break;
           case ACTT_SLIDER:
               actc->getSliderValue(buff , size);
               break;
           case ACTT_CHECK:
             buff[1]=0;
             buff[0]=(SendMessage((HWND)act.handle , BM_GETCHECK , 0,0)==BST_CHECKED?'1':'0');
             break;
		   case ACTT_EDIT: case ACTT_TEXT : case ACTT_PASSWORD: case ACTT_FILE : case ACTT_DIR: case ACTT_COLOR: case ACTT_FONT: case ACTT_SPINNER:
             SendMessage((HWND)act.handle , WM_GETTEXT , size,(LPARAM)buff);
             break;
         }
	   } else { /* skoro nie zosta³a stworzona kontrolka, spróbujemy odczytaæ wartoœæ z tablicy... */
		   getCfgValue(act , buff , size);
	   }
	   if (convert && act.id && (!act.p1 || act.status & ACTR_CONVERT)) {
		   sUIActionNotify_2params n (ACTN_CONVERT_FROM , (int)buff , size);
		   act.call(&n);
		   buff = (char *) n.notify1;
		   size = n.notify2;
	   }
   	   if (newSize) 
		   *newSize = size;
	   if (strUsed) {TLS().str.ReleaseBuffer();}
	   return buff;
}
const char * setActionValue(cUIAction & act , const char * buff , bool convert) {
       if (!buff) {buff = "";}
       cUIAction_cfg * actc = (cUIAction_cfg *)&act;
	   if (convert && act.id && (!act.p1 || act.status & ACTR_CONVERT)) {
		   sUIActionNotify_2params n (ACTN_CONVERT_TO , (int)buff , strlen(buff)+1);
		   act.call(&n);
		   buff = (char*) n.notify1;
	   }
	   if (act.handle) {
         switch (act.status & ACTM_TYPE) {
           case ACTT_COMBO:
               actc->setComboValue(buff);
               break;
           case ACTT_TIME:
               actc->setTimeValue(buff);
               break;
           case ACTT_CHECK:
             SendMessage((HWND)act.handle , BM_SETCHECK , *buff=='1'?BST_CHECKED:BST_UNCHECKED,0);
             break;
           case ACTT_RADIO:
               actc->setRadioValue(buff);
               break;
           case ACTT_SLIDER:
               actc->setSliderValue(buff);
               break;
		   case ACTT_EDIT: case ACTT_TEXT : case ACTT_PASSWORD: case ACTT_FILE : case ACTT_DIR: case ACTT_COLOR: case ACTT_FONT: case ACTT_SPINNER:
               SendMessage((HWND)act.handle , WM_SETTEXT , 0 , (LPARAM)buff);
             break;
         }
	   }
       return buff;
}

void saveCfg(cUIAction * a) {
     cUIGroup & g=Act[a?a->id : IMIG_CFG];
     for (int i=0;i<g.size();i++) {
       if (g[i].status & ACTR_SAVE) g[i].call(ACTN_SAVE , 0 , 0);
       if (g[i].status & ACTS_GROUP) {saveCfg(&g[i]);continue;}
       if (g[i].handle) {
		   size_t size = 0;
		   char * buff = (char *)getActionValue(g[i] , 0 , 0 , false , &size);
           setCfgValue(g[i] , buff , size);
       }
     }
//     IMessageDirect(IM_UIACTION , g.owner , (int)g.fill() , ACTF_DESTROY);
     if (!(g.pparent->type & ACT_CFGBRANCH)) { // Pierwszy w confige'u
         if (g.type & ACT_WNDCFG) {
              IMessage(IM_CFG_CHANGED,NET_BC , IMT_CONFIG);
              IMessageDirect(IM_CFG_CHANGED);
              ICMessage(IMC_SAVE_CFG);
              ICMessage(IMC_SAVE_CNT);
         }
        if (cfgNeedRestart) {
            cfgNeedRestart = 0;
            if (MessageBox((HWND)g.pparent->handle , 
                "Zmiany których dokona³eœ wymagaj¹ restartu programu.\r\nZrestartowaæ teraz?" , "Konnekt" , MB_ICONWARNING | MB_YESNO)==IDYES)
                ICMessage(IMC_RESTART);
        }
     }
}

/*
void SetCfgTip(HWND hwnd , const char * tip , void * handle);
void SetCfgTip(HWND hwnd , const char * tip , void * handle) {
   if (!tip || !*tip) {tip = "";handle = 0;}
   if (GetProp(hwnd , "TIPHANDLE") == handle) return;
   SetWindowText(hwnd , tip);
   SetProp(hwnd , "TIPHANDLE" , handle);
}
*/

//---------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
HWND makeToolBar(HWND hwndOwner , cUIGroup & g) {
     g.setType(ACT_TB);
     // Initialize structure members that both bands will share.
     HWND hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL,
        WS_CHILD | WS_CLIPCHILDREN |WS_CLIPSIBLINGS
        |TBSTYLE_TRANSPARENT
        |CCS_NODIVIDER
//        | CCS_ADJUSTABLE
        | (g.status & ACTSTB_OPAQUE?0:TBSTYLE_FLAT)
        | (g.status & ACTSTB_LIST?TBSTYLE_LIST:0)
//        | (g.pparent->status & ACTS_VERT?TBSTYLE_WRAPABLE:0)
        | CCS_NOPARENTALIGN
        | CCS_NORESIZE
//        | TBSTYLE_WRAPABLE
        /*| TBSTYLE_TOOLTIPS*/
        , 0, 0, 0, 0, hwndOwner,
        (HMENU)g.index, hInst, 0);
     // Get the height of the toolbar.
     g.ghandle=hwndTB;
     SetProp(hwndTB , "ACTION" , (void*)&g);
	 SetProp(hwndTB , "CNTID" , (void*)g.cnt);

     SendMessage(hwndTB , TB_SETEXTENDEDSTYLE , 0 ,  
                       (g.status & ACTSTB_DARROWS?TBSTYLE_EX_DRAWDDARROWS:0)
                       |(g.status & ACTSTB_MIXED?TBSTYLE_EX_MIXEDBUTTONS:0)
                       /*| TBSTYLE_EX_HIDECLIPPEDBUTTONS*/
                       );
     SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
     SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)Ico.iml[0]);
     // Set values unique to the band with the toolbar.
     TBBUTTON * bb = new TBBUTTON [g.size()];
     for (int i=0; i<g.size(); i++) {
       g[i].setType(ACT_TBB);
       g[i].cnt = g.cnt;
       if (g[i].status & ACTR_INIT) g[i].call(ACTN_CREATE , 0 , 0);
       bb[i].iBitmap=Ico[g[i].p1].index[0];
       bb[i].idCommand=g[i].index;
       bb[i].fsState=(~g[i].status & (ACTS_DISABLED|ACTS_HIDDEN)?TBSTATE_ENABLED:0)
                     |(g[i].status & ACTS_HIDDEN?TBSTATE_HIDDEN:0)
                     |(g[i].status & ACTS_CHECKED?TBSTATE_CHECKED:0)
                     |((g[i].status & ACTSTB_WRAP || g.pparent->status & ACTSBAR_VERT)?TBSTATE_WRAP:0);
       bb[i].fsStyle=(g[i].status & ACTSTB_SHOWTEXT?BTNS_SHOWTEXT:0)
                     |(g[i].w?0:BTNS_AUTOSIZE)
                     ;
       switch (g[i].status & (ACTM_TYPE | ACTSTB_WHOLEDROPDOWN | ACTS_GROUP)) {
         case ACTT_CHECK: bb[i].fsStyle|=BTNS_CHECK; break;
         case ACTT_CHECKGROUP: bb[i].fsStyle|=BTNS_CHECKGROUP; break;
         case ACTSTB_WHOLEDROPDOWN: bb[i].fsStyle|=BTNS_WHOLEDROPDOWN;
         case ACTS_GROUP: bb[i].fsStyle|=BTNS_DROPDOWN;break;
         case ACTT_SEPARATOR: bb[i].fsStyle|=TBSTYLE_SEP;break;
         case ACTT_HWND: bb[i].fsStyle|=TBSTYLE_SEP;break;
         default: bb[i].fsStyle|=BTNS_BUTTON; break;
       }
       bb[i].iString=(int)g[i].txt.c_str();
       bb[i].dwData=(long)&g[i];
       #ifdef __DEBUG
       DbgActionsSet(&g[i] , 0);
       #endif
     }
     if (g.size()) SendMessage(hwndTB, TB_ADDBUTTONS, g.size() , (LPARAM)bb);
     for (int i=0; i<g.size(); i++) {
         if (g[i].statusType() == ACTT_HWND) {
            sUIActionNotify_createWindow ancw;
            ancw.act = g[i].act();
            ancw.hwndParent = hwndTB;
            ancw.status = g[i].status;
            ancw.txt = g[i].txt.c_str();
            ancw.font = font;
            ancw.fontBold = fontB;
            RECT rc = {0,0,0,0};
            SendMessage(hwndTB , TB_GETITEMRECT , i , (LPARAM)&rc);
            ancw.x=rc.left;
            ancw.y=rc.top;
            g[i].call(&ancw);
            g[i].w=short(ancw.x - rc.left);
            g[i].h=short(ancw.y - rc.top);
            g[i].handle = ancw.hwnd;

         }
         if (g[i].w)
             SendMessage(hwndTB, TB_SETBUTTONSIZE , 0 , MAKELONG(g[i].w , g[i].h));
     }
     // Add the band that has the toolbar.
     SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);

     delete [] bb;
     return hwndTB;

}

void prepareBandInfo(cUIAction & act , REBARBANDINFO & rbBand) {
    HWND hwnd = (HWND)act.handle;
    if (!(act.status & ACTS_GROUP)) {
        if (act.statusType() == ACTT_HWND) {
            RECT rc = {0,0,0,0};
            GetClientRect(hwnd , &rc);
            rbBand.cyMinChild = act.h;
            rbBand.cyMaxChild = rc.bottom;
            rbBand.cyChild = rc.right;
            rbBand.cxIdeal = rc.right;
            rbBand.cxMinChild = act.w;
            rbBand.cx = rc.right;
        }
     } else {
       //Toolbar
       long dwBtnSize = SendMessage(hwnd, TB_GETBUTTONSIZE, 0,0);
       SIZE sz;
       sz = ToolBar_GetSize(hwnd);
       rbBand.cyMinChild = rbBand.cyMaxChild = sz.cy;
       rbBand.cyChild = sz.cy;
       RECT rc;
       GetClientRect(hwnd , &rc);
       int dwPad = SendMessage( hwnd, TB_GETPADDING, 0, 0 );
       rbBand.cxIdeal = sz.cx;
       rbBand.cxMinChild = (act.status & ACTSBAND_FULL? sz.cx*0:0);
       rbBand.cx = sz.cx+20;
     }

}

HWND makeBar(HWND hwndOwner , cUIGroup & g) {
  g.setType(ACT_BAR);
  REBARINFO     rbi;
  REBARBANDINFO rbBand;
  HWND hwndRB;
  if (g.status & ACTR_INIT) g.call(ACTN_CREATEGROUP , 0 , 0);
  Act.Mod.loadAction(g);
  hwndRB = CreateWindowEx(WS_EX_TOOLWINDOW ,REBARCLASSNAME,NULL,
                           WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN
                           |RBS_VARHEIGHT
                           /*|RBS_FIXEDORDER*/
                           | CCS_NODIVIDER|RBS_DBLCLKTOGGLE
                           | (g.status & ACTSBAR_NOALIGN?CCS_NOPARENTALIGN:0)
                           | (g.status & ACTSBAR_BOTTOM?CCS_BOTTOM:0)
                           | (g.status & ACTSBAR_VERT?CCS_VERT:0)
//                           | CCS_ADJUSTABLE
                           /*| RBS_AUTOSIZE */
                           ,0,0,0,0,
                           hwndOwner,(HMENU)g.index,hInst,NULL);
  SetProp(hwndRB , "ACTION" , (void*)&g);
  SetProp(hwndRB , "CNTID" , (void*)g.cnt);

  SetProp(hwndRB , "oldWNDPROC" , (HANDLE)GetWindowLong(hwndRB , GWL_WNDPROC));
  SetWindowLong(hwndRB , GWL_WNDPROC , (LONG) ReBarControlProc);

  tooltip.addTarget(hwndRB, ToolTipX::TargetPtr(new TargetActionRebar()));

  g.ghandle=hwndRB;
  rbi.cbSize = sizeof(REBARINFO);  // Required when using this struct.
  rbi.fMask  = 0;rbi.himl   = (HIMAGELIST)NULL;if(!SendMessage(hwndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
     return NULL;
  for (int i=0; i < g.size() ; i++) {
     Act.Mod.loadAction(g[i]);
     g[i].setType(ACT_BAND);
     g[i].cnt = g.cnt;
     if (g[i].status & ACTR_INIT) g[i].call(ACTN_CREATE , 0 , 0);
     ZeroMemory(&rbBand , sizeof(REBARBANDINFO));
     rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
     rbBand.fMask  = RBBIM_TEXT | //RBBIM_BACKGROUND |
                  RBBIM_STYLE | RBBIM_CHILD
                  |RBBIM_CHILDSIZE
                  |RBBIM_SIZE
                  |RBBIM_IDEALSIZE
                  |RBBIM_LPARAM
                  |RBBIM_ID
                  ;
     rbBand.fStyle = RBBS_VARIABLEHEIGHT | RBBS_USECHEVRON |
                     (g[i].status & ACTS_HIDDEN?RBBS_HIDDEN:0)
                   | (g[i].status & ACTSTB_WRAP?RBBS_BREAK:0)
                   | (g.status & ACTSBAR_LOCKED?RBBS_NOGRIPPER:0)
                   ;
     rbBand.lpText     = "";//(char *)g[i].txt.c_str();
     rbBand.cxMinChild = 30;
     rbBand.cyMinChild = 18;
     rbBand.cyMaxChild = 0;
     rbBand.cyChild = 18;
     rbBand.cx         = 20;
     rbBand.hwndChild  = 0;
     rbBand.lParam = (long)&g[i];
     rbBand.wID = g[i].index;
     HWND hWnd=0;

     if (!(g[i].status & ACTS_GROUP)) {
        if (g[i].statusType() == ACTT_HWND) {
            sUIActionNotify_createWindow ancw;
            ancw.act = g[i].act();
            ancw.hwndParent = hwndRB;
            ancw.status = g[i].status;
            ancw.txt = g[i].txt.c_str();
            ancw.x=0;
            ancw.y=0;
            ancw.font = font;
            ancw.fontBold = fontB;
            g[i].call(&ancw);
            RECT rc = {0,0,0,0};
            g[i].w = ancw.x;
            g[i].h = ancw.y;
            hWnd = ancw.hwnd;
        } else continue; 
     } else {
       //Toolbar
       hWnd = makeToolBar(hwndRB , g[i].getGroup());
     }

     rbBand.hwndChild  = hWnd;
     g[i].handle=hWnd;
     prepareBandInfo(g[i] , rbBand);
     // Korzystamy z MODow!
     int row = Act.Mod.getEntry(g[i]);
     if (Act.Mod.DT->getint(row , AMOD_MASK) & MODM_WH)
         rbBand.cx = LOWORD(Act.Mod.DT->getint(row , AMOD_WH));

     SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
     #ifdef __DEBUG
     DbgActionsSet(&g[i] , 0);
     #endif
  }
  Act.Mod.loadBar(g);
  return hwndRB;
}


// --------------------------------------------
//---------------------------------------------
//void ActionText(sUIACTION & a, char * txt) {
//}

int ActionSet(sUIActionInfo & ai) {
  if (!Act.exists(ai.act)) return 0;
  Act(ai.act).setInfo(ai , true);
  return 1;
}

int ActionStatus(sUIAction & act , int status, char * text , int ico) {
  sUIActionInfo si;
  memset(&si , 0 , sizeof(si));
  si.act = act;
  if (status != -1) {si.status = status; si.statusMask=ACTM_STATE; si.mask |= UIAIM_STATUS;}
  if (text) {si.txt = text; si.mask |= UIAIM_TXT;}
  if (ico != -1) {si.p1 = ico; si.mask |= UIAIM_ICO;}
  return ActionSet(si);
}

void cUIAction::addParams(const string & add) {
	size_t pos = 0;
	CStdString name = "";
	CStdString value = "";
	while (pos != add.npos) {
		// szukamy pocz¹tku nazwy
		while (!ParamSpecialChar(add[pos]) && pos < add.length()) {pos++;}
		if (pos >= add.length()) break;
		if (add[pos] != EXT_PARAM_CHAR) {
			name = add[pos]; // koniec szukania...
			pos++;
		} else {
			size_t end = add.find("=", pos);
			if (end == add.npos) {
				pos ++;
				continue;
			}// jakaœ zjebka...
			name = add.substr(pos, end - pos + 1);
			pos = end+1;
		}
		size_t end = pos;
		while (!ParamSpecialChar(add[end]) && end < add.length()) {end++;}
		if (end == add.length()) end = add.npos;
		this->setParam(name, add.substr(pos, end == add.npos ? end : end-pos));
	}
}


void cUIAction::setInfo(sUIActionInfo & ai, bool update) {
  critical.action.enter();
  bool leaved = false;
  this->setCnt(ai.act.cnt);
  int oldStatus = status;

  if (ai.mask & UIAIM_STATUS) {this->status &=~ai.statusMask; this->status|=ai.status&ai.statusMask;ai.status = this->status;}
  if (ai.mask & UIAIM_TXT && ai.txt) {
	  if (this->statusType() == ACTT_COMBO && !strchr(ai.txt, *AP_PARAMS)) {
		  this->txt = ai.txt; /*HACK: ustawiamy txt specjalnie dla combo...*/
	  } else {
		  this->txt = GetActParam(ai.txt, "");
		  this->addParams(ai.txt);
	  }
  }
  if (ai.mask & UIAIM_P1) {this->p1 = ai.p1;
    }
  if (ai.mask & UIAIM_P2) {this->p2 = ai.p2;}
  if (ai.mask & UIAIM_PARAM) {this->userParam = ai.param;}
  if (ai.mask & UIAIM_EXSTYLE) {this->exstyle = ai.exStyle;}
  if (ai.mask & UIAIM_SIZE) {this->w = ai.w; this->h = ai.h;}

  #ifdef __DEBUG
  DbgActionsSet(this , 0);
  #endif

  if (!update) goto leave;

  switch (this->getType()) {
     case ACT_TBB: {
        TBBUTTONINFO bi;
        bi.cbSize = sizeof(bi);
        bi.dwMask = 0;
        if (ai.mask & UIAIM_ICO) {
            bi.dwMask |= TBIF_IMAGE;
            bi.iImage = ai.p1?Ico[ai.p1].index[0] : I_IMAGENONE;
        }
        if (ai.mask & UIAIM_TXT) {
            bi.dwMask |= TBIF_TEXT;
            bi.pszText = ai.txt;
        }         
        if (ai.mask & UIAIM_STATUS) {
            bi.dwMask |= TBIF_STATE | TBIF_STYLE;
            bi.fsState=(ai.status & (ACTS_DISABLED|ACTS_HIDDEN)?TBSTATE_INDETERMINATE:TBSTATE_ENABLED)
                    |(ai.status & ACTS_HIDDEN?TBSTATE_HIDDEN:0)
                    |(ai.status & ACTS_CHECKED?TBSTATE_CHECKED:0)
                    |(ai.status & ACTSTB_WRAP?TBSTATE_WRAP:0);
            bi.fsStyle=(ai.status & ACTSTB_SHOWTEXT?BTNS_SHOWTEXT:0)
                    |BTNS_AUTOSIZE
                    |(((ai.status & ACTM_TYPE) == ACTT_SEPARATOR)?TBSTYLE_SEP:0);
            switch (ai.status & (ACTM_TYPE | ACTSTB_WHOLEDROPDOWN | ACTS_GROUP)) {
            case ACTT_CHECK: bi.fsStyle|=BTNS_CHECK; break;
            case ACTT_CHECKGROUP: bi.fsStyle|=BTNS_CHECKGROUP; break;
            case ACTSTB_WHOLEDROPDOWN: bi.fsStyle|=BTNS_WHOLEDROPDOWN;break;
            case ACTS_GROUP: bi.fsStyle|=BTNS_DROPDOWN;break;
            default: bi.fsStyle|=BTNS_BUTTON; break;
            }
        }
        critical.action.leave();
        leaved = true;
        SendMessage((HWND)this->pparent->ghandle , TB_SETBUTTONINFO , this->index , (LPARAM)&bi);
        SendMessage((HWND)this->pparent->ghandle , TB_AUTOSIZE , 0 , 0);
//          RedrawWindow((HWND)Act[act.parent].handle,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);
//        if (ai.mask & UIAIM_ICO) {
//           SendMessage((HWND)Act[act.parent].handle , TB_CHANGEBITMAP , act.id , MAKELPARAM(ai.ico?Ico[ai.ico].index : I_IMAGENONE , 0));
//        }

        break;}
  }
  if (ai.mask & UIAIM_STATUS && this->status & ACTR_STATUS) this->call(ACTN_STATUS , this->status , oldStatus);
  
  
  leave:
  if (!leaved) critical.action.leave();
  
}


void cUIGroup::setInfo(sUIActionInfo & ai, bool update) {
  int oldStatus = status;
  critical.action.enter();
  cUIAction::setInfo(ai , update);
  if (!update) goto leave;
  switch (this->getType()) {
      case ACT_BAR: {
          if (ai.mask & UIAIM_STATUS && ai.statusMask & ACTSBAR_LOCKED) {
              SendMessage(GetParent((HWND)this->ghandle) , WM_SETREDRAW , FALSE , 0);
              SendMessage((HWND)this->ghandle , WM_SETREDRAW , FALSE , 0);
              int c = SendMessage((HWND)this->ghandle , RB_GETBANDCOUNT , 0 , 0);
              REBARBANDINFO rbi;  
              rbi.cbSize = sizeof(rbi);
              rbi.fMask = RBBIM_STYLE;
              for (int i=0; i<c; i++) {
                  SendMessage((HWND)this->ghandle , RB_GETBANDINFO , i , (LPARAM)&rbi);
                  rbi.fStyle &= ~RBBS_NOGRIPPER;
                  if (status & ACTSBAR_LOCKED) rbi.fStyle|=RBBS_NOGRIPPER;
                  SendMessage((HWND)this->ghandle , RB_SETBANDINFO , i , (LPARAM)&rbi);
                  if (!(rbi.fStyle & RBBS_HIDDEN)) {
                      SendMessage((HWND)this->ghandle , RB_SHOWBAND , i , FALSE);
                      SendMessage((HWND)this->ghandle , RB_SHOWBAND , i , TRUE);
                  }
              }



              SendMessage((HWND)this->ghandle , WM_SETREDRAW , TRUE , 0);
              SendMessage(GetParent((HWND)this->ghandle) , WM_SETREDRAW , TRUE , 0);
              RepaintWindow((HWND)this->ghandle);
              

          }
          break;}
      case ACT_TB: {
          if (ai.mask & UIAIM_STATUS && ai.statusMask & ACTSTB_LIST) {
              SendMessage((HWND)ghandle , TB_SETSTYLE  , 0 , (SendMessage((HWND)ghandle , TB_GETSTYLE , 0 , 0) & ~TBSTYLE_LIST) | (ai.status & ACTSTB_LIST?TBSTYLE_LIST : 0));
          }
          if (ai.mask & UIAIM_STATUS && ai.statusMask & (ACTSTB_DARROWS | ACTSTB_MIXED)) {
              SendMessage((HWND)ghandle , TB_SETEXTENDEDSTYLE , 0 , 
                  SendMessage((HWND)ghandle , TB_GETEXTENDEDSTYLE , 0 , 0) & ~((ai.statusMask & ACTSTB_DARROWS?TBSTYLE_EX_DRAWDDARROWS:0)|(ai.statusMask & ACTSTB_MIXED?TBSTYLE_EX_MIXEDBUTTONS:0))
                  | (ai.statusMask & ACTSTB_DARROWS?ai.status & ACTSTB_DARROWS?TBSTYLE_EX_DRAWDDARROWS:0:0)
                  | (ai.statusMask & ACTSTB_MIXED?ai.status & ACTSTB_MIXED?TBSTYLE_EX_MIXEDBUTTONS:0:0)
                  );
          }
		  SendMessage((HWND)ghandle, TB_AUTOSIZE, 0, 0);
  		  int iTextRows = (int)::SendMessage((HWND)ghandle, TB_GETTEXTROWS, 0, 0);
		  SendMessage((HWND)ghandle, WM_SETREDRAW, FALSE, 0);
		  SendMessage((HWND)ghandle, TB_SETMAXTEXTROWS, iTextRows+1, 0);
		  SendMessage((HWND)ghandle, TB_SETMAXTEXTROWS, iTextRows, 0);
		  SendMessage((HWND)ghandle, WM_SETREDRAW, TRUE, 0);
          // Trzeba jeszcze jakos zmienic rozmiar Bandy...
          REBARBANDINFO rbbi;
          rbbi.cbSize = sizeof(rbbi);
          rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_IDEALSIZE;
          SendMessage(GetParent((HWND)ghandle) , RB_GETBANDINFO , SendMessage(GetParent((HWND)ghandle) , RB_IDTOINDEX , this->index , 0) , (LPARAM)&rbbi);
          prepareBandInfo(*this , rbbi);
          SendMessage(GetParent((HWND)ghandle) , RB_SETBANDINFO , SendMessage(GetParent((HWND)ghandle) , RB_IDTOINDEX , this->index , 0) , (LPARAM)&rbbi);

//          RepaintWindow((HWND)ghandle);

          break;}
  }

  leave:
  critical.action.leave();
}


void cUIAction_cfg::setInfo(sUIActionInfo & ai, bool update) {
  critical.action.enter();
  int oldStatus = status;
  cUIAction::setInfo(ai , false);
  if (ai.s_size > sizeof(sUIActionInfo)) {
    sUIActionInfo_cfg * aic = (sUIActionInfo_cfg*)&ai; 
    if (ai.mask & UIAIM_POS) {this->x = aic->x; this->y=aic->y;}
  }  
  if (!update) goto leave;
  // Sprawdzam czy nie nalozyc statusu na cala grupe
  if ((this->id & ~IMIB_) && (ai.mask & UIAIM_STATUS) && (ai.statusMask & ACTS_DISABLED) && ((this->status & ACTM_TYPE)==ACTT_GROUP)) {
     cUIGroup & g = *this->pparent;
     int pos = g.find(this->id) + 1;
     sUIActionInfo setAi;
     setAi.mask = UIAIM_STATUS;
     setAi.status = ai.status;
     setAi.statusMask = ACTS_DISABLED;
     while (pos < g.size() && ((g[pos].status & ACTM_TYPE) != ACTT_GROUPEND)) {
         setAi.act=g[pos].act();
         g[pos].setInfo(setAi , true);
         pos++;
     }
  }


  {
    if (!this->handle || (this->status & ACTM_TYPE) == ACTT_HOLDER || this->status & ACTS_GROUP) goto leave;
	if (ai.mask & UIAIM_STATUS) {
		HWND set = (HWND)this->handle;
		while (set) {
			EnableWindow(set , !(ai.status & ACTS_DISABLED));
			set = (HWND)GetProp(set, "connectedWnd");
			if (set == (HWND)this->handle) break; // pêtla?
		}
	}

/*	switch (this->status & ACTM_TYPE) {
		case ACTT_SPINNER:
			this->setSpinnerInfo(&ai);
			break;
	}*/

	if (ai.mask & UIAIM_TXT) {
        switch (this->status & ACTM_TYPE) {
            case ACTT_COMMENT: case ACTT_TEXT: case ACTT_INFO:
				SetWindowText((HWND)this->handle , this->txt.c_str());break;
			case ACTT_HTMLINFO:
				cRichEditHtml::SetHTML((HWND)this->handle , this->txt.c_str() , HTMLInfoStyleCB);
				break;
            case ACTT_COMBO:
                this->setComboItems(this->txt.c_str());break;
            case ACTT_RADIO:
				this->setRadio(ai.txt, true);break;
            case ACTT_SLIDER:
				this->setSlider(ai.txt, true);break;
            case ACTT_IMAGE:
                this->setImage(ai.txt, true);break;
			case ACTT_TIPBUTTON:
            case ACTT_BUTTON:
            case ACTT_LINK:
                this->setButton(ai.txt, true);break;
        }
    }
  }
  if (ai.mask & UIAIM_STATUS && this->status & ACTR_STATUS) this->call(ACTN_STATUS , this->status , oldStatus);
  leave:
  critical.action.leave();
}


void cUIAction_cfg::setComboItems(const char * _txt) {
    CStdString bck;
	const char * txt = this->txt.c_str();
	/*HACK: Inne ustawianie elementów w combo*/
    getComboValue(bck.GetBuffer(MAX_STRING),MAX_STRING);
    bck.ReleaseBuffer();
//    if (this->status & ACTSCOMBO_LIST) {
      SendMessage((HWND)handle , CB_RESETCONTENT , 0 , 0);
/*    } else {
       int c = SendMessage((HWND)handle , CB_GETCOUNT , 0 , 0);
       for (int i=c-1; i>=0; i--)
           SendMessage((HWND)handle , CB_DELETESTRING , i , 0);
    }*/
    char * buff = strdup(txt);
    char * ch = buff;
    COMBOBOXEXITEM cbi;
    memset(&cbi , 0 , sizeof(cbi));

    while (ch && *ch) {

        char * next = strchr(ch , '\n');
        if (next) *next=0;
		char * value = strchr(ch , '\2'); // CFGVALUE
        if (value) {*value = 0;value++;}
        char * ico = strchr(ch , '\3');  // CFGICO
        if (ico) {*ico = 0;ico++;}
        else ico="0";
        cbi.iItem = -1;
        cbi.mask = CBEIF_LPARAM|CBEIF_TEXT|(ico?(CBEIF_IMAGE|CBEIF_SELECTEDIMAGE):0);
        if (ico) {
          cbi.iImage = Ico[chtoint(ico)].index[0];
          cbi.iSelectedImage = Ico[chtoint(ico)].index[0];
        }
        cbi.lParam = (LPARAM)(value?txt+(value-buff):txt+(ch-buff));
        cbi.pszText = ch;
//        if (this->status & ACTSCOMBO_LIST) {
          SendMessage((HWND)handle , CBEM_INSERTITEM , 0 , (LPARAM)&cbi);
//        } else {
//          SendMessage((HWND)handle , CB_ADDSTRING , 0 , (LPARAM)ch);
//        }
        ch = next;
        if (next) ch++;
    }
    free(buff);
    setComboValue(bck.c_str());

//    if (this->status & ACTSCOMBO_LIST) {
//        SendMessage((HWND)handle , CB_SETCURSEL , 0 , 0);
//    } else {
//    }

}
void cUIAction_cfg::setComboValue(const char * val) {
    if (!val) return;
    if (this->status & ACTSCOMBO_LIST) {
        if (this->status & ACTSCOMBO_BYPOS) {
            SendMessage((HWND)this->handle , CB_SETCURSEL , atoi(val) , 0);
        } else {  // Po ID
            COMBOBOXEXITEM cbi;
            memset(&cbi , 0 , sizeof(cbi));
            cbi.mask = CBEIF_LPARAM;
            int c = SendMessage((HWND)this->handle , CB_GETCOUNT , 0 , 0);
            for (int i = 0; i<c; i++) {
                cbi.iItem = i;
                SendMessage((HWND)this->handle , CBEM_GETITEM , 0 , (LPARAM)&cbi);
                int length = strcspn((char*)cbi.lParam,"\n\1\2\3");
                if (cbi.lParam && length==strlen(val) && !strncmp((char*)cbi.lParam,val,length)) {SendMessage((HWND)this->handle , CB_SETCURSEL , i , 0);break;}
            }
        }
    } else {
        SetWindowText((HWND)handle , val);
    }
}
void cUIAction_cfg::getComboValue(char * val , size_t size) {
    if (!val) return;
    if (this->status & ACTSCOMBO_LIST) {
        if (this->status & ACTSCOMBO_BYPOS) {
            itoa(SendMessage((HWND)this->handle , CB_GETCURSEL , 0 , 0) , val , 10);
        } else {  // Po ID
            COMBOBOXEXITEM cbi;
            memset(&cbi , 0 , sizeof(cbi));
            cbi.mask = CBEIF_LPARAM;
            cbi.iItem = SendMessage((HWND)this->handle , CB_GETCURSEL , 0 , 0);
            SendMessage((HWND)this->handle , CBEM_GETITEM , 0 , (LPARAM)&cbi);
            if (cbi.lParam) {
                int i=0; char * ch1 = val; char * ch2 = (char*)cbi.lParam;
                while (((size_t)ch1<(size_t)val+size-1) && *ch2 && *ch2!='\n' && (char)*ch2>5) {
                    *ch1 = *ch2;
                    ch1++; ch2++;
                }
                *ch1=0;
            }

        }
    } else {
        GetWindowText((HWND)handle , val , size);
    }
}


void cUIAction_cfg::setTimeValue(const char * val) {
    if (!val) return;
    cDate64 time;
    switch (val[0]) {
        case 'Y': time.year = atoi(val+1);time.day=1;time.month=1; break;
        default:time = _atoi64(val);
    }
    SYSTEMTIME st = (SYSTEMTIME)time;
    DateTime_SetSystemtime((HWND)this->handle , time.empty()?GDT_NONE:GDT_VALID , &st);
}
void cUIAction_cfg::getTimeValue(char * val , size_t size) {
    if (!val) return;
    SYSTEMTIME st;
    if (DateTime_GetSystemtime((HWND)this->handle , &st)==GDT_NONE) {
        strcpy(val , "0");return;
    } else {
        cTime64 time(st);
        _i64toa((__int64)time , val , 10);
    }

}

void cUIAction_cfg::setImage(const char * txt, bool update) {
    int imageType;
    cGarbageCollection * GarbageCfg = Garbage.findCollection(getCfgHandle(*(this->pparent)));
	sUIIconRegister::tIconParams ip;
	ip.bits = 0;
	ip.size = this->w == this->h?this->w:0;
	HANDLE imgHandle = cUIIcons::getImageFromURL(this->txt , imageType , (HINSTANCE)ICMessage(IMC_PLUGID_HANDLE , this->owner) ,  &ip);
    SetWindowLong((HWND)this->handle , GWL_STYLE , (GetWindowLong((HWND)this->handle,GWL_STYLE)&(~(SS_ICON|SS_BITMAP)))|(imageType==IMAGE_ICON?SS_ICON:SS_BITMAP));
    if (imgHandle) {
		SIZE sz = GetImageSize(imgHandle, imageType);
		if (!this->w)
			this->w = sz.cx;
		if (!this->h)
			this->h = sz.cy;

        GarbageCfg->remove((void*)SendMessage((HWND)this->handle , STM_SETIMAGE , imageType , (LPARAM)imgHandle));
		if (imageType == IMAGE_ICON)
			GarbageCfg->add(new cGarbage_DestroyIcon(imgHandle));
		else
			GarbageCfg->add(new cGarbage_DeleteObject(imgHandle));
    }

}

void cUIAction_cfg::setRadio(const char * txt, bool update) {
	CStdString title = this->txt;
	SetWindowText((HWND)this->handle, title);
	if (!this->paramExists(AP_RADIOGRP)) {
		CStdString group = this->getParam(AP_RADIOGRP);
		if (group.empty()) {
			group = itoa(this->p1, group.GetBuffer(10) , 10);
			group.ReleaseBuffer();
		}
		this->setParam(AP_RADIOGRP, group);
	}
}
void cUIAction_cfg::setRadioValue(const char * txt) {
	CStdString group = this->getParam(AP_RADIOGRP);
	if (!this->pparent) return;
	int i = 0;
	// przegl¹damy sobie s¹siadów w poszukiwaniu przycisków "RADIO" które mo¿emy ustawiæ...
	for (cUIGroup::tAct::iterator it = this->pparent->Act.begin(); it != this->pparent->Act.end(); ++it) {
		if ((*it)->statusType() != ACTT_RADIO || (*it)->getParam(AP_RADIOGRP) != group) continue;
		bool check = false;
		if (!txt) {
			check = (*it == this);
		} else if ((*it)->status & ACTSRADIO_BYPOS) {
			char pos [4];
			itoa(i, pos, 10);
			check = strcmp(txt, pos) == 0;
		} else {
			check = (*it)->getParam(AP_VALUE) == txt;
		}
		SendMessage((HWND)(*it)->handle, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED , 0);
		i++;
		if ((*it)->status & ACTSRADIO_LAST) {
			break;
		}
	}
}
void cUIAction_cfg::getRadioValue(char * txt , size_t size) {
	CStdString group = this->getParam(AP_RADIOGRP);
	if (!this->pparent) return;
	int i = 0;
	// przegl¹damy sobie s¹siadów w poszukiwaniu przycisków "RADIO" które mo¿emy ustawiæ...
	for (cUIGroup::tAct::iterator it = this->pparent->Act.begin(); it != this->pparent->Act.end(); ++it) {
		if ((*it)->statusType() != ACTT_RADIO || (*it)->getParam(AP_RADIOGRP) != group) continue;
		if (SendMessage((HWND)(*it)->handle, BM_GETCHECK, 0 , 0) == BST_CHECKED) {
			if ((*it)->status & ACTSRADIO_BYPOS) {
				itoa(i, txt, 10);
				return;
			} else {
				strncpy(txt, (*it)->getParam(AP_VALUE), size);
				return;
			}
		}
		i++;
		if ((*it)->status & ACTSRADIO_LAST) {
			break;
		}
	}
	txt[0] = 0;
}

void cUIAction_cfg::setSlider(const char * txt, bool update) {
	if ((!update || ActParamExists(txt, AP_MINIMUM)) && this->paramExists(AP_MINIMUM)) {
		SendMessage((HWND)this->handle, TBM_SETRANGEMIN, 1, atoi(this->getParam(AP_MINIMUM).c_str())); 
	}
	if ((!update || ActParamExists(txt, AP_MAXIMUM)) && this->paramExists(AP_MAXIMUM)) {
		SendMessage((HWND)this->handle, TBM_SETRANGEMAX, 1, atoi(this->getParam(AP_MAXIMUM).c_str())); 
	}
	if ((!update || ActParamExists(txt, AP_STEP)) && this->paramExists(AP_STEP)) {
		SendMessage((HWND)this->handle, TBM_SETTICFREQ,  atoi(this->getParam(AP_STEP).c_str()), 0); 
	}
	if ((!update || ActParamExists(txt, "")) && GetProp((HWND)this->handle, "connectedWnd") && !this->txt.empty()) {
		CStdString left, right;
		left = this->txt;
		if (left.find('\n') != left.npos) {
			right = left.substr(left.find('\n')+1);
			left.erase(left.find('\n'));
		}
		HWND buddy = (HWND)GetProp((HWND)this->handle, "connectedWnd");
		SetWindowText(buddy, " " + left);
		SetWindowText((HWND)GetProp(buddy, "connectedWnd"), right + " ");
	}
}
void cUIAction_cfg::setSliderValue(const char * txt) {
	SendMessage((HWND)this->handle, TBM_SETPOS, 1, atoi(txt)); 
}
void cUIAction_cfg::getSliderValue(char * txt , size_t size) {
	itoa(SendMessage((HWND)this->handle, TBM_GETPOS, 0, 0),txt , 10); 
}

void cUIAction_cfg::setSpinner(const char * txt, bool update) {
	HWND spinner = (HWND)GetProp((HWND)this->handle, "connectedWnd");
	int max, min;
	SendMessage(spinner, UDM_GETRANGE32, (WPARAM)&min, (LPARAM)&max); 
	if ((!update || ActParamExists(txt, AP_MINIMUM)) && this->paramExists(AP_MINIMUM)) {
		min = atoi(this->getParam(AP_MINIMUM).c_str());
	}
	if ((!update || ActParamExists(txt, AP_MAXIMUM)) && this->paramExists(AP_MAXIMUM)) {
		max = atoi(this->getParam(AP_MAXIMUM).c_str());
	}
	if (!update || ActParamExists(txt, AP_MINIMUM) || ActParamExists(txt, AP_MAXIMUM)) {
		SendMessage(spinner, UDM_SETRANGE32, min, max); 
	}
}


void cUIAction_cfg::setButton(const char * txt, bool update) {
	CStdString title = this->txt;
	SetWindowText((HWND)this->handle, title);
	if ((!update || ActParamExists(txt, AP_ICO)) && this->paramExists(AP_ICO)) {
		int ico = atoi(this->getParam(AP_ICO).c_str());
		IML_enum il = this->getParam(AP_ICONSIZE) == "32" ? IML_32 : IML_16;
		if (Ico.iconExists(ico, il)) {
			ButtonX::fromHWND((HWND)this->handle)->setIcon(Ico.getImageList(il), Ico.getIconIndex(ico, il));
		}
	}
	if (this->paramExists(AP_IMGURL) && (!update || ActParamExists(txt, AP_IMGURL))) {
		int type = 0;
 		sUIIconRegister::tIconParams ip;
		ip.bits = 0; ip.size = 0;
		if (this->paramExists(AP_ICONSIZE)) 
			ip.size = atoi(this->getParam(AP_ICONSIZE).c_str());
		if (this->paramExists(AP_ICONDEPTH)) 
			ip.bits = atoi(this->getParam(AP_ICONDEPTH).c_str());
		
		HANDLE img = Ico.getImageFromURL(this->getParam(AP_IMGURL), type, (HINSTANCE)ICMessage(IMC_PLUG_HANDLE, this->owner), &ip);
		if (type == IMAGE_ICON) {
			ButtonX::fromHWND((HWND)this->handle)->setIcon((HICON)img);
		} else {
			HICON icon = makeIconFromBitmap((HBITMAP)img);
			ButtonX::fromHWND((HWND)this->handle)->setIcon(icon);
			DestroyIcon(icon);
		}
	}
	if (this->statusType() == ACTT_TIPBUTTON && this->handle && tooltip.getCurrentTarget().get()
		&& (ActParamExists(txt, AP_TIP) || ActParamExists(txt, AP_TIPTITLE) || ActParamExists(txt, AP_TIPIMAGEURL))) 
	{
		TargetActionTipButton tab(ToolTipX::TipPtr(), (HWND)this->handle);
		if (tooltip.isCurrentTarget(&tab)) {
			static_cast<ToolTipX::Target*>(tooltip.getCurrentTarget().get())->setTip(&tooltip, ToolTipX::TipPtr(this->getTipObject()));
		}
	}
}




//----------------------------------------------

 void makeActions(){
//   hmenuMain = makeActionsMenu(*Act[IMIG_UIMAIN].getId(IMIG_MAIN));
//   hmenuCnt  = makeActionsMenu(*Act[0].getId(IMIG_CNT));
//   hmenuTray = makeActionsMenu(*Act[0].getId(IMIG_TRAY));
   hwndBar = makeBar(hwndMain , Act[IMIG_BAR]);
   // MAIN menu
 }

 void makeCfg(){
   Act[IMIG_CFG].handle=0;
   Act[IMIG_CFG].cnt=0;
   makeCfgTree(Act[IMIG_CFG] , hwndConfigTree);
 }


 void destroyActions(){
//   DestroyMenu(hmenuMain);
//   DestroyMenu(hmenuCnt);
//   DestroyMenu(hmenuTray);
 }

