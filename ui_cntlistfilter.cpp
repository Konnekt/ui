/*

  Konnekt UI

  Filtrowanie listy kontaktów

*/


#include "stdafx.h"
#pragma hdrstop
#include "ui_main.h"
#include "ui_cntlist.h"
#include "ui_actions.h"

#include "ui_cntlistfilter.h"

using namespace Konnekt;
using namespace UI;
using namespace Konnekt::UI::CntList;

cFilters Konnekt::UI::CntList::filters;

void cFilter::Prepare() {
	if (!cfgID.empty()) {
		CStdString setting = GetExtParam(GETSTR(CFG_UIFILTERS) , this->cfgID);
		if (setting == "1") 
			enabled = true;
		else if (setting == "0")
			enabled = false;
	}
	if (!hidden)
		UIActionAdd(ACT::Gfilters , id , ACTT_CHECK | (enabled ? ACTS_CHECKED : 0) , this->name , this->ico);
}
enFilterResult cFilter::Apply(const sUICnt * cnt) {
	return this->enabled ? frApply : frNoApply;
}
void cFilter::SetEnabled(bool enabled) {
	this->enabled = enabled;
	if (!cfgID.empty()) {
		SETSTR(CFG_UIFILTERS , SetExtParam(GETSTR(CFG_UIFILTERS) , this->cfgID , enabled ? "1" : "0").c_str());
	}
	if (!hidden)
		UIActionSetStatus(sUIAction(ACT::Gfilters , id) , enabled ? -1 : 0 , ACTS_CHECKED);
}



enFilterResult cFilter_group::Apply(const sUICnt * cnt) {
	return (!cnt->user && !cnt->hidden && ICMessage(IMC_CNT_INGROUP , cnt->ID))? frNoApply : frHide;
}

enFilterResult cFilter_enabled::Apply(const sUICnt * cnt) {
	return enabled ? frNoApply : frShow;
}
void cFilter_enabled::SetEnabled(bool enabled) {
	cFilter::SetEnabled(enabled);
	if (!hidden) {
		sUIActionInfo ai (sUIAction(IMIG_MAINTB , UI::ACT::Gfilters));
		ai.mask = UIAIM_ICO;
		ai.p1 = enabled ? IDI_CLFILTER_ON : IDI_CLFILTER_OFF;
		UIActionSet(ai);
	}
}


enFilterResult cFilter_offline::Apply(const sUICnt * cnt) {
	return (cnt->net > 0 && !cnt->active && (cnt->status & ST_MASK) == ST_OFFLINE) ? frApply : frNoApply;
}

cFilter_net::cFilter_net(int net) {
	this->net = net;
	char * netName = (char*)IMessage(IM_PLUG_NETNAME , (Net::tNet)net , IMT_NETUID);
	if (!netName || !*netName) {
		this->name = "Sieæ nr ";
		this->name += inttoch(net);
	} else {
		this->name = netName;
	}
	this->cfgID = this->name;
	this->ico = UIIcon(IT_LOGO , net , 0 , 0);
}
enFilterResult cFilter_net::Apply(const sUICnt * cnt) {
	return (cnt->net == this->net) ? frApply : frNoApply;
}




// -------------------------------------------------------
cFilters::cFilters() {
}
cFilters::~cFilters() {
	// po dodaniu trzeba je zgrabnie pousuwaæ
	for (cFilters::iterator it = this->begin(); it != this->end(); it++)
		delete *it;
	this->clear();
}
bool cFilters::CheckCnt(const sUICnt * cnt) {
	if (!cnt) return false;
	bool applied = false;
	for (cFilters::iterator it = this->begin(); it != this->end(); it++) {
		switch ((*it)->Apply(cnt)) {
			case frHide: return false;
			case frShow: return true;
			case frApply: 
				if ((*it)->IsEnabled()) {
					applied = true;
					break;
				} else 
					return false;
		}
	}
	return applied;
}
bool cFilters::ApplyToCnt(sUICnt * cnt) {
	bool show = CheckCnt(cnt);
	if (show != cnt->onList) {
		if (show)
			cntList.addCnt(cnt);
		else
			cntList.removeCnt(cnt);
		return true;
	}
	return false;
}
void cFilters::Prepare() {
	Ico.iconRes(IDI_CLFILTER_OFF, 0, 0, IML_16_32);
	Ico.iconRes(IDI_CLFILTER_ON, 0, 0, IML_16_32);
	CFG::enGroupType gtype = (CFG::enGroupType)GETINT(CFG_UIFILTERACTION);
	if (gtype == CFG::gtypeMenu) {
		CStdString txt = "Filtry";
		if (ShowBits::checkBits(ShowBits::showTooltipsBeginner)) {
			txt += AP_TIP "Filtrowanie listy kontaktów" AP_TIP_WIDTH "70";
			txt += AP_TIPICONURL "reg://IML_32/" + inttostr(IDI_CLFILTER_ON) + ".ico";
		}

		UIGroupAdd(IMIG_MAINTB , IMIG_MAIN_FILTERS , ACTSMENU_CHECKSPACE , txt , IDI_CLFILTER_OFF);
	} else if (gtype == CFG::gtypeToolbar)
		UIGroupAdd(IMIG_BAR , IMIG_MAIN_FILTERS , ACTSTB_LIST , "Filtry" , IDI_CLFILTER_OFF);

	push_back(new cFilter_group());
	switchIndex = size();
	push_back(new cFilter_enabled());
	push_back(new cFilter_offline());
    int c = IMessage(IMC_PLUG_COUNT);
    for (int i=0;i<c;i++) {
		oPlugin plugin = Ctrl->getPlugin((tPluginId)i);
		if (plugin->getType() & IMT_NETUID) {
			push_back(new cFilter_net(plugin->getNet()));
        }
    }
	push_back(new cFilter(ICON_USER , "__OTH" , "Pozosta³e" , false));
	for (cFilters::iterator it = this->begin(); it != this->end(); it++) {
		if (gtype == CFG::gtypeHidden) {
			(*it)->hidden = true;
			(*it)->cfgID = "";
		}
		(*it)->Prepare();
	}
	if (gtype == CFG::gtypeHidden)
		this->at(1)->SetEnabled(false);
	this->at(1)->SetEnabled(this->at(1)->IsEnabled()); // ¿eby ustawi³ ikonkê grupy :)
}
int cFilters::ActionProc(sUIActionNotify_base * anBase) {
	if (anBase->code != ACTN_ACTION) return 0;
	int id = anBase->act.id;
	if (anBase->act.id == ACT::Gfilters) 
		id = switchIndex; // 0 to groups, switchIndex to "w³¹cznik"
	if (id < 0 || id >= (int)this->size()) return 0;
	if ((GetKeyState(VK_SHIFT)&0x80) || (GetKeyState(VK_CONTROL)&0x80)) {
		bool set = (GetKeyState(VK_SHIFT)&0x80) ? !this->at(id)->IsEnabled() : true;
		this->at(switchIndex)->SetEnabled(true);
		// prze³¹czamy wszystkie pozosta³e na odwrotnoœæ set
		for (unsigned int i=switchIndex+1; i < this->size(); i++) {
			if (i == id) continue;
			this->at(i)->SetEnabled(!set);
		}
		this->at(id)->SetEnabled(set);
	} else {
		this->at(id)->SetEnabled(!this->at(id)->IsEnabled());
	}
	cntList.refill();
	return 0;
}
