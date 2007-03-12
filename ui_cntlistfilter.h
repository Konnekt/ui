/*

    KONNEKT UI

	Filtrowanie listy kontaktów

*/

#pragma once
namespace Konnekt {
namespace UI {
	namespace ACT {
		const int Gfilters = IMIG_MAIN_FILTERS;
	};

	namespace CntList {

		enum enFilterResult {
			frNoApply, // filtr siê nim nie zajmuje...
			frApply, // w zale¿noœci od ustawienia filtra
            frHide, // ukryj i przerwij
			frShow, // pokaz i przerwij
		};

		class cFilter {
		public:
			cFilter(int ico=0 , const CStdString & cfgID="" , const CStdString & name="" , bool enabled = true , bool hidden = false) {
				this->enabled = enabled;
				this->hidden = hidden;
				this->ico = ico;
				this->name = name;
				static int lastID = 0;
				this->id = lastID ++;
				this->cfgID = cfgID;
			}
			virtual enFilterResult Apply(const sUICnt * cnt);
			virtual ~cFilter() {}
			virtual void SetEnabled(bool enabled);
			void Prepare();
			bool IsEnabled() {return enabled;}
		protected:
			unsigned enabled :1;
			unsigned hidden  :1;
			unsigned id:4;
			CStdString name;
			CStdString cfgID;
			int ico;
			friend class cFilters;
		};
		class cFilter_group: public cFilter {
		public:
			cFilter_group():cFilter(0 , "" , "" , true , true) {}
			enFilterResult Apply(const sUICnt * cnt);
		};
		class cFilter_enabled: public cFilter {
		public:
			cFilter_enabled():cFilter(IDI_CLFILTER_ON , "__ON" , "Filtruj" , false) {}
			void SetEnabled(bool enabled);
			enFilterResult Apply(const sUICnt * cnt);
		};
		class cFilter_offline: public cFilter {
		public:
			cFilter_offline():cFilter(UIIcon(IT_STATUS , 0 , ST_OFFLINE , 0) /*IDI_CLFILTER_OFFLINE*/ , "__OFFLINE" , "Kontakty offline",false) {}
			enFilterResult Apply(const sUICnt * cnt);
		};
		class cFilter_net: public cFilter {
		public:
			cFilter_net(int net);
			enFilterResult Apply(const sUICnt * cnt);
		protected:
			int net;
		};

		class cFilters: private deque<cFilter*> {
		public:
			cFilters();
			~cFilters();
			bool CheckCnt(const sUICnt * cnt); // Zwraca czy kontakt mo¿e byæ wyœwietlony, czy nie
			bool ApplyToCnt(sUICnt * cnt); // Ukrywa/pokazuje kontakt
			void Prepare(); // Tworzy wszystkie akcje, generuje filtry
			int ActionProc(sUIActionNotify_base * anBase);
		private:
			int switchIndex;
		};
		extern cFilters filters;
	}; // ns CntList

}; // ns UI
}; // ns Konnekt