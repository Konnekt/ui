


int ActionProfileProc(sUIActionNotify_base * anBase) {
  sUIActionNotify_2params * an = static_cast<sUIActionNotify_2params*>(anBase);
  int id = an->act.id & ~IMIB_;
  if (an->code!=ACTN_ACTION || id > Act[IMIG_MAIN_PROFILE].count - 3) return 0;
  string profile = Act(an->act).txt;
  if (ICMessage(IMI_CONFIRM , (int)STR(_sprintf("Profil zostanie zmieniony na %s." , profile.c_str())))) {
       ICMessage(IMC_PROFILECHANGE , (int)profile.c_str());
  }
  return  1;
}

int ActionCfgProc(sUIActionNotify_base * anBase) {
    sUIActionNotify_2params * an = static_cast<sUIActionNotify_2params*>(anBase);
    sUIAction ua = an->act;
    sUIActionInfo act = sUIActionInfo(an->act);
    act.mask = UIAIM_STATUS;
    Act(an->act).getInfo(&act);
    bool state;
    switch (an->act.id & ~IMIB_) {
		case CFG_TIMEOUT: case CFG_TIMEOUT_RETRY: {
			// w obu chodzi tylko o konwertowanie danych...
			if (anBase->code == ACTN_CONVERT_TO) {
				// obcinamy zera...
				sUIActionNotify_buff * ab = static_cast<sUIActionNotify_buff *>(anBase);
				char * buff = (char*)Ctrl->GetTempBuffer(16);
				itoa((atoi(ab->buff) / 1000) , buff, 10);
				ab->buff = buff;
				ab->buffSize = 15;
			} else if (anBase->code == ACTN_CONVERT_FROM) {
				// dodajemy zera...
				sUIActionNotify_buff * ab = static_cast<sUIActionNotify_buff *>(anBase);
				char * buff = (char*)Ctrl->GetTempBuffer(16);
				itoa((atoi(ab->buff) * 1000) , buff, 10);
				ab->buff = buff;
				ab->buffSize = 15;
			}
			break;}
       case CFG_PROXY:
       if (an->code == ACTN_CREATE || an->code == ACTN_STATUS || an->code == ACTN_ACTION) {
		   state = getActionValue(Act(an->act)) == "1";
         ua.id = CFG_PROXY_HTTP_ONLY | IMIB_CFG;
         IMLOG("PROXY");
         ActionStatus(ua , state?0:ACTS_DISABLED);
//         }
//       break;
//       case CFG_PROXY_AUTO:
//       if (an->code == ACTN_CREATE || an->code == ACTN_STATUS || an->code == ACTN_ACTION) {
//         state = ~act.status & ACTS_DISABLED && *getActionValue(*Act(an->act))=='0';
         ua.id = CFG_PROXY_HOST | IMIB_CFG;
         ActionStatus(ua , state?0:ACTS_DISABLED);
         ua.id = CFG_PROXY_PORT | IMIB_CFG;
         ActionStatus(ua , state?0:ACTS_DISABLED);
         ua.id = CFG_PROXY_AUTH | IMIB_CFG;
         ActionStatus(ua , state?0:ACTS_DISABLED);
         }
       break;
       case CFG_PROXY_AUTH:
       if (an->code == ACTN_CREATE || an->code == ACTN_STATUS || an->code == ACTN_ACTION) {
		   state = getActionValue(Act(an->act))=="1" && ~act.status & ACTS_DISABLED;
         ua.id = CFG_PROXY_LOGIN | IMIB_CFG;
         ActionStatus(ua , state?0:ACTS_DISABLED);
         ua.id = CFG_PROXY_PASS | IMIB_CFG;
         ActionStatus(ua , state?0:ACTS_DISABLED);
         }
       break;

       case CFG_UICANDOCK:
       if (an->code == ACTN_CREATE || an->code == ACTN_STATUS || an->code == ACTN_ACTION) {
         //state = ~act.status & ACTS_DISABLED && getActionValue(Act(an->act))[0]=='1';
         //ua.id = CFG_UIDOCKHIDE | IMIB_CFG;
         //ActionStatus(ua , state?0:ACTS_DISABLED);
         }
       break;
       case CFG_UIMSGFROMOTHER:{
         if (an->code != ACTN_CREATE && an->code != ACTN_STATUS && an->code != ACTN_ACTION) break;
		 bool state = getActionValue(Act(an->act)) == "1";
         ua.id = IMIA_CFG_UIM_MSG_NIL_PASS | IMIB_CFG;
         ActionStatus(ua , state?ACTS_DISABLED:0);
         ua.id = IMIA_CFG_UIM_MSG_NIL_REPLY | IMIB_CFG;
         ActionStatus(ua , state?ACTS_DISABLED:0);
         break;}


       case IMIA_CFG_UICLIP_INFO: case IMIA_CFG_UICLIP_EMAIL: case IMIA_CFG_UICLIP_PHONE: {
           int offset = an->act.id & 0xF;
           sUIActionNotify_buff * ab = static_cast<sUIActionNotify_buff *>(anBase);
           if (ab->code == ACTN_GET) {
               itoa((atoi(ab->buff) & (3<<(offset*2)))>>(offset*2) , (char*)ab->buff , 10);
           } else if (ab->code == ACTN_SET) {
               static int set;
               if (!offset) set = 0;
               set |= atoi(ab->buff) <<(offset*2);
               if (offset == 2) {
                   ab->buff = (char*)Ctrl->GetTempBuffer(32);
                   itoa(set , ab->buff , 10);
               } else ab->buff = 0;
           }
           break;}

       case CFG_UIF_CLIST:
           if (an->code==ACTN_SHOW || an->code==ACTN_CHECK) {
               LOGFONTEX lfe  = StrToLogFont(UIActionCfgGetValue(an->act , 0 , 0));
               LOGFONTEX lfe2 = StrToLogFont(UIActionCfgGetValue(sUIAction(an->act.parent , IMIB_CFG|CFG_UIF_CLISTSEL) , 0 , 0));
               lfe.bgColor = lfe2.bgColor;
               lfe.color = lfe2.color;
               lfe.lf.lfItalic = lfe2.lf.lfItalic;
               lfe.lf.lfWeight = lfe2.lf.lfWeight;
               UIActionCfgSetValue(sUIAction(an->act.parent , IMIB_CFG|CFG_UIF_CLISTSEL) , LogFontToStr(lfe).c_str());
           }
           break;
	   case CFG_UIAUTOSTARTASKFORPROFILE: 
		   if (an->code == ACTN_GET) {
			    sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
				strncpy(anb->buff, Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTARTASKFORPROFILE), anb->buffSize);
		   } else if (an->code == ACTN_SET) {
			   sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
			   if (strcmp(anb->buff,Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTARTASKFORPROFILE))) {
				   Ctrl->DTsetStr(DTCFG, 0, CFG_UIAUTOSTARTASKFORPROFILE, anb->buff);
				   // ustawiamy na -1, ¿eby zapisa³ na nowo wartoœæ...
				   Ctrl->DTsetInt(DTCFG, 0, CFG_UIAUTOSTART, -1);

			   }
		   }
		   break;
	   case CFG_UIAUTOSTARTAPPLIESTO: 
		   if (an->code == ACTN_GET) {
			    sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
				strncpy(anb->buff, Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTARTAPPLIESTO), anb->buffSize);
		   } else if (an->code == ACTN_SET) {
			   sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
			   if (strcmp(anb->buff,Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTARTAPPLIESTO))) {
				   // usuwamy poprzedni wpis...
				   HKEY hKey=HKEY_CURRENT_USER;
				   if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
						RegDeleteValue(hKey , getAutostartKey());
						RegCloseKey(hKey);
				   }

				   Ctrl->DTsetStr(DTCFG, 0, CFG_UIAUTOSTARTAPPLIESTO, anb->buff);
				   // ustawiamy na -1, ¿eby zapisa³ na nowo wartoœæ...
				   Ctrl->DTsetInt(DTCFG, 0, CFG_UIAUTOSTART, -1);

			   }
		   }
		   break;
	   case CFG_UIAUTOSTART: 
		   if (an->code == ACTN_GET) {
			   sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
				HKEY hKey=HKEY_CURRENT_USER;
				/* dla autostartów trzeba to sprawdziæ... */
				if (Ctrl->DTgetInt(DTCFG, 0, CFG_UIAUTOSTART) == 1 && !RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
					if (RegQuerySZ(hKey,getAutostartKey()).empty() == true) {
						/* skoro nie istnieje wpis w autostarcie - dane siê NIE zgadzaj¹... */
						strncpy(anb->buff, "0", anb->buffSize);
						Ctrl->DTsetInt(DTCFG, 0, CFG_UIAUTOSTART, 0);
					}
					RegCloseKey(hKey);
				}
				strncpy(anb->buff, Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTART), anb->buffSize);
		   } else if (an->code == ACTN_SET) {
			   sUIActionNotify_buff * anb = (sUIActionNotify_buff*) anBase;
			   if (strcmp(anb->buff,Ctrl->DTgetStr(DTCFG, 0, CFG_UIAUTOSTART))) {
				   int autostart = atoi(anb->buff);
				   Ctrl->DTsetInt(DTCFG, 0, CFG_UIAUTOSTART, autostart);
				   HKEY hKey=HKEY_CURRENT_USER;
				   if (!RegOpenKeyEx(hKey,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)) {
						if (autostart == 1) {
							RegSetSZ(hKey,getAutostartKey(), getAutostartValue());
						} else {
							RegDeleteValue(hKey , getAutostartKey());
						}
						RegCloseKey(hKey);
				   }
			   }
		   }
		   break;
	   case CFG_UIUSEMAINALPHA:
		   if (an->code == ACTN_CREATE || an->code == ACTN_ACTION) {
		//	   UIActionSetStatus(sUIAction(an->act.parent, IMIB_CFG | CFG_UIMAINALPHA), (UIActionCfgGetValue(an->act, 0, 0)[0] == '1') ? 0 : -1, ACTS_DISABLED);
		   }
		   break;

       case 8000: case 8001:
           if (an->code == ACTN_DROP) {
               sUIActionInfo ai;
               ai.act = an->act;
               ai.mask = UIAIM_TXT;
               ai.txt = (char*)_sprintf("%d\nTekst\n%d" , rand() , rand());
               ActionSet(ai);
           }
           break;
       case 8002:{
           ACTIONONLY(an);
           sUIActionInfo ai;
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9000);
           ai.txt = TLS().buff;
           ai.txtSize = MAX_STRING;
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           string s = "9000 = [" + string(ai.txt);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9001);
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           s += "]\n9001 = [" + string(ai.txt);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9002);
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           s += "]\n9002 = [" + string(ai.txt);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9003);
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           s += "]\n9003 = [" + string(ai.txt);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 8000|IMIB_CFG);
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           s += "]\n8000 = [" + string(ai.txt);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 8001|IMIB_CFG);
           ICMessage(IMI_ACTION_GETVALUE , (int)&ai);
           s += "]\n8001 = [" + string(ai.txt) + "]";
           MessageBox(hwndMain , s.c_str() , "8002" , 0);
           break;}
       case 8003:{
           ACTIONONLY(an);
           sUIActionInfo ai;
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9000);
           ai.txt = "SetEM!";
           ICMessage(IMI_ACTION_SETVALUE , (int)&ai);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9001);
           ai.txt = "Blu";
           ICMessage(IMI_ACTION_SETVALUE , (int)&ai);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9002);
           ai.txt = "1";
           ICMessage(IMI_ACTION_SETVALUE , (int)&ai);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9003);
           ai.txt = "50";
           ICMessage(IMI_ACTION_SETVALUE , (int)&ai);
           ai.act = sUIAction(IMIG_CFG_CONNECTION , 9003);
           ai.txt = "Z£Y!";
           ICMessage(IMI_ACTION_SETVALUE , (int)&ai);
           break;}
    }
    return 0;
}


// ------------------------------------

int ActionCntProc(sUIActionNotify_base * anBase) {
    sUIActionNotify_2params * an = static_cast<sUIActionNotify_2params*>(anBase);
    sUIActionInfo ai;
    sUIAction act = an->act;
    switch (an->act.id & ~IMIB_) {
		case CNT_NET: {
            if (an->code != ACTN_ACTION && an->code != ACTN_SHOW) return 0;
            String v = getActionValue(Act(an->act));
            ai.act = sUIAction(an->act.parent , IMIB_CNT|CNT_UID , an->act.cnt);
            ai.mask = UIAIM_STATUS;
			ai.status = (v != "0") ?0:ACTS_DISABLED;
            ai.statusMask = ACTS_DISABLED;
            ActionSet(ai);
			break;}


        case CNT_DISPLAY: {
            if (an->code != ACTN_DROP && an->code != ACTN_CREATE) return 0;
            string s = InfoGetDisplay(act.cnt);
            ai.act = an->act;
            ai.mask = UIAIM_TXT;
            ai.txt = (char*)s.c_str();
            ActionSet(ai);
            break;}
        case CNT_GROUP: {
            if (an->code == ACTN_CREATE) {
                ai.act = an->act;
                ai.mask = UIAIM_STATUS;
                ai.statusMask = ACTS_HIDDEN;
                ai.status = Ctrl->DTgetPos(DTCNT , an->act.cnt)==0?ACTS_HIDDEN:0;
                ActionSet(ai);
            }
            if (an->code != ACTN_DROP && an->code != ACTN_CREATE) return 0;
            string s = "\n";
			char * grps2 = strdup(getCfgString(CFG_GROUPS).a_str()) , * grp = grps2 , * grps = grps2;
            int i = 0;
            while ((grps = strchr(grps , '\n'))!=0) {
                *grps = 0;
                grps++;
                if (*grp) {
                    s+=grp + string("\n");
                }
                grp = grps;
            }
            free(grps2);

            ai.act = an->act;
            ai.mask = UIAIM_TXT;
            ai.txt = (char*)s.c_str();
            ActionSet(ai);
            break;}
        case CNT_BORN:
            switch (an->code) {
                case ACTN_CONVERT_TO:{
					sUIActionNotify_buff * ab = static_cast<sUIActionNotify_buff *>(anBase);
					int born = atoi(ab->buff);
                    cDate64 time;
                    if (!born) {strcpy(ab->buff,"0");break;}
                    time.day = (BYTE)born;
                    time.month = (born & 0xFF00) >> 8;
                    time.year = (born & 0xFFFF0000) >> 16;
					_i64toa((__int64)cTime64(time) , ab->buff , 10);
                    break;}
                case ACTN_CONVERT_FROM: {
					sUIActionNotify_buff * ab = static_cast<sUIActionNotify_buff *>(anBase);
          cDate64 time = an->notify1 ? _atoi64((char*)an->notify1) : 0;
					if (ab->buff) itoa((BYTE)time.day | (BYTE)time.month << 8 | (WORD)time.year << 16 , ab->buff , 10);
                    break;}
                break;
			break;
            }
		case CNT_DESCRIPTION:
			if (an->code == ACTN_CREATE)
				UIActionSetStatus(an->act , (Ctrl->DTgetPos(DTCNT , an->act.cnt)!=0) ? -1 : 0 , ACTSEDIT_READONLY);
			break;
        case IMIA_NFO_DETAILS_NET:
            if (an->code != ACTN_CREATE) return 0;
            ai.act = an->act;
            ai.mask = UIAIM_STATUS;
            ai.statusMask = ACTS_HIDDEN;
            ai.status = Ctrl->DTgetPos(DTCNT , an->act.cnt)==0?ACTS_HIDDEN:0;
            ActionSet(ai);
            break;
    }
    return 0;
}
