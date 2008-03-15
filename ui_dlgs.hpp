

#define __GETCNT(n) SetDlgItemText(hwndDlg , IDE_##n , GETCNTC(pos,CNT_##n));
#define __SETCNT(n) GetDlgItemText(hwndDlg , IDE_##n , TLS().buff , MAX_STRBUFF);SETCNTC(pos,CNT_##n,TLS().buff)

void InfoDialogLoad(HWND hwndDlg , int pos) {
}

string InfoGetDisplay(int cntID) {
    sUIAction act = sUIAction(IMIG_NFO_DETAILS , CNT_DISPLAY|IMIB_CNT , cntID);
    String s = getActionValue(Act(act)) + "\n";

	CStdString format = GETSTR(CFG_UICNT_CREATEDISPLAY_FORMAT);
	if (format.empty()) {// standardowe podejœcie
		act.id = IMIB_CNT|CNT_NICK;
		String nick = getActionValue(Act(act));
		if (nick.empty() == false) s += nick + "\n";
		act.id = IMIB_CNT|CNT_NAME;
		String name = getActionValue(Act(act));
		act.id = IMIB_CNT|CNT_SURNAME;
		String surname = getActionValue(Act(act));
		if (name.empty() != false || surname.empty() != false) {
			if (surname.empty() != false) s += surname;
			if (name.empty() != false && surname.empty() != false) s+=" ";
			if (name.empty() != false) s+=name;
			s+="\n";
		}
		act.id = IMIB_CNT|CNT_UID;
		String uid = getActionValue(Act(act));
		if (!uid.empty()) s += uid + "\n";
	} else {
		s += formatTitle(format, cntID, FT_DISPLAYPROPOSAL) + "\n";
		cPreg preg(false);
		s.replace("\r","");
		s = preg.replace("/\\n{2,}/","\n", s.a_str());
	}
    return s;
}

void InfoDialogSave(int cntID , bool upload) {
    Act[IMIG_NFO].setCnt(cntID , true);
    string display = InfoGetDisplay(cntID);
    if (display[0]=='\n') { // Nie ma display!
        if (display.size()<3) {// w ogóle nie ma display!!
            if (Cnt[cntID].user) display = "Ja";
            else {MessageBox(Cnt[cntID].hwndInfo , "Musisz podaæ przynajmniej \"Wyœwietl jako...\"..."
                                   , "Kontakt" , MB_ICONWARNING);
                return;}
        } else { // Bierzemy pierwszy
            display = display.substr(display.find('\n')+1);
            display = display.substr(0,display.find('\n'));
        }
        setActionValue(Act(sUIAction(IMIG_NFO_DETAILS , CNT_DISPLAY|IMIB_CNT , cntID)) , display);
    }
    bool fill = false;
    CStdString oldUID = GETCNTC(cntID , CNT_UID);
    int oldNet = GETCNTI(cntID , CNT_NET);
    CStdString oldGroup = GETCNTC(cntID , CNT_GROUP);
    saveCfg(&Act[IMIG_NFO]);
    // sprawdza czy sie cos zmienilo
    bool changedNet = GETCNTI(cntID , CNT_NET) != oldNet;
	bool changedUid = GETCNTC(cntID , CNT_UID) != oldUID;
	bool changedGroup = GETCNTC(cntID , CNT_GROUP) != oldGroup;
    if (1 /*zakladamy ze zawsze sie cos zmienia*/ || changedNet || changedUid || changedGroup || GETCNTI(cntID , CNT_INTERNAL)&CNT_INTERNAL_adding) {
        sIMessage_CntChanged icc(IMC_CNT_CHANGED , cntID);
		icc.net = Net::core;
		icc.type = imtCore;
        if (changedGroup) {
			icc._changed.group = 1;
		}
        if (changedNet) {
			icc._changed.net = 1;
            icc._oldNet = oldNet;
        }
        if (changedUid) {
			icc._changed.uid = 1;
			icc._oldUID = oldUID;
        }
        IMessage(&icc);
    }
    if (!GETCNTI(cntID , CNT_NET)) SETCNTC(cntID , CNT_UID , "");
	if (changedGroup) fill = true;
	if (upload) {
		EnableWindow(Cnt[cntID].hwndInfo , false);
		IMessage(IM_CNT_UPLOAD,Net::broadcast,imtContact,cntID,0);
		EnableWindow(Cnt[cntID].hwndInfo , true);
	}
    ICMessage(IMI_REFRESH_CNT ,cntID);
//    checkinList(cntID);
    if (fill) {
        cntList.refill();
    }
    ICMessage(IMC_SAVE_CNT);
    ICMessage(IMC_SAVE_MSG);

	SetProp(Cnt[cntID].hwndInfo , "NEWContact" , 0);
//    DestroyWindow(Cnt[cntID].hwndInfo);
    PostMessage(Cnt[cntID].hwndInfo , WM_CLOSE , 0 , 0);
}
void InfoDialogCancel(int cntID , HWND hwnd) {
    if (!hwnd) {hwnd = Cnt[cntID].hwndInfo;}
    if (!hwnd) return;
    if (GetProp(hwnd , "NEWContact")) {
        SetProp(hwnd , "CNT" , 0);
		DestroyWindow(hwnd);
        ICMessage(IMC_CNT_REMOVE , cntID);
        ICMessage(IMI_REFRESH_LST);
    } else {
        DestroyWindow(hwnd);
    }
}

void InfoDialogRefresh(int cntID) {
    HWND hwnd = Cnt[cntID].hwndInfo;
    int tNet = GETCNTI(cntID , CNT_NET);
    string tUID = GETCNTC(cntID , CNT_UID);
    EnableWindow(hwnd , false);
//    SetWindow
    IMessage(IM_CNT_DOWNLOAD,Net::first,imtContact,cntID,1);
	IMLOG("Refreshed");
    EnableWindow(hwnd , true);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SETCNTI(cntID , CNT_NET , tNet);
    SETCNTC(cntID , CNT_UID , tUID.c_str());
    InfoDialogSummary(cntID , true);
}

void NetFillIn(HWND item , int type , int sel = -1) {
    SendMessage(item , CBEM_SETIMAGELIST , 0 , (LPARAM)Ico.iml[0]);
    COMBOBOXEXITEM cbi;
    cbi.mask = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
    cbi.iItem = -1;
    int c = IMessage(IMC_PLUG_COUNT);
    for (int i=0;i<c;i++) {
		oPlugin plugin = Ctrl->getPlugin((tPluginId) i);
            
		if (plugin->getType() & type) {
			String name = plugin->getNetName();
			cbi.pszText = (char*)name.a_str();
			cbi.lParam = plugin->getNet();
            cbi.iImage = cbi.iSelectedImage = Ico[UIIcon(2, cbi.lParam ,0,0)].index[0];
            int pos = SendMessage(item , CBEM_INSERTITEM , 0 , (LPARAM)&cbi);
            if (sel>=0 && sel==cbi.lParam) {SendMessage(item , CB_SETCURSEL , pos , 0); sel = -2;}
        }
    }
    if (sel != -2) SendMessage(item , CB_SETCURSEL , 0 , 0);
}

void onSizeInfo(HWND hwnd , int w , int h , bool bar=1) {
  HDWP hDwp;
  if (!w && !h) {
    RECT rc;
    GetClientRect(hwnd , &rc);
    w = rc.right;
    h = rc.bottom;
  }
  int bw = 0; 
  int bh = SendMessage(GetDlgItem(hwnd , Act[IMIG_NFO_BAR].index) , RB_GETBARHEIGHT , 0 , 0);

  hDwp=BeginDeferWindowPos(3);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , IDC_TREE) ,0, 5,11,150,h - 11 - 4 - bh,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , IDG_GROUP) ,0, 160,5,w - 160 - 5 - bw,h - 5 - 4 - bh,SWP_NOZORDER);
//  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , IDC_TIP) ,0 , 6, h - 38,w - 4 - 6 - bw,35,SWP_NOZORDER);
  if (bar)
    hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , Act[IMIG_NFO_BAR].index) ,0,
       2,h-bh-2,w-4,bh,SWP_NOZORDER);

//  GetWindowInfo(hwndUp , &wi);
//  int y=(wi.dwStyle & WS_VISIBLE)? 22 : 0;
  EndDeferWindowPos(hDwp);
  SendMessage(GetDlgItem(hwnd , Act[IMIG_NFO_BAR].index), TB_AUTOSIZE, 0, 0);

}

#define TITLE_FIX ""

void InfoDialogSummary(int cnt , bool local) {
    Act[IMIG_NFOWND].setCnt(cnt);
    ActionStatus(sUIAction(IMIG_NFO , IMIG_NFO_SUMMARY) 
        , (!Ctrl->DTgetPos(DTCNT , cnt))||(GetProp((HWND)Act[IMIG_NFOWND].ghandle , "NEWContact"))?ACTS_HIDDEN:0);
	if (!Ctrl->DTgetPos(DTCNT, cnt) || !Cnt[cnt].hwndInfo) return;
    CStdString sum = "";
	cPreg preg(false);
    Act[IMIG_NFO].setCnt(cnt , true);
	int d_gender = local?atoi(getActionValue(Act(CNT_GENDER|IMIB_CNT)).c_str()) : GETCNTI(cnt , CNT_GENDER);
#define D_STRING(var , col) \
    CStdString var = CntGetInfoValue(local , cnt , col)
    D_STRING(d_name , CNT_NAME);
    D_STRING(d_middlename , CNT_MIDDLENAME);
    D_STRING(d_surname , CNT_SURNAME);
    D_STRING(d_nick , CNT_NICK);
    D_STRING(d_display , CNT_DISPLAY);
    D_STRING(d_email , CNT_EMAIL);
    D_STRING(d_phone , CNT_PHONE);
    D_STRING(d_cellphone , CNT_CELLPHONE);
    D_STRING(d_fax , CNT_FAX);
    D_STRING(d_url , CNT_URL);
    D_STRING(d_description , CNT_DESCRIPTION);
    D_STRING(d_street , CNT_STREET);
    D_STRING(d_address_more , CNT_ADDRESS_MORE);
    D_STRING(d_postalcode , CNT_POSTALCODE);
    D_STRING(d_country , CNT_COUNTRY);
    D_STRING(d_region , CNT_REGION);
    D_STRING(d_locality , CNT_LOCALITY);
	D_STRING(d_pobox , CNT_POBOX);
// WORK
    D_STRING(d_w_organization , CNT_WORK_ORGANIZATION);
	D_STRING(d_w_org_unit , CNT_WORK_ORG_UNIT);
	D_STRING(d_w_title , CNT_WORK_TITLE);
	D_STRING(d_w_role , CNT_WORK_ROLE);
	D_STRING(d_w_email , CNT_WORK_EMAIL);
	D_STRING(d_w_phone , CNT_WORK_PHONE);
	D_STRING(d_w_fax , CNT_WORK_FAX);
	D_STRING(d_w_url , CNT_WORK_URL);

	D_STRING(d_w_street , CNT_WORK_STREET);
    D_STRING(d_w_address_more , CNT_WORK_ADDRESS_MORE);
	D_STRING(d_w_pobox , CNT_WORK_POBOX);
    D_STRING(d_w_postalcode , CNT_WORK_POSTALCODE);
    D_STRING(d_w_country , CNT_WORK_COUNTRY);
    D_STRING(d_w_region , CNT_WORK_REGION);
    D_STRING(d_w_locality , CNT_WORK_LOCALITY);

    D_STRING(d_uid , CNT_UID);

    int d_born = 0;
	int d_net = local?atoi(getActionValue(Act(CNT_NET|IMIB_CNT)).c_str()) : GETCNTI(cnt , CNT_NET);

    if (local) {
		cDate64 time = _atoi64(getActionValue(Act(CNT_BORN|IMIB_CNT)).c_str());
        d_born = (BYTE)time.day | (BYTE)time.month << 8 | (WORD)time.year << 16;
    } else 
        d_born = GETCNTI(cnt , CNT_BORN);


/*    if (d_gender == GENDER_UNKNOWN && d_name.size() > 1) {
        d_gender = (d_name[d_name.size()-1]=='a')?GENDER_FEMALE : GENDER_MALE;
    }*/
    sum += d_name;
    if (!d_middlename.empty()) {
        if (!sum.empty()) sum+=' ';
        sum+=d_middlename;
    }
    if (!d_nick.empty()) {
        if (!sum.empty()) sum+=' ';
        sum+='"'+d_nick+'"';
    }
    if (!d_surname.empty()) { 
        if (!sum.empty()) sum+=' ';
        sum+=d_surname;
    }
    if (sum.empty()) sum = d_display;
	// uzupe³niamy...
	sum = "<p align='left'><p align='center'><span class='title'>" + sum;
	sum += "<br/></span>";

//	sum+=" jest ";
    sum+=(d_gender==GENDER_MALE)?"mê¿czyzna ":d_gender==GENDER_FEMALE?"kobieta ":"";
    if (d_born) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        st.wYear;
		sum+="lat ";
        sum+="<span class='value'>" + string(inttoch(st.wYear-((d_born&0xFFFF0000)>>16)
			-(st.wMonth < ((d_born&0xFF00)>>8)?1:0))) + "</span>";
    }
    sum+="</p><br/>"; // center
    if (d_net) {
		char * net_name = (char*)IMessage(IM_PLUG_NETNAME , (Net::tNet) d_net  , imtNet);
        if (net_name)
            sum+="<div class='var'>Sieæ:\t<span class='value'>" + string(net_name)
                + "  #" + string(d_uid) + "</span></div>";
    }
    if (!d_email.empty()) {
        sum+="<div class='var'>eMail:\t<span class='value'>" + d_email + "</span></div>";
    }
    if (!d_url.empty()) {
        sum+="<div class='var'>WWW:\t<span class='value'>" + d_url + "</span></div>";
    }
    if (!d_phone.empty()) {
        sum+="<div class='var'>Tel. stacjonarny:\t<span class='value'>" + d_phone + "</span></div>";
    }
    if (!d_cellphone.empty()) {
        sum+="<div class='var'>Komórka:\t<span class='value'>" + d_cellphone + "</span></div>";
    }
    if (!d_fax.empty()) {
        sum+="<div class='var'>Fax:\t<span class='value'>" + d_fax + "</span></div>";
    }
    string sum_address="";
    if (!d_address_more.empty()) {
        sum_address+="" + d_address_more + "<br/>";
    }
    if (!d_street.empty()) {
        sum_address+="ul. " + d_street + "<br/>";
    }
    if (!d_pobox.empty()) {
        sum_address+="skr. " + d_pobox + "<br/>";
    }
    if (!d_postalcode.empty()) {
        sum_address+= d_postalcode;
    }
    if (!d_locality.empty()) {
        sum_address+=(!d_postalcode.empty()?" ":"") + d_locality + "<br/>";
    } else if (!d_postalcode.empty()) sum_address+="<br/>";
    if (!d_region.empty()) {
        sum_address+=d_region + "<br/>";
    }
    if (!d_country.empty()) {
        sum_address+=d_country + "<br/>";
    }
    if (!sum_address.empty()) {
        sum+="<div class='subtitle'>" TITLE_FIX "ADRES" TITLE_FIX "</div><p class='value' align='center'>" + sum_address + "</p>";
    }
	string sum_work="";
    if (!d_w_organization.empty()) {
		sum_work+="<div class='var'>Organizacja:\t<span class='value'>" + d_w_organization;
	    if (!d_w_org_unit.empty())
			sum_work+=" - " + d_w_org_unit;
		sum_work+="</span></div>";
    }
    if (!d_w_url.empty()) {
        sum_work+="<div class='var'>Strona WWW:\t<span class='value'>" + d_w_url + "</span></div>";
    }
    if (!d_w_email.empty()) {
        sum_work+="<div class='var'>eMail:\t<span class='value'>" + d_w_email + "</span></div>";
    }
    if (!d_w_phone.empty()) {
        sum_work+="<div class='var'>Telefon:\t<span class='value'>" + d_w_phone + "</span></div>";
    }
    if (!d_w_fax.empty()) {
        sum_work+="<div class='var'>Fax:\t<span class='value'>" + d_w_phone + "</span></div>";
    }

    if (!d_w_title.empty()) {
		sum_work+="<div class='var'>Stanowisko:\t<span class='value'>" + d_w_title;
	    if (!d_w_role.empty())
			sum_work+=" - " + d_w_role;
		sum_work+="</span></div>";
    }

    sum_address="";
    if (!d_w_address_more.empty()) {
        sum_address+= d_w_address_more + "<br/>";
    }
    if (!d_w_street.empty()) {
        sum_address+="ul. " + d_w_street + "<br/>";
    }
    if (!d_w_pobox.empty()) {
        sum_address+="skr. " + d_w_pobox + "<br/>";
    }
    if (!d_w_postalcode.empty()) {
        sum_address+= d_w_postalcode;
    }
    if (!d_w_locality.empty()) {
        sum_address+=(!d_w_postalcode.empty()?" ":"") + d_w_locality + "<br/>";
    } else if (!d_w_postalcode.empty()) sum_address+="<br/>";
    if (!d_w_region.empty()) {
        sum_address+="" + d_w_region + "<br/>";
    }
    if (!d_w_country.empty()) {
        sum_address+="" + d_w_country + "<br/>";
    }
    if (!sum_address.empty()) {
        sum_work+="<div class='subtitle'>ADRES (praca)</div><p class='value' align='center'>" + sum_address + "</p>";
    }
    if (!sum_work.empty()) {
        sum+="<div class='subtitle'>" TITLE_FIX "PRACA" TITLE_FIX "</div>" + sum_work;
    }


	if (d_net) {
		if (*GETCNTC(cnt , CNT_HOST) && GETCNTI(cnt , CNT_PORT)) {
			sum +="<div class='var'>IP\t<span class='value'>";
			sum +=GETCNTC(cnt,CNT_HOST);
			sum +=":";
			sum +=GETCNTC(cnt,CNT_PORT);
			sum +="</span></div>";
		}
		sum += "<div class='var'>Status\t<span class='value'>" + getStatusName(GETCNTI(cnt , CNT_STATUS));
		if (*GETCNTC(cnt , CNT_STATUSINFO)) {
			sum +=" \"" + string(GETCNTC(cnt , CNT_STATUSINFO)) + "\"";
		}
 		sum +="</span></div>";
		cTime64 t = GETCNTI64(cnt , CNT_LASTACTIVITY);
		if (!t.empty()) {
			sum+=t.strftime("<div class='var'>Ostatnio aktywny: <span class='value'>%d %B %Y %H:%M:%S</span></div>");
		}
	}
	sum += "</p>";
    ActionStatus(Act[IMIG_NFO_SUMMARY](IMIA_NFO_SUM).act() , 0 , (char*)sum.c_str());
}

void InfoDialogSetCnt(sUICnt * cnt) {
	if (!cnt) return;
	Act[IMIG_NFOWND].setCnt(cnt->ID , true);
}

int CALLBACK InfoDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
//   int a=Cnt.findBy(hwndDlg,1);
//   if (a<0) return 0;
   sUICnt * cnt = (sUICnt*)GetProp(hwnd , "CNT");
   if (message == WM_NOTIFY) 
	   InfoDialogSetCnt(cnt); /* Zanim wywo³amy configproc ... */
   int ret = ConfigProc(hwnd , message , wParam , lParam);
   switch (message)
   {
        case WM_INITDIALOG: {
                SetProp(hwnd , "CNT" , (void*)lParam);
                SendMessage(hwnd , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_INFO].ico_);
                SendMessage(hwnd , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_INFO].ico);
                cnt=(sUICnt*)lParam;
//                int p2 = (int)cnt->hwndInfo;
                cnt->hwndInfo = hwnd;
                SetProp(hwnd , "CNTID" , (void*)(cnt->user?0:cnt->ID));
/*                if (!cnt->user && !*GETCNTC(cnt->ID , CNT_DISPLAY)) {
                    SetProp(hwnd , "NEWContact" , (void*)lParam);
                }*/
                SetWindowText(hwnd , _sprintf("Informacje o %s",GETCNTC(cnt->ID , CNT_DISPLAY)));
                // Wyglad
                //SendDlgItemMessage(hwndDlg , IDE_BORN , DTM_SETFORMAT , 0 , (LPARAM)"dd-MM-yyyy");

                SendMessage(GetDlgItem(hwnd,IDC_TREE) , TVM_SETIMAGELIST , TVSIL_NORMAL , (LPARAM)Ico.iml[0]);
				SendMessage(GetDlgItem(hwnd , IDG_GROUP), WM_SETFONT, (WPARAM)fontB, 1);
                /*
                HTREEITEM hti;
                hti = TreeView_GetRoot(GetDlgItem(hwnd,IDC_TREE));
                while (hti) {
                  TreeView_Expand(GetDlgItem(hwnd,IDC_TREE) , hti , TVE_EXPAND);
                  hti = TreeView_GetNextSibling(GetDlgItem(hwnd,IDC_TREE) , hti);
                }
                */
                // Ustawia wszystkie kontrolki
                //SetDlgItemText(hwndDlg , IDE_DISPLAY , (char*)IMessage(IMC_GETCNTC ,0,0,cnt->pos,CNT_DISPLAY));
                break;}
        case MYWM_REFRESH: { // Wypelniamy wszystko elegancko
                Act[IMIG_NFOWND].ghandle=hwnd;
                Act[IMIG_NFOWND].setCnt(cnt->ID,true);
                Act.Mod.loadWindow(Act[IMIG_NFOWND]);
                InfoDialogSummary(cnt->ID , false);
                if (GetProp(hwnd , "NEWContact")) {
                    ActionStatus(Act[IMIG_NFO_SUMMARY].act() , ACTS_HIDDEN);
                }
                makeCfgTree(Act[IMIG_NFO] , GetDlgItem(hwnd,IDC_TREE));
                Act[IMIG_NFO_BAR].cnt = cnt->ID;
                makeBar(hwnd , Act[IMIG_NFO_BAR]);
                onSizeInfo(hwnd , 0 , 0);
                makeCfgProp(Act[IMIG_NFO_SUMMARY]);
                makeCfgProp(Act[IMIG_NFO_DETAILS]);
                makeCfgProp(Act[IMIG_NFO_CONTACT]);
				makeCfgProp(Act[IMIG_NFO_WORK]);
                makeCfgProp(Act[IMIG_NFO_INFO]);
                openCfgAction(wParam?*(cUIGroup*)wParam:cnt->ID?Act[IMIG_NFO_SUMMARY]:Act[IMIG_NFO_DETAILS]);

                break;}
        case WM_SIZING:
			 InfoDialogSetCnt(cnt);
 			 clipPropSize(hwnd, wParam, *((RECT *)lParam), 250, 200);
             return 1;
        case WM_SIZE:
			 InfoDialogSetCnt(cnt);
             onSizeInfo(hwnd , LOWORD(lParam) , HIWORD(lParam));
             resizeProp(hwnd);
             return 1;

        case WM_NOTIFY: {
                NMHDR * nmhdr=(NMHDR *)lParam;
                switch (nmhdr->code) {
                    case RBN_HEIGHTCHANGE:
					    InfoDialogSetCnt(cnt);
                        onSizeInfo(hwnd , 0,0 , 1);
                        return 0;
                }
                break;
                }



        case  WM_ACTIVATE:
			InfoDialogSetCnt(cnt);
            return 0;

		case WM_ENTERIDLE:
		    return UIMessageProc(0,hwnd , message , wParam , lParam);


        case WM_COMMAND:

             if (cnt) UIMessageProc(cnt->ID , hwnd , message , wParam , lParam);
             if (!lParam) return 0;
             switch(HIWORD(wParam)) {
               case BN_CLICKED:{ // Control
                  cnt=(sUICnt*)GetProp(hwnd , "CNT");
                  cUIGroup * pGrp;
                  if ((pGrp=(cUIGroup*)GetProp((HWND)lParam , "ACTION"))) {
                    int b = pGrp->findIndex(LOWORD(wParam));
                    if (b<0) return 0;
                    (*pGrp)[b].setCnt(cnt->ID);
                    (*pGrp)[b].call(ACTN_ACTION,0,0);
                  }
                  break;}
             }     
             break;


        case WM_CLOSE:
                InfoDialogCancel((int)GetProp(hwnd , "CNTID") , hwnd);
                break;
        case WM_DESTROY:
				if (cnt)
					Act[IMIG_NFOWND].setCnt(cnt->ID , true);
				destroyCfgTree(&Act[IMIG_NFO]);
                Act[IMIG_NFOWND].ghandle = hwnd;
                Act.Mod.saveWindow(Act[IMIG_NFOWND]);
                Act[IMIG_NFOWND].setCnt(AC_NONE , true);
                if (!cnt) return 0;
//         IMLOG("hwndC %x" , cnt->hwndInfo);
//         IMLOG("hwndD %x" , hwndDlg);
				/*TODO: SPRAWDZIÆ!*/
                cnt->hwndInfo=0;
                RemoveProp(hwnd , "CNT");
                //if (Act[IMIG_NFOWND].cnt == cnt->ID) Act[IMIG_NFOWND].setCnt(-1 , true);
                break;
//        case WM_GETDLGCODE:
//                return DLGC_BUTTON|DLGC_DEFPUSHBUTTON;
                
	default:
                break;
   }
   if (cnt) 
	   UIMessageProc(cnt->ID , hwnd , message , wParam , lParam);

   return ret;
}


//-------------------------------------------------------------
//-------------------------------------------------------------
void ConfigDialogOpen (int id) {
	if (!id) id = IMIG_CFG_SETTINGS;
    if (!Act.exists(id)) return;
    openCfgAction(Act[id]);
}

// ---------------------------------------------------------

void onSizeConfig(int w , int h) {
  HDWP hDwp;
//  }
//  WINDOWINFO wi;wi.cbSize=sizeof(WINDOWINFO);
  hDwp=BeginDeferWindowPos(5);

#define TREE_WIDTH 180

  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDC_TREE) ,0, 5,11,TREE_WIDTH,h - 11 - 40,SWP_NOZORDER);

  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDG_GROUP) ,0, TREE_WIDTH+10,5,w - (TREE_WIDTH+10) - 5,h - 5 - 40,SWP_NOZORDER);

  //hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDC_TIP) ,0 , 6, h - 38,w - 265 - 4 - 6,35,SWP_NOZORDER);

  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDCANCEL) ,0 , w - 265, h - 33,80,25,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDAPPLY)  ,0 , w - 170, h - 33,80,25,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndConfig , IDOK)     ,0 , w - 85 , h - 33,80,25,SWP_NOZORDER);
#undef TREE_WIDTH
//  GetWindowInfo(hwndUp , &wi);
//  int y=(wi.dwStyle & WS_VISIBLE)? 22 : 0;
  EndDeferWindowPos(hDwp);
}




//-------------------------------------------------------------
INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam) {
   RECT rc;
   int ret = ConfigProc(hwndDlg , message , wParam , lParam);
   switch (message)
   {
        case MYWM_CANAPPLY:
            EnableWindow(GetDlgItem(hwndDlg , IDAPPLY) , true);
            return 0;
        case WM_INITDIALOG:
                EnableWindow(GetDlgItem(hwndDlg , IDAPPLY) , false);
                hwndConfigTree=GetDlgItem(hwndDlg , IDC_TREE);
                SendMessage(hwndDlg , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_CONFIG].ico);
				SendMessage(hwndDlg , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_CONFIG].ico_);
                SendMessage(hwndConfigTree , TVM_SETIMAGELIST , TVSIL_NORMAL , (LPARAM)Ico.iml[0]);
//                onSizeConfig( , );
                hwndConfig = hwndDlg;
                Act[IMIG_CFGWND].ghandle = hwndDlg;
                GetClientRect(GetDesktopWindow() , &rc);
                SetWindowPos(hwndDlg , 0 , rc.right / 2 - 260 , rc.bottom / 2 - 140 , 520 , 450 , SWP_NOZORDER);
                Act.Mod.loadWindow(Act[IMIG_CFGWND]);
                makeCfg();
                HTREEITEM hti;
                hti = TreeView_GetRoot(hwndConfigTree);
                while (hti) {
                  TreeView_Expand(hwndConfigTree , hti , TVE_EXPAND);
                  hti = TreeView_GetNextSibling(hwndConfigTree , hti);
                }
                TreeView_EnsureVisible(hwndConfigTree,TreeView_GetRoot(hwndConfigTree)); 
				SendMessage(GetDlgItem(hwndDlg , IDG_GROUP), WM_SETFONT, (WPARAM)fontB, 1);
                break;
        case WM_SIZING:
			clipPropSize(hwndDlg, wParam, *((RECT *)lParam), 400, 270);
            return 1;
        case WM_SIZE:
//             IMessage(IMC_LOG , 0,0,(int)"WM_SIZE" , wParam);
             onSizeConfig(LOWORD(lParam) , HIWORD(lParam));
             resizeProp(hwndDlg);
             return 1;
        case WM_DESTROY:
                Act.Mod.saveWindow(Act[IMIG_CFGWND]);
                Act[IMIG_CFGWND].ghandle = 0;
                hwndConfig=0;
                destroyCfgTree();
                break;
        /*case WM_NOTIFY:
                nmhdr=(NMHDR *)lParam;
                if (nmhdr->hwndFrom==hwndConfigTree) {
                     switch (nmhdr->code) {
                       case TVN_SELCHANGED:
                         nmtv=(NMTREEVIEW *)lParam;
                         if (nmtv->itemNew.lParam)
                           openCfgAction(*(cUIGroup *)nmtv->itemNew.lParam);
                         break;
                       case TVN_GETINFOTIP: {
                         cUIAction * act = (cUIAction*)((NMTVGETINFOTIP*)lParam)->lParam;
                         SetCfgTip(GetDlgItem(hwndDlg , IDC_TIP) , (act && !act->tip.empty())?act->tip.c_str() : "" , act);
                         break;}
                       }
                     }
                break;*/
        case WM_COMMAND:
                switch (HIWORD(wParam)) {
                 case BN_CLICKED: // Control
                     switch(LOWORD(wParam)) {
                       case IDOK :
                          saveCfg();
                          DestroyWindow(hwndConfig);
                          break;
                       case IDAPPLY:
                          EnableWindow(GetDlgItem(hwndDlg , IDAPPLY) , false);
                          saveCfg();
                          break;
                       case IDCANCEL:
                          DestroyWindow(hwndConfig);
                          break;
                     }
                     return 1;
                }     
                break;     
        case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;
		case WM_ENTERIDLE:
		    return UIMessageProc(0,hwndDlg , message , wParam , lParam);

   }
   return ret;
}


//----------------------------------------------------------
// -----------------------------------------------------

void onSizeMsg(HWND hwnd , int w , int h , bool bar) {
	if (GetProp(hwnd, "wndCreated") == 0) return;
  HDWP hDwp;
  if (!w && !h) {
    RECT rc;
    GetClientRect(hwnd , &rc);
    w = rc.right;
    h = rc.bottom;
  }
  hDwp=BeginDeferWindowPos(bar?2:3);
  int bh = SendMessage(GetDlgItem(hwnd , Act[IMIG_MSGBAR].index) , RB_GETBARHEIGHT , 0 , 0);
  sUICnt * cnt=(sUICnt*)GetProp(hwnd , "CNT");
  IMDEBUG(DBG_DEBUG, "onSizeMsg id=%x bar=%d ix=%x bh=%d", cnt->ID, bar, Act[IMIG_MSGBAR].index, bh);
  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , IDC_MSG) ,0,
       0,0,w,h - 80 - bh,SWP_NOZORDER);
  if (bar && bh)
    hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , Act[IMIG_MSGBAR].index) ,0,
       0,h-80-bh,w,bh,SWP_NOZORDER);

  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwnd , Act(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend)).index) ,0,
       1,h-79,w-2,78,SWP_NOZORDER);
  EndDeferWindowPos(hDwp);
}


//-------------------------------------------------------------

void MsgCreateWindows(HWND hwndOwner , sUICnt * cnt) {
//  IHTMLWindow2 * hw2;
//  IHTMLWindow2::open(L"http://onet.pl" , L"" , L"" , 0 , &hw2);

//   CreateWindow("{3050F4F8-98B5-11CF-BB82-00AA00BDCE0B}" , "eee" ,
//     WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_THICKFRAME , 10,10,300,300,0,0,hInst,0);
//   ErrorMsg();

//RICHEDIT_CLASS
/*  hwndRE = CreateWindowEx(0,"EDIT",NULL,
                           WS_CHILD|WS_VISIBLE|ES_MULTILINE | ES_AUTOVSCROLL |
                           ES_WANTRETURN | WS_VSCROLL | WS_BORDER
                           ,
                           0,0,200,50,
                           hwndOwner,(HMENU)IDC_MSG,hInst,NULL);
  SendMessage(hwndRE , WM_SETFONT , (WPARAM)font , 1);*/

	IMDEBUG(DBG_FUNC, "MsgCreateWindows(id=%x)", cnt->ID);

  Act[IMIG_MSGWND].cnt = cnt->ID;
  Act[IMIG_MSGWND].ghandle = hwndOwner;
  Act[IMIG_MSGBAR].cnt = cnt->ID;
  cnt->hwndMsg = hwndOwner;
  makeBar(hwndOwner , Act[IMIG_MSGBAR]);

	sUIAction sendAct = sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , cnt->ID);
	sUIActionNotify_createWindow cw;
	cw.hwndParent = hwndOwner;
	cw.act = sendAct;
	Act(sendAct).call(&cw);
	SetWindowLong(cw.hwnd , GWL_ID , Act(sendAct).index);
	Act(sendAct).handle = cw.hwnd;
	cnt->_msgControl = new cMsgControl(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlview , cnt->ID) , 1,1,100,100,hwndOwner , (HMENU)IDC_MSG);

	//Act[IMIG_MSGWND].setCnt(-1, true);
	Act[IMIG_MSGWND].setCnt(cnt->ID, true);

	cnt->MsgUpdate(false);

	SetProp(hwndOwner, "wndCreated", (HANDLE)1);
  #ifdef __DEBUG
  DbgActionsRefresh(&Act[IMIG_MSGWND]);
  #endif
}

//-------------------------------------------------------------
INT_PTR CALLBACK MsgDialogProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam) {
   sUICnt * cnt;
   int b;
   const char * cch;
   UIMessageProc(0,hwndDlg , message , wParam , lParam);
   switch (message)
   {
        case WM_INITDIALOG: {
                SetProp(hwndDlg , "CNT" , (void*)lParam);
                SetProp(hwndDlg , "MsgWaiting" , 0);
				SetProp(hwndDlg , "MSGBYENTER" , (HANDLE)GETINT(CFG_UIMSGBYENTER));
                cnt=(sUICnt*)lParam;
                cnt->hwndMsg = hwndDlg;
				//SendMessage(hwndDlg , MYWM_SETICON , 0 , 0);
                SendMessage(hwndDlg , MYWM_SETICON , IDI_WND_MSG , 0);
//                CreateWindowEx();
                MsgCreateWindows(hwndDlg , (sUICnt *)lParam);
                IMLOG("MSGWNDCRATED");
                SetWindowPos(hwndDlg , 0 , 0 , 0 , 450 , 280 , SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
				Act.Mod.loadWindow(Act[IMIG_MSGWND]);
                //onSizeMsg(hwndDlg,0,0);
//                SendDlgItemMessage(hwndDlg , IDC_EDIT , DM_SETDEFID , IMIA_MSG_SEND,0);

                break;}
		case MYWM_SETICON: {
                SendMessage(hwndDlg , WM_SETICON , ICON_SMALL , (LPARAM)Ico[wParam].ico);
                SendMessage(hwndDlg , WM_SETICON , ICON_BIG , (LPARAM)Ico[wParam].ico_);
				break;}
        case MYWM_REFRESH: {
                 cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
                 cch = GETCNTC(cnt->ID , CNT_DISPLAY);
                 if (!*cch) cch = GETCNTC(cnt->ID , CNT_UID);
                 string title = formatTitle((char*)GETSTR(CFG_UIMSGTITLE), cnt->ID , FT_WINDOW);
                 SetWindowText(hwndDlg , title.c_str());
//                 DeleteObject(fnt);

                }
                break;
        case WM_SIZING:
             clipSize(wParam , (RECT *)lParam , 250 , 200);
             return 1;
        case WM_SIZE:
             onSizeMsg(hwndDlg , LOWORD(lParam) , HIWORD(lParam));
             return 1;
		case WM_CLOSE: {
             Act[IMIG_MSGWND].ghandle = hwndDlg;
             Act[IMIG_MSGWND].cnt = -1;
             Act.Mod.saveWindow(Act[IMIG_MSGWND]);
			 bool remember = GETINT(CFG_UIREMEMBERMSGWND)!=0;
			 if (GetKeyState(VK_SHIFT)&0x80)
				 remember = !remember;
             if (remember) ShowWindow(hwndDlg , SW_HIDE);
               else DestroyWindow(hwndDlg);
			 break;}
        case WM_NCDESTROY:
             cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
             cnt->MsgWndClose();
			 Act[IMIG_MSGWND].setCnt(cnt->ID);
			 destroyActionsTree(Act[IMIG_MSGWND]);
             Act[IMIG_MSGWND].setCnt(-1);
             RemoveProp(hwndDlg , "CNT");
             break;
/*        case WM_ACTIVATE:
                cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
                curCnt = cnt->pos;
                IMLOG("curCnt = %d" , cnt->pos);
                return 1;*/
        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
             //Act[IMIG_MSGWND].setCnt(cnt->ID , true);
             switch (nm->code) {
               case RBN_HEIGHTCHANGE:
                   onSizeMsg(hwndDlg , 0,0 , 1);
                   return 0;
/*
               case TBN_DROPDOWN:
               case RBN_CHEVRONPUSHED:
                   return UIMessageProc(cnt->ID , hwndDlg , message , wParam , lParam);
*/
               case EN_SELCHANGE:
//                      SendMessage(nmhdr->hwndFrom , EM_HIDESELECTION , 1 , 0);
				   if (GetWindowLong(nm->hwndFrom, GWL_STYLE) & ES_READONLY) {
						HideCaret(nm->hwndFrom);
					}
                    break;



             }
             return 0;



        case WM_COMMAND:
             if (!lParam) return 0;
             switch(HIWORD(wParam)) {
               case BN_CLICKED:{ // Control
                  cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
//                  IMLOG("Clicked w %d curCnt %d parent %d" , LOWORD(wParam) , curCnt , GetMenu((HWND)lParam));
                  cUIGroup * pGrp;
                  if ((pGrp=(cUIGroup*)GetProp((HWND)lParam , "ACTION"))) {
                    b = pGrp->findIndex(LOWORD(wParam));
                    if (b<0) return 0;
                    (*pGrp)[b].setCnt(cnt->ID);
                    (*pGrp)[b].call(ACTN_ACTION,0,0);
                  }
                  break;}
             }     
             break;
//        case DM_GETDEFID:
//             if (GetFocus() != GetDlgItem(hwndDlg , IDC_EDIT)) return 0;
//             IMessageDirect(IM_UIACTION,0,(int)Act[IMIG_UIMSG](IMIA_MSG_SEND).fill(curCnt),ACTF_ACTION);
//             return 0;

        case  WM_ACTIVATE:
             cnt=(sUICnt*)GetProp(hwndDlg , "CNT");
             Act[IMIG_MSGWND].setCnt(cnt->ID);
             if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) {
  			   SetFocus(GetDlgItem(hwndDlg , Act(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend , cnt->ID)).index));
               if (GetProp(hwndDlg , "WindowFlashing")) {
                    FLASHWINFO fwi;
                    memset(&fwi , 0 , sizeof(fwi));
                    fwi.cbSize=sizeof(fwi);
                    fwi.hwnd=hwndDlg;
                    fwi.dwFlags = FLASHW_STOP | FLASHW_STOP;
                    FlashWindowEx(&fwi);
                    SetProp(hwndDlg , "WindowFlashing" , 0);
               }
               if (GetProp(hwndDlg , "MsgWaiting") || cnt->notify) {
                  // Usuwa powiadomienia ...
                  MessageSelect mp;
                  mp.net = cnt->net;
                  mp.setUid(GETCNTC(cnt->ID , CNT_UID));
                  mp.type = Message::typeMessage;
                  mp.wflag = Message::flagOpened;
                  mp.woflag = Message::flagSend;
                  ICMessage(MessageSelect::IM::imcMessageRemove, (int)&mp , -1);
                  SetProp(hwndDlg , "MsgWaiting" , 0);
                  SendMessage(hwndDlg , MYWM_SETICON , IDI_WND_MSG , 0);
               }
             }
             return 0;
        case WM_MOUSEACTIVATE:
            return MA_ACTIVATE;
		case WM_SHOWWINDOW:
			SetFocus(GetDlgItem(hwndDlg , Act(sUIAction(IMIG_MSGWND , Konnekt::UI::ACT::msg_ctrlsend)).index));
			return 0;
             
        case WM_CTLCOLOREDIT: {
/*            static HBRUSH hBrush = 0;
            if ((WORD)GetMenu((HWND)lParam) != IDC_EDIT) return 0;
            HDC DC = (HDC)wParam;
            LOGFONTEX lf = StrToLogFont(GETSTR(CFG_UIF_MSGSEND)); 
            SetTextColor(DC , (COLORREF)GetProp((HWND)lParam , "Color"));
            SetBkColor(DC , (COLORREF)GetProp((HWND)lParam , "BGColor"));
            if (hBrush) DeleteObject(hBrush);
            hBrush = CreateSolidBrush((COLORREF)GetProp((HWND)lParam , "BGColor"));
            return (int)hBrush;	*/
            }     

   }
   return 0;
}

INT_PTR CALLBACK EditMsgControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   sUICnt * cnt;
   switch (message)
   {
   case WM_GETDLGCODE:
	   return DLGC_WANTALLKEYS;
   case WM_CONTEXTMENU: {
	   if (lParam == -1) {
		   POINT caret;
		   if (GetCaretPos(&caret)) {
			   ClientToScreen(GetFocus(), &caret);
			   lParam = MAKELONG( caret.x + 4, caret.y + 16 );
		   }
	   }
	   SetCursor(LoadCursor(0,IDC_ARROW));
       cnt=(sUICnt*)GetProp(GetParent(hwnd) , "CNT");
	   Act[UI::ACT::msg_ctrlsend_menu].setCnt(cnt->ID);	
	   UIPopupMenu(Act[UI::ACT::msg_ctrlsend_menu], TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, LOWORD(lParam), HIWORD(lParam), 0, hwnd, 0, 0);
	   break;
		}
   case WM_MEASUREITEM:
   case WM_DRAWITEM: {
       cnt=(sUICnt*)GetProp(GetParent(hwnd) , "CNT");
	   return UIMessageProc(cnt->ID, hwnd, message, wParam, lParam);}
     case WM_SYSKEYDOWN:
         //Beep(200,200);
//         goto send;
//       IMLOG("SYSKEY [%d] '%c'  |  STATE: %x %x %x" , wParam , wParam , GetKeyState(VK_SHIFT)&0x80 , GetKeyState(VK_CONTROL)&0x80 , GetKeyState(VK_MENU)&0x80);
     case WM_KEYDOWN:
//       if (GetKeyState(VK_MENU)&0x80 || (unsigned char)wParam < 32) IMLOG("[%d] '%c'  |  STATE: %x %x %x" , wParam , wParam , GetKeyState(VK_SHIFT)&0x80 , GetKeyState(VK_CONTROL)&0x80 , GetKeyState(VK_MENU)&0x80);
       cnt=(sUICnt*)GetProp(GetParent(hwnd) , "CNT");
	   Konnekt::UI::Cnt::checkComposing.composing(cnt->ID);
	   bool ctrl = (GetKeyState(VK_CONTROL)&0x80)!=0;
	   bool shift = (GetKeyState(VK_SHIFT)&0x80)!=0;
	   bool alt = GetKeyState(VK_MENU)&0x80;
       if ((wParam == VK_UP || wParam == VK_DOWN)) {
		   using namespace Konnekt::UI;
		   CFG::enMessageHistory type = (CFG::enMessageHistory)GETINT(CFG_UIMESSAGEHISTORY);
		   if (type == CFG::mhistDisabled || shift || (type == CFG::mhistWithCtrl && !ctrl) )
			   break;
		   cUIAction & sendAct = Act(sUIAction(IMIG_MSGWND , ACT::msg_ctrlsend , cnt->ID));
		   sendAct.setCnt(cnt->ID);
	  	   //int length = sendAct.call(Notify::getMessageSize , 0 , 0);
		   Notify::_getSelection gs;
	  	   sendAct.call(&gs);
		   if (wParam == VK_UP) {
			   if (!ctrl && sendAct.call(&sUIActionNotify_2params(Notify::getMessageLine , gs._start , 0)) > 0) break;
			   cnt->useMsgHistory(false);
		   } else { // VK_DOWN
			   if (!ctrl && sendAct.call(&sUIActionNotify_2params(Notify::getMessageLine , gs._end , 0))+1 < sendAct.call(&sUIActionNotify_base(Notify::getMessageLines))) break;
			   cnt->useMsgHistory(true);
		   }
		   return 0;
       } else if (wParam==0x20 || (wParam >=0x30 && wParam <=0x5A)) {cnt->msgHistoryPos=0;}
//	   if (wParam == VK_RETURN) IMLOG("RETURN %x , %x" , GetKeyState(VK_SHIFT)&0x80 , GetKeyState(VK_CONTROL)&0x80);	
/*       if  (wParam == VK_RETURN && ((GetKeyState(VK_SHIFT)&0x80)
		    || (!(GetKeyState(VK_CONTROL)&0x80) && !GetProp(cnt->hwndMsg , "MSGBYENTER")))
		   )
         {
			 SendMessage(hwnd , EM_REPLACESEL,1,(LPARAM)"\r\n");
		 }*/
	   if (wParam == VK_ESCAPE) {
		   SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
	   }
	   if (ctrl && (wParam == 'I' || wParam == 'R' || wParam == 'L') ) {
		   return 1;
	   }
       if (
         (wParam == 'S' && (alt) && !(ctrl))
         ||
         (wParam == 'S' && !(alt) && (ctrl))
         ||
         (wParam == VK_RETURN &&
           (GetProp(cnt->hwndMsg , "MSGBYENTER")?
           !(shift || (ctrl) || alt)
           :ctrl)
         )  
           ) {

//         IMLOG("SENDING - %d %x" , curCnt , cnt);
           //         IMLOG("%x[%d] %x %x" , wParam , wParam , lParam , GetKeyState(VK_SHIFT));
         if (!  GetProp(hwnd , "SENDING")) {
           Act[IMIG_MSGTB](IMIA_MSG_SEND).setCnt(cnt->ID);
           Act[IMIG_MSGTB](IMIA_MSG_SEND).call(ACTN_ACTION,0,0);
		   return 0;
         }  
       }
		if (wParam == 'V' && ctrl) {
			sUIAction act (UI::ACT::msg_ctrlsend_menu, UI::ACT::msg_ctrlsend_menu_paste_nf, cnt->ID);
			if (shift)
				act.id = UI::ACT::msg_ctrlsend_menu_paste_qt;
			else if (alt)
				act.id = UI::ACT::msg_ctrlsend_menu_paste;
		   Act(act).call(ACTN_ACTION, 0, 0, cnt->ID);
		   return 1;
		}

       break;
   }
   WNDPROC proc = (WNDPROC)GetProp(hwnd, "OldWindowProc");
   if (proc) {
		return CallWindowProc(proc , hwnd , message , wParam , lParam);
   } else {
	   return 0;
	}
}





