#include "stdafx.h"
#pragma hdrstop

#include "ui_main.h"
#include "checkComposing.h"
using namespace Konnekt::UI::Cnt;

cCheckComposing Konnekt::UI::Cnt::checkComposing;

cCheckComposing::cCheckComposing() {
	this->timer = CreateWaitableTimer(0 , 0 , 0);
	timerWaiting = false;
}
cCheckComposing::~cCheckComposing() {
    CloseHandle(this->timer);	
}
void cCheckComposing::composing(int cntID) {
	cCheckComposing::const_iterator find = this->find(cntID);
	if (find == this->end()) {
		IMDEBUG(DBG_MISC , "- composing cnt=%d" , cntID);
    IMessage(IM_CNT_COMPOSING , Net::broadcast , imtContact, cntID);
	}
	(*this) [cntID] = time(0);
	checkTimer();
}
void cCheckComposing::stopComposing(int cntID) {
	cCheckComposing::iterator find = this->find(cntID);
	if (find != this->end()) {
		stopComposing(find);
	}
}
void cCheckComposing::stopComposing(iterator cnt) {
	IMessage(IM_CNT_COMPOSING_STOP , Net::broadcast , imtContact , cnt->first);
	IMDEBUG(DBG_MISC , "- stopComposing cnt=%d" , cnt->first);
	this->erase(cnt);
	checkTimer();
}

bool cCheckComposing::isComposing(int cntID) {
	return this->find(cntID) != this->end();
}

void cCheckComposing::checkTimer() {
	if (this->size() == 0) {
		CancelWaitableTimer(timer);
		timerWaiting = false;
	} else if (!timerWaiting) {
		LARGE_INTEGER lDueTime;
		lDueTime.QuadPart = -10000 * 10000;
		SetWaitableTimer(timer , &lDueTime , 10000,(PTIMERAPCROUTINE)timerProc, this , FALSE);
		timerWaiting = true;
	}
}

VOID CALLBACK  cCheckComposing::timerProc(cCheckComposing * ct , DWORD low , DWORD high) {
	time_t current = time(0);
	cCheckComposing::iterator it = ct->begin();
	while (it != ct->end()) {
		if ((current - it->second) > 15 /*sekund*/) {
			ct->stopComposing(it++);
		} else
			it++;
	}
}


