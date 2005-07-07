

CdTable ModDT;



cUIActMod::cUIActMod(){
    ModDT.cols = CdtColDesc(11
                            ,AMOD_ACTION , DT_CT_64 , 0 , "Action"
                            ,AMOD_MASK , DT_CT_INT , 0 , "Mask"
                            ,AMOD_STATUS_AND , DT_CT_INT , 0 , "StatusAnd"
                            ,AMOD_STATUS_OR , DT_CT_INT , 0 , "StatusOr"
                            ,AMOD_STYLE_AND , DT_CT_INT , 0 , "StyleAnd"
                            ,AMOD_STYLE_OR , DT_CT_INT , 0 , "StyleOr"
                            ,AMOD_POS , DT_CT_INT , 0 , "Pos"
                            ,AMOD_TITLE , DT_CT_PCHAR , "" , "Title"
                            ,AMOD_P1 , DT_CT_INT , 0 , "P1"
                            ,AMOD_XY , DT_CT_INT , 0 , "XY"
                            ,AMOD_WH , DT_CT_INT , 0 , "WH"
                            );
    this->DT = &ModDT;
}


void cUIActMod::load(){ // Laduje ustawienia do pamieci
    CdtFileBin FBin;
    FBin.assign(DT);
    FBin.load(profileDir + "acts.dtb");

}
void cUIActMod::save(){ // Zapisuje ust. do pliku
    CdtFileBin FBin;
    FBin.assign(DT);
    FBin.save(profileDir + "acts.dtb");
}

int cUIActMod::createEntry(cUIAction & act) {
    __int64 actID = act.id | ((__int64)act.parent << 32);
    int row = DT->findby((TdEntry)&actID , AMOD_ACTION);
    if (row==-1) {
        row = DT->addrow();
        DT->set64(row , AMOD_ACTION , actID);
    }
    return row;
}

int cUIActMod::getEntry(cUIAction & act) {
    __int64 actID = act.id | ((__int64)act.parent << 32);
    return DT->findby((TdEntry)&actID , AMOD_ACTION);
}


void cUIActMod::loadWindow(cUIGroup & grp){ // ustawia okno
    int row = getEntry(grp);
    if (row == -1 || !grp.ghandle) return;
    int mask = DT->getint(row , AMOD_MASK);
    if (mask & (MODM_XY | MODM_WH)) {
        RECT nrc; // nowy rozmiar
        nrc.left = mask & MODM_XY ? (short)LOWORD(DT->getint(row , AMOD_XY)): 0;
        nrc.top = mask & MODM_XY ? (short)HIWORD(DT->getint(row , AMOD_XY)): 0;
        nrc.right = nrc.left + (mask & MODM_WH ? LOWORD(DT->getint(row , AMOD_WH)): 0);
        nrc.bottom = nrc.top + (mask & MODM_WH ? HIWORD(DT->getint(row , AMOD_WH)): 0);
        RECT wa;
        SystemParametersInfo(SPI_GETWORKAREA,0,&wa,0);
        // Sprawdzamy czy nowe okienko zmieœci siê w ca³oœci na ekranie
        nrc.left = max(nrc.left , wa.left);
        nrc.left = min(nrc.left , wa.right);
        nrc.right = min(nrc.right , wa.right) & 0xFFFF;
        nrc.top = max(nrc.top , wa.top);
        nrc.top = min(nrc.top , wa.bottom - 50);
        nrc.bottom = min(nrc.bottom , wa.bottom) & 0xFFFF;

        // Sprawdzamy, czy nowy rozmiar nie jest za maly...
        SendMessage((HWND)grp.ghandle , WM_SIZING , WMSZ_BOTTOM , (LPARAM)&nrc);

        SetWindowPos((HWND)grp.ghandle , 0 , nrc.left , nrc.top , nrc.right - nrc.left , nrc.bottom - nrc.top , SWP_NOACTIVATE | SWP_NOZORDER 
            | (mask & MODM_XY?0:SWP_NOMOVE) | (mask & MODM_WH?0:SWP_NOSIZE));
    }
}
void cUIActMod::saveWindow(cUIGroup & grp){ // zapamietuje okno
    RECT rc = {0,0,0,0};
    if (!IsIconic((HWND)grp.ghandle) && !IsZoomed((HWND)grp.ghandle) && GetWindowRect((HWND)grp.ghandle , &rc)) {
        int row = createEntry(grp);
        DT->setint(row , AMOD_MASK , DT->getint(row , AMOD_MASK) | MODM_XY | MODM_WH);
        DT->setint(row , AMOD_XY , MAKELPARAM(rc.left , rc.top));
        DT->setint(row , AMOD_WH , MAKELPARAM(rc.right - rc.left , rc.bottom - rc.top));
    }
}
void cUIActMod::loadAction(cUIAction & act){ // szykuje caly toolbar
    int row = getEntry(act);
    if (row == -1) return;
    int mask = DT->getint(row , AMOD_MASK);
    if (mask & MODM_STATUS) {
        act.status &= ~DT->getint(row , AMOD_STATUS_AND);
        act.status |= DT->getint(row , AMOD_STATUS_OR);
    }
    if (act.type & ACT_ISGROUP) {
        // Przeszukujemy liste, zeby poprzestawiac jego dzieci
        cUIGroup & grp = static_cast<cUIGroup &>(act);
        if (grp.Act.size() < 2) return;
        t_rows_it row = DT->rows.begin();
        int found = 0;
        while (row != DT->rows.end() && found < grp.size()) {
            if ((*(__int64*)(*row)->get(AMOD_ACTION))>>32 == grp.id) {
                if ((int)(*row)->get(AMOD_MASK) & MODM_POS) {
                    unsigned int newPos = (unsigned int)(*row)->get(AMOD_POS);
                    int pos = grp.find((int)(*(__int64*)(*row)->get(AMOD_ACTION)));
                    if (pos >= 0) {
                        cUIAction * tmp = &grp[pos];
                        grp.Act.erase(grp.Act.begin() + pos);
                        if (newPos >= (unsigned int)grp.size()) 
                            grp.Act.push_back(tmp);
                        else
                            grp.Act.insert(grp.Act.begin() + newPos , tmp);
                    }
                }
                found++;
            }
            row++;
        }
    }


}

void cUIActMod::changeStatus(cUIAction & act , int mask , int value){
    value &= mask;
    sUIActionInfo ai;
    ai.mask = UIAIM_STATUS;
    ai.statusMask = mask;
    ai.status = value;
    act.setInfo(ai , true);
    int row = Act.Mod.createEntry(act);
    Act.Mod.DT->setint(row , AMOD_MASK , Act.Mod.DT->getint(row , AMOD_MASK) | MODM_STATUS);
    Act.Mod.DT->setint(row , AMOD_STATUS_AND , Act.Mod.DT->getint(row , AMOD_STATUS_AND) | mask);
    Act.Mod.DT->setint(row , AMOD_STATUS_OR , (Act.Mod.DT->getint(row , AMOD_STATUS_OR) & ~mask) | (value));
}


void cUIActMod::loadBar(cUIGroup & grp){ // ustawia caly toolbar

}
void cUIActMod::saveBar(cUIGroup & grp){ // zapisuje caly toolbar
    HWND hwnd = (HWND)grp.ghandle;
    int c = SendMessage(hwnd , RB_GETBANDCOUNT , 0 , 0);
    for (int i=0; i<c; i++) {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_SIZE | RBBIM_LPARAM | RBBIM_STYLE;
        rbbi.lParam = 0;
        SendMessage(hwnd , RB_GETBANDINFO , i , (LPARAM)&rbbi);
        if (rbbi.fStyle & RBBS_HIDDEN) continue;
        cUIAction * act = (cUIAction*)rbbi.lParam;
        if (!act) continue;
        int row = createEntry(*act);
        DT->setint(row , AMOD_MASK , DT->getint(row , AMOD_MASK)
            | MODM_POS | MODM_STATUS | MODM_WH);
        DT->setint(row , AMOD_STATUS_AND , DT->getint(row , AMOD_STATUS_AND)
            | ACTSBAND_WRAP);
        DT->setint(row , AMOD_STATUS_OR , (DT->getint(row , AMOD_STATUS_OR) & ~ACTSBAND_WRAP)
            | (rbbi.fStyle & RBBS_BREAK ? ACTSBAND_WRAP:0));
        DT->setint(row , AMOD_POS , i);
        DT->setint(row , AMOD_WH , MAKELPARAM(rbbi.cx , rbbi.cyChild));
    }
}