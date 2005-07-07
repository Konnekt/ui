#pragma once


namespace Konnekt {
namespace UI {
namespace Cnt {
	class cCheckComposing: private map<int , time_t> {
	public:
		~cCheckComposing();
		cCheckComposing();
		void composing(int cntID);
		void stopComposing(int cntID);
		void stopComposing(iterator cnt);
		bool isComposing(int cntID);
	private:
		HANDLE timer;
		bool timerWaiting;
		static VOID CALLBACK  timerProc(cCheckComposing * ct , DWORD low , DWORD high);
		void checkTimer();
	};
	extern cCheckComposing checkComposing;
}; // ns Cnt
}; // ns UI
}; // ns Konnekt