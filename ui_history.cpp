/*

  Konnekt UI

  Obs³uga historii.

*/

#include "stdafx.h"
#pragma hdrstop

#include <io.h>
#include <fstream>
#include <Richedit.h>
#include "include\func.h"
#include "include\simxml.h"
#include "include\dbtable.h"
#include "include\dtablebin.h"
#include "include\time64.h"

#include "ui_main.h"


#include "resource_ui.h"
#include "include\win_listview.h"

#include "ui_actions.h"
#include "ui_cntlist.h"
#include "ui_history.h"
#include "ui_msgcontrol.h"

CdtColDesc h_desc;


string historyDir;

#define HDT_DIR   0
#define HDT_DTB   1
#define HDT_QUEUE 2
#define HDT_SEARCH 3

#define HDT_ITEM  0x80


enum enReturn {
	retFalse = 0 , retTrue = 1 , retCancel = 2
};

  struct cHistHeader {
    string id;
    int width;
    string name;
    string fmt;
    int index;
    cHistHeader() {width=0;name="";index=-1;}
  };


class cHist {
  public:
  int type;
  string path; // sciezka do pliku
  string dir;
  string info;
  string name;
  int cnt; // spokrewniony kontakt
  int listState; // stan elementu na liœcie...
  unsigned int size; // rozmiar w b
  unsigned subItems; // ilosc podelementow
  unsigned __int64 sort; // wartosc wg. ktorej sortuje
  int ico;
  virtual void open() = 0; // wypisuje element
  virtual void prepare() = 0; // otwiera element i wywo³uje open
  virtual void select() {prepare();} // zaznacza element i wywo³uje prepare
  virtual enReturn erase(){return retFalse;}; // usuwa element
  virtual bool canErase(){return false;};
  virtual enReturn save(ofstream & file , int type){return retFalse;}; // zapisuje element
  virtual bool saveBefore() {return false;} // zanim wybierzemy plik... true przerwie zapisywanie...
  const static char saveXML = 2;
  const static char saveTXT = 1;
  virtual bool canSave(){return false;};
  virtual bool canSearch() {return true;}
  virtual bool search() {return false;} // przeszukuje w g³¹b
  virtual void compact(){};
  virtual void listRepaint(); // odœwie¿a element na liœcie
  virtual void listSetState(int newState);
  virtual int listGetState();
  virtual void listPushBack(); // wstawia na listê
  virtual const CStdString getColumnString(const cHistHeader * col);
  virtual int count() {return subItems;}
  virtual class cHistDir * getParent() {return 0;}

  cHist () {sort = 0; ico=0; cnt=-1;subItems=0;listState=0;size=0;}
};


class cHistHeaders {
  public:
  void load (const char * src); // wczytuje kolumny
  int pos (string hdr);
  vector <cHistHeader> hdrs;
};
typedef vector <cHistHeader>::iterator hdrs_it_t;

class cHistItem : public cHist {
  public:
  unsigned int pos; // pozycja (w bajtach) w pliku lub ID w Queue
  unsigned int id;
  string title;
  string first_uid;
  cTime64 date;
  class cHistDir * parent;
  cHistItem() {type = HDT_ITEM;}
  virtual void open() {}
  virtual void prepare();
  virtual void remove() {}// Usuwa zaznaczone elementy
  void select(); // zaznacza element i wywo³uje prepare
  const CStdString getColumnString(const cHistHeader * col);
  cHistDir * getParent() {return parent;}

};

class cHistItemDTB : public cHistItem {
	public:
	unsigned int limit;
	bool wholeThread;
	void open();
	bool canErase() {return true;}
	bool canSave() {return true;}
	enReturn erase();
	enReturn save(ofstream &file , int type);
	cHistItemDTB() {limit = 0x7FFFFFFF; wholeThread = false;}
};

class cHistItemQueue : public cHistItem {
  public:
  void open();
  bool canErase() {return true;}
  enReturn erase();
};


class cHistDir : public cHist {
  public:
  HTREEITEM treeItem;
  string when_empty;
  string list;
  vector <cHistItem*> items;
  typedef  vector <cHistItem*>::iterator items_it;

  cHistHeaders * headers , * dirheaders;
  bool headers_created;
  bool searchDisabled;
  cHistDir * parent;
  int sortColumn; // kolumna wg. ktorej sortuje (od 1) . 0 - wg. sort
  cHistDir(cHistDir * par) {sortColumn = 0; type = HDT_DIR;headers = dirheaders = 0;parent = par;headers_created=false; searchDisabled = false;}
  ~cHistDir();
  void prepare(); // laduje naglowki
  void open(); // otwiera katalog
  virtual enReturn save(ofstream &file , int type);
  int count() {return items.size();}
  virtual void fill();
  int itemsClear();
  bool search();
  bool canErase() {return false;}
  bool canSave() {return true;}
  bool canSearch() {return !searchDisabled;}
  virtual void msgInsert(cMessage * m , const char * display , bool scroll); // dodaje wiadomoœæ do okna ...
  void select(); // zaznacza element i wywo³uje prepare
  cHistDir * getParent() {return parent;}

};
typedef  cHistDir::items_it  items_it_t;

class cHistDirDTB : public cHistDir {
  public:
  cHistDirDTB(cHistDir * par):cHistDir(par) {type = HDT_DTB;parent = par;}
  void open();
  enReturn erase();
  bool canErase() {return 1;}
  void compact();
  bool search();
};

class cHistDirQueue : public cHistDir {
  public:
  int net , type , flag , noflag;
  cHistDirQueue(cHistDir * par):cHistDir(par) {type = HDT_QUEUE;parent = par;}
  void open();
  int count();
  int checkMsg(int pos);
  enReturn erase();
  bool search();
  bool canSave() {return false;}
};

struct cHistSearchOptions {
	CStdString query;
	bool useCase;
	bool words;
	bool findAll;
	bool onlyCurrent;
	bool regexp;
	signed int threadPrev; // ile wczeœniejszych -1 - ca³oœæ
	signed int threadNext; // ile póŸniejszych.  -1 - ca³oœæ
	__time64_t timeFrom, timeTo;
	void GetFromDialog(HWND wnd);
	void SetInDialog(HWND wnd);
	void SetHistoryWindow(HWND wnd);
	cHistDir * currentOpen;
	CStdString GetName();
	CStdString GetInfo();
	void Load(cXML & xml);
	bool const operator == (const cHistSearchOptions & b) {
		return query == b.query && useCase == b.useCase 
			&& words == b.words /*&& findAll == b.findAll*/
			&& onlyCurrent == b.onlyCurrent && regexp == b.regexp
			&& (!currentOpen || !b.currentOpen || currentOpen == b.currentOpen)
			&& timeFrom == b.timeFrom && timeTo == b.timeTo
			/*&& threadPrev == b.threadPrev && threadNext == b.threadNext*/
			;
	}
	cHistSearchOptions() {currentOpen = 0; }
};

class cHistDirSearch : public cHistDir {
  public:
  cHistSearchOptions opt;
  cPreg preg; // wzorzec do u¿ycia podczas wyœwietlania
  cHistDirSearch(cHistDir * par , cHistSearchOptions & opt):cHistDir(par),opt(opt),preg(0) {type = HDT_SEARCH;parent = par;}
  void open();
  enReturn erase(); // tylko zapytania
  bool saveBefore();

  /* TODO: zapisywanie zapytañ */
  bool canSearch() {return false;}
  bool canErase() {return true;}
  void addFoundItem(cHistItem * item); /* Dodaje do wyników szukania... */
  void msgInsert(cMessage * m , const char * display , bool scroll);
};


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


struct hist_s{
    HIMAGELIST iml;
    class cHist * selected;
    class cHistDir * dir_opened;
    class cHistItem * item_opened;
    class cHistHeaders * headers; // aktualne naglowki
	int selectedList;
    bool DTopened;
    CdTable DT;
    CdtFileBin DTfb;
    bool DTtemp;
	struct search_s {
		class cHistDirSearch * foundDir; // katalog znalezionych
		bool inProgress; // czy wyszukiwanie idzie...
		enum eState{
			stReady , stPreparing , stSearch , stCancel
		} state;
		bool canGetNext; // czy wyszukiwanie idzie...
		cPreg preg; // preg ze wzorcem do znalezienia
		cHistSearchOptions opt;
		void Show();
		void Reset();
		void ResetMRU();
		void Start();
		void Next(); // do nastêpnego wyniku
		void Stop();
		bool IsActive() {return inProgress && foundDir;}
		static LRESULT WndProc(HWND wnd , int message , WPARAM wParam , LPARAM lParam);
		HWND wnd;
		HWND prgrsWnd;
		search_s () {
			state = stReady;
		}
	} search;

    void changeDT(const char * file , bool useTemp = true);
    HWND statusWnd , prgrsWnd , tbWnd , listWnd , hwnd;
    cMsgControl * msgControl;
    bool running;
    vector <class cHistDir*> dirs;
	typedef vector <cHistDir*>::iterator dirs_it;
    vector <class cHist*> itemsOnList; // elementy aktualnie na liœcie
	typedef vector <class cHist*>::iterator itemsOnList_it; // elementy aktualnie na liœcie

	void GetDispInfo(NMLVDISPINFO * disp);
	void SetDispInfo(NMLVDISPINFO * disp);
	void ODStateChanged(NMLVODSTATECHANGE * state);
	void SetStatus(const char * status);

    void start(int cnt=0);
    void end();
    void checkErase();
    void checkSave();
    void compact();
    void eraseSelected();
    void saveSelected();
    void printSelected();
    void listPrepare(cHistHeaders * headers);
    void dialogFill(HWND hwnd);
    void removeFromDirs(cHistDir * dir);
    //  int sortColumn;
	void sort() {listSort();}
    void listSort();
    void init();
    void refresh(bool all = false);
    void shred();
    enReturn erase(cHist * item);
    enReturn save(ofstream & file , cHist * item , int type);
	void listSynchronize();
	void listClear();
	void resetOpenedItem();
    hist_s();
};

hist_s hist;



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------



int hist_add (cMessage * m , const char * dir , sUICnt * cnt , const char * name = 0 , int session = 0 ) {
  if (!cnt && (!dir || !dir[0])) return 0;
  if (!GETINT(CFG_LOGHISTORY)) return 0;
  string path = profileDir + "history\\";
//  IMLOG("HIST %s   ...   %s" , profileDir.c_str() , path.c_str());
  path += dir;
  mkdirs((char*)path.c_str());
  path += "\\";
  string fn;
  if (name) fn = name;
  if (cnt) {
    if (*GETCNTC(cnt->ID , CNT_UID)) fn="u"+GETCNTS(cnt->ID , CNT_UID);
    else if (*GETCNTC(cnt->ID , CNT_CELLPHONE)) fn="p"+GETCNTS(cnt->ID , CNT_CELLPHONE);
	else if (*GETCNTC(cnt->ID , CNT_EMAIL)) fn="e"+GETCNTS(cnt->ID , CNT_EMAIL);
    else if (*GETCNTC(cnt->ID , CNT_DISPLAY)) fn="d"+GETCNTS(cnt->ID , CNT_DISPLAY);
    else fn="_inne";
  }
  fn = urlEncode(fn , '#');
  if (cnt) { // dodajemy jeszcze sieæ...
    fn+=".";
    fn+=inttoch(cnt->net);
  }
  fn+=".dtb";
//  fn = Preg.replace("/[^a-z0-9¹æê³ñóœ¿Ÿ¥ÆÊ£ÑÓŒ¯ _.-]+/ig" , "_" , fn.c_str());
  CdTable Msg;
  Msg.cxor_key = HIST_XOR_KEY;
  Msg.cols = h_desc;

  CdtFileBin fb;
  fb.assign(&Msg);

  Msg.addrow();
  Msg.setint(0 , MSG_ID , m->id);
  Msg.setint(0 , MSG_NET , m->net);
  Msg.setint(0 , MSG_TYPE , m->type);
  Msg.setch(0 , MSG_FROMUID , m->fromUid);
  Msg.setch(0 , MSG_TOUID , m->toUid);
  Msg.setch(0 , MSG_BODY , m->body);
  Msg.setch(0 , MSG_EXT  , m->ext);
  Msg.setint(0 , MSG_FLAG , m->flag);
  Msg.set64(0 , MSG_TIME , m->time);
  Msg.setint(0 , MSGH_SESSION , session);
  if (!*m->fromUid)
    Msg.setch(0 , MSGH_DISPLAY , GETCNTC(0,CNT_DISPLAY));
  else
    {
      int p = ICMessage(IMC_CNT_FIND , m->net , (int)m->fromUid);
      Msg.setch(0 , MSGH_DISPLAY , (p>0)?GETCNTC(p,CNT_DISPLAY) : "");
    }
  fb.append((path + fn).c_str());
  return 1;
}



  int cHistHeaders::pos (string hdr) {
    for (hdrs_it_t hdrs_it=hdrs.begin() ; hdrs_it != hdrs.end(); hdrs_it++)
      if (hdrs_it->id == hdr) return hdrs_it->index;
    return -1;
  }




// **************************************************

void cHist::listRepaint(){ 
	hist_s::itemsOnList_it it = std::find(hist.itemsOnList.begin() , hist.itemsOnList.end() , this);
	if (it == hist.itemsOnList.end()) return;
	int i = it - hist.itemsOnList.begin();
	ListView_RedrawItems(hist.listWnd , i , i);
}
void cHist::listSetState(int newState) {
	listState = newState;
}
int cHist::listGetState() {
	return listState;
}

void cHist::listPushBack() {
	this->listState = 0;
	hist.itemsOnList.push_back(this);
}

const CStdString cHist::getColumnString(const cHistHeader * col) {
	static char buff [10];
	if (col->id == "info") 
		return this->info.empty() ? this->name : this->info;
	else if (col->id == "name" || col->id == "contact")
		return this->name;
	else if (col->id == "count")
		return itoa(this->count() , buff , 10);
	else if (col->id == "parent")
		return this->getParent() ? this->getParent()->name : "-";
	else if (col->id == "size") {
		_snprintf(buff , 10 , "%.0fKB" , (float)this->size / 1024);
		return buff;
	}
	else return "???";
}


// --------------------------- DIR
  int cHistDir::itemsClear() {
    if (!items.size()) return 0;
    for (items_it_t items_it = items.begin(); items_it!=items.end(); items_it++)
      delete *items_it;
    items.clear();
	if (hist.dir_opened == this) {
		hist.listClear();
	}
    return 1;
  }

  cHistDir::~cHistDir() {
    if (headers && headers_created) {delete headers;headers=0;}
    if (dirheaders) {delete dirheaders;}
    itemsClear();
  }

void cHistHeaders::load (const char * src){
  cPreg Preg;
  while (src) {
    Preg.match("/< *([^ >/]+)[^>]*>/i" , src);
    if (!Preg.matched) {break;}
    src += Preg[0].size();
    if (Preg[1]!="") {
      cHistHeader hh;
      hh.id = Preg[1];
      string attr = Preg[0];
      Preg.match("/width *= *\"(\\d+)\"/i" , attr.c_str());
      hh.width = max(16 , Preg.matched?chtoint(Preg[1].c_str()):0);
      Preg.match("/name *= *\"(.+?)\"/i" , attr.c_str());
      hh.name = !Preg.matched?hh.id:Preg[1];
      Preg.match("/format *= *\"(.+?)\"/i" , attr.c_str());
      hh.fmt = !Preg.matched?string(""):Preg[1];
      hdrs.push_back(hh);
    }
    src = strchr(src , '<');
  }
}


enReturn cHistDir::save(ofstream &file , int type) {
	if (type==saveXML) {
		file << "	<" << (this->cnt > 0 ? "contact" : "dir") << " name=\"" << EncodeEntities(this->name) << "\" info=\"" << EncodeEntities(this->info) << "\">" << endl;
	} else if (type==saveTXT) {
		file << "**********************************************************************" << endl;
		file << "* " << this->name.c_str() << endl;
	}
	enReturn ret = retTrue;
	for (hist_s::dirs_it it=hist.dirs.begin(); it != hist.dirs.end(); it++) {
		if ((*it)->parent == this) {
			if ((ret = (*it)->save(file , abs(type))) == retCancel) 
				break;
		}
    }
	if (ret != retCancel) {
		ret = retTrue;
		for (unsigned int i=0; i<this->items.size(); i++) {
			if (this->items[i]->save(file , abs(type)) == retCancel) break;
		}
	}
	if (type==saveXML) {
		file << "	</"<< (this->cnt > 0 ? "contact" : "dir") << ">" << endl;
	}
    return ret;
}

void cHistDir::select() {
	if (hist.dir_opened == this) return;
	TreeView_SelectItem(GetDlgItem(hwndHistory , IDC_TREE) , treeItem);
	cHist::select();
}
void cHistDir::prepare() {
	if (hist.dir_opened == this) return;
	hist.dir_opened = this;
	hist.listPrepare(headers);
	open();
}

void cHistDir::open() {
  if (!headers) return;
  // Wyswietla wszystkie podkatalogi

  HWND titem = GetDlgItem(hwndHistory , IDC_TREE);
  if (list=="ALL") {

    HTREEITEM it = TreeView_GetChild(titem , treeItem);
    SendMessage(hist.prgrsWnd , PBM_SETRANGE , 0 , MAKELPARAM(0 , subItems));
    SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);

    while (it) {
      TVITEMEX ti;
      ti.mask = TVIF_PARAM;
      ti.hItem = it;
      TreeView_GetItem(titem , &ti);
      ((cHist*)ti.lParam)->open();
      it = TreeView_GetNextSibling(titem , it);
      SendMessage(hist.prgrsWnd , PBM_STEPIT , 0 , 0);
    }
    SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);
  // PLIKI
  } else if (list=="FILES") {
/*    HTREEITEM it = TreeView_GetChild(titem , treeItem);
    while (it) {
      TVITEMEX ti;
      ti.mask = TVIF_PARAM | TVIF_IMAGE;
      ti.hItem = it;
      TreeView_GetItem(titem , &ti);
      LVITEM lvi;
      lvi.mask = LVIF_IMAGE | LVIF_PARAM;
      lvi.iItem=0;
      lvi.iSubItem=0;
      lvi.lParam=ti.lParam;
      lvi.iImage=ti.iImage;
      int pos = ListView_InsertItem(item, &lvi);
      lvi.mask = LVIF_TEXT;
      cHistDir * param = (cHistDir*)ti.lParam;
      lvi.pszText=(char*)(param->info.empty()?param->name.c_str():param->info.c_str());
      lvi.iItem=pos;
      lvi.iSubItem=hist.headers->pos("info");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);
      lvi.pszText=(char*)inttoch(((cHistDir*)ti.lParam)->count());
      lvi.iItem=pos;
      lvi.iSubItem=hist.headers->pos("count");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);

      it = TreeView_GetNextSibling(titem , it);
    }*/
	  for (hist_s::dirs_it it = hist.dirs.begin(); it != hist.dirs.end(); it++) {
		  if ((*it)->parent == this)
			  (*it)->listPushBack();
	  }
  }
  hist.sort();
}


void cHistDir::fill() {
  //Wypelnia liste ...
  if (!hist.headers) return;
/*
  HWND item = GetDlgItem(hwndHistory , IDC_LIST);
  LVITEM lvi;
  for (items_it_t items_it = items.begin(); items_it!=items.end(); items_it++) {
      cHistItem * hi = *items_it;
      lvi.iImage=Ico[hi->ico].index[0];
      lvi.mask = LVIF_IMAGE | LVIF_PARAM;
      lvi.iItem=0;
      lvi.iSubItem=0;
      lvi.lParam=(LPARAM)(*items_it);
      int pos = ListView_InsertItem(item, &lvi);

      //IMERROR();
      lvi.mask = LVIF_TEXT;
      lvi.iItem=pos;
      lvi.pszText=(char*)hi->name.c_str();
      lvi.iSubItem=hist.headers->pos("contact");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);
      lvi.pszText=(char*)hi->first_uid.c_str();
      lvi.iSubItem=hist.headers->pos("first_uid");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);
      lvi.pszText=(char*)hi->title.c_str();
      lvi.iSubItem=hist.headers->pos("title");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);

	  lvi.pszText=(char*)hi->parent->name.c_str();
      lvi.iSubItem=hist.headers->pos("parent");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);


      int hpos = hist.headers->pos("date");
      if (hpos>=0) {
        string s=(char*)hi->date.strftime(
                     (hist.headers->hdrs[hpos].fmt==""?"%H:%M %d %b'%y %A"
                     :(char*)hist.headers->hdrs[hpos].fmt.c_str())).c_str();
	    lvi.pszText = (char*)s.c_str();
		lvi.iSubItem=hpos;
        ListView_SetItem(item, &lvi);
      }  
      lvi.pszText=(char*)inttoch(hi->subItems+1);
      lvi.iSubItem=hist.headers->pos("count");
      if (lvi.iSubItem>=0) ListView_SetItem(item, &lvi);
  }
  */
  // kopiujemy po prostu listê elementów, to tablicy aktualnie wyœwietlanych
//  hist.itemsOnList.clear();
  for (items_it it = items.begin(); it != items.end(); it++)
	  (*it)->listPushBack();
  hist.listSynchronize();
  hist.sort();
}

bool cHistDir::search() {
	if (!canSearch()) return false;
	for (items_it_t it = items.begin(); it != items.end(); it++) {
		if (!hist.search.IsActive()) return false;
		(*it)->search();
	}
	return false;
}

void cHistDir::msgInsert(cMessage * m , const char * display , bool scroll) {
	hist.msgControl->msgInsert(m , display , scroll);
}

enReturn cHistDirDTB::erase() {
  IMLOG("* ERASE %s" , path.c_str());
  return unlink(path.c_str()) ? retFalse : retTrue;
}

void cHistDirDTB::compact() {
    CdTable Msg;
    Msg.cxor_key = HIST_XOR_KEY;
    Msg.cols = h_desc;
    CdtFileBin fb;
    fb.assign(&Msg);
	IMLOG("- > Wczytujê %s" , path.c_str());
    fb.load(path.c_str());
	IMLOG("- > Zapisujê...");
    fb.save(path.c_str());
}


void cHistDirDTB::open() {
//  if (hist_dir_opened == this) return;
//  MessageBox(0 , "DTB" , "" , 0);
  if (!items.size()) {
// Wczytuje dane do tablicy items'ow
    CdTable Msg;
    Msg.cxor_key = HIST_XOR_KEY;
    Msg.cols = h_desc;
    CdtFileBin fb;
    fb.assign(&Msg);
    if (fb.open(path.c_str() , DT_READ)) return;
    if (fb.freaddesc()) return;
    Msg.cols = fb.fcols;
    fb.freadsize();
    fb.fset(fb.pos_row , SEEK_SET);
    Msg.addrow();
    cHistItemDTB * lastHi = 0;
	int readColumns [] = {MSGH_SESSION , 0};
	SendMessage(hist.prgrsWnd , PBM_SETRANGE32 , 0 , this->size);
    SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);
	hist.SetStatus("Wczytujê plik...");

	while (1) {
//		LARGE_INTEGER tick , tick1 , tick2;
//		QueryPerformanceCounter(&tick);
		if (fb.freadpartialrow(0 , readColumns)) {
			if (!fb.ffindnextrow())
				continue; // mijamy...
			else
				break; // definitywnie koniec
		}
//		QueryPerformanceCounter(&tick1);
//		tick1.QuadPart = tick1.QuadPart - tick.QuadPart;
       if (Msg.getint(0 , MSGH_SESSION) && lastHi) {
         lastHi->subItems++;
       } else {
		   // Wczytaliœmy CZÊŒCIOWO! Trzeba wróciæ i wczytaæ porz¹dnie
		   fb.fset(Msg.rows[0]->pos , SEEK_SET);
		   SendMessage(hist.prgrsWnd , PBM_SETPOS , Msg.rows[0]->pos , 0);

//		   QueryPerformanceCounter(&tick);
		   if (fb.freadrow(0) != 0)
			   break; // to ju¿ jakiœ tragiczny b³¹d, raz czyta, raz nie
//		   QueryPerformanceCounter(&tick2);
//		   tick2.QuadPart = tick2.QuadPart - tick.QuadPart;
//		   IMLOG("TICK: %2.2f , %08I64d , %08I64d" , (float)(tick1.LowPart+1)/(float)(tick2.LowPart+1) , tick1.QuadPart , tick2.QuadPart);
         cHistItemDTB * hi = new cHistItemDTB;
         hi->parent = this;
		 hi->wholeThread = true; // optymistycznie, ale raczej prawda...
         hi->pos = Msg.rows[0]->pos;
         hi->id  = DT_MASKID(Msg.rows[0]->id);
         hi->subItems = 1;
         hi->dir = dir;
         hi->path = path;
         hi->name = name;
         hi->first_uid = Msg.getint(0,MSG_FLAG)&MF_SEND?Msg.getch(0 , MSG_TOUID):Msg.getch(0 , MSG_FROMUID);
		 hi->title = GetExtParam(Msg.getch(0 , MSG_EXT) , MEX_TITLE);
		 if (hi->title.empty()) {
			 string body = Msg.getch(0 , MSG_BODY);
			 if (Msg.getint(0 , MSG_FLAG) & MF_HTML) {
				 cPreg preg(false);
				 body = preg.replace("/<.+?>/" , "" , body.c_str());
				 body = DecodeEntities(body.c_str());
			 }
		     hi->title = body.substr(0 , 50);
		 }
		 hi->ico = UIIcon(IT_MESSAGE , Msg.getint(0,MSG_NET) , Msg.getint(0,MSG_TYPE) , 0);
		 if (!Ico.find(hi->ico)) hi->ico = UIIcon(IT_MESSAGE , 0 , Msg.getint(0,MSG_TYPE) , 0);

         unsigned int i;
         for (i=0; i<hi->title.size(); i++) {
            if ((unsigned char)hi->title[i] < 31) {hi->title.erase(i);break;}
         }
         if (i>=49) hi->title+="...";
         cTime64 t64(Msg.get64(0 , MSG_TIME));
         hi->sort = __int64(t64) & 0xFFFFFFFFFFFF;

         hi->date = t64;
         items.push_back(hi);
         lastHi = hi;
       }
    }
    fb.close();
  }
  SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);
  hist.SetStatus(0);
  fill();
}
bool cHistDirDTB::search() {
	if (!canSearch() || !hist.search.IsActive()) return false;
    CdTable Msg;
    Msg.cxor_key = HIST_XOR_KEY;
    Msg.cols = h_desc;
    CdtFileBin fb;
    fb.assign(&Msg);
    if (fb.open(path.c_str() , DT_READ)) return false;
    if (fb.freaddesc()) return false;
    Msg.cols = fb.fcols;
    fb.freadsize();
    fb.fset(fb.pos_row , SEEK_SET);
    Msg.addrow();
	int readColumns [] = {MSGH_SESSION , 0};
	int progress = SendMessage(hist.search.prgrsWnd , PBM_GETPOS , 0 , 0);
	bool alreadyRead = false;
	while (hist.search.IsActive()) {
		// Ka¿da taka pêtla, to jeden ca³y w¹tek (znaleziony, lub nie)
		if (!alreadyRead) {
			if (fb.freadrow(0) != 0)
				break; // nie ma wiêcej w¹tków
		} else 
			alreadyRead = false;

		size_t startPos = Msg.rows[0]->pos; // zapamiêtujemy pozycjê pierwszego w w¹tku...
		vector <size_t> positions; // pozycje kolejnych wiadomoœci
		cHistItemDTB * found;
		unsigned int inThread = 0;
		bool nextMatch = false; // czy skakaæ do nastêpnego szukania
		bool threadBeginner = Msg.getint(0 , MSGH_SESSION) == 0; // czy zaczêliœmy prawid³owo w¹tek...
		do {
			inThread ++;
			if (hist.search.opt.threadPrev > 0) // œledzimy poprzedzaj¹ce
				positions.push_back(Msg.rows[0]->pos);
			if (hist.search.opt.query.empty() == false)
				hist.search.preg.setSubject(Msg.getch(0 , MSG_BODY));
			__time64_t msgTime = 0;
			if (hist.search.opt.timeFrom || hist.search.opt.timeTo) {
				msgTime = cTime64(Msg.get64(0 , MSG_TIME));
			}
			if ((!hist.search.opt.timeFrom || hist.search.opt.timeFrom <= msgTime)
				&& (!hist.search.opt.timeTo || hist.search.opt.timeTo >= msgTime)
				&& (hist.search.opt.query.empty() || hist.search.preg.match() > 0)
			) {
				// skoro siê znalaz³ tworzymy item'a
				found = new cHistItemDTB();
				found->parent = hist.search.foundDir;
				found->cnt = this->cnt;
				found->date = Msg.get64(0 , MSG_TIME);
				found->id  = DT_MASKID(Msg.rows[0]->id);
				found->dir = this->dir;
				found->path = this->path;
				found->name = this->name;
				found->first_uid = Msg.getint(0,MSG_FLAG)&MF_SEND?Msg.getch(0 , MSG_TOUID):Msg.getch(0 , MSG_FROMUID);
				found->ico = UIIcon(IT_MESSAGE , Msg.getint(0,MSG_NET) , Msg.getint(0,MSG_TYPE) , 0);
				if (!Ico.find(found->ico)) found->ico = UIIcon(IT_MESSAGE , 0 , Msg.getint(0,MSG_TYPE) , 0);
				if (hist.search.opt.query.empty()) {
					found->title =  CStdString(Msg.getch(0 , MSG_BODY)).substr(0 , 50);
				} else {
					found->title =  hist.search.preg.getSubjectRef().substr(hist.search.preg.getVector(0) , 50);
				}
				unsigned int i;
				for (i=0; i<found->title.size(); i++) {
					if ((unsigned char)found->title[i] < 31) {found->title[i] = ' ';}
				}
				if (i>=49) found->title+="...";
				cTime64 t64(Msg.get64(0 , MSG_TIME));
				found->sort = __int64(t64) & 0xFFFFFFFFFFFF;
				found->date = t64;

				// Gdy zaliczamy WSZYSTKIE wczeœniejsze
				if (hist.search.opt.threadPrev < 0) {
					found->subItems = inThread;
					found->pos = startPos;
				} else if (hist.search.opt.threadPrev == 0) {
					// Zaliczamy OD znalezionego
					found->subItems = 1;
					found->pos = Msg.rows[0]->pos;
				} else {
					// i inne sytuacje...
					found->subItems = min(hist.search.opt.threadPrev + 1 , (signed)positions.size());
					found->pos = positions[(positions.size() - 1) - (found->subItems - 1)];
				}
				// Je¿eli zaliczonych, jest mniej ni¿ w w¹tku - to wiadomo - nie jest od pocz¹tku
				if (found->subItems < inThread - 1)
					threadBeginner = false;
				// Ustalamy limit
				found->limit = hist.search.opt.threadNext < 0 ? -1 : found->subItems + hist.search.opt.threadNext;
				// na szybko przelatujemy iloœæ zaliczanych póŸniejszych...
				while (found->subItems < found->limit 
					&& fb.freadpartialrow(0 , readColumns) == 0) 
				{
					if (Msg.getint(0 , MSGH_SESSION) == 0) {
						// to ju¿ nastêpny w¹tek... zostawiamy go nowej pêtli, wiêc trzeba cofn¹æ do czytania
						fb.fset(Msg.rows[0]->pos , SEEK_SET);
						// Je¿eli pierwszy zaliczony, to pierwszy w w¹tku, znaczy ¿e dojechaliœmy do koñca w¹tku i mamy ca³y
						if (threadBeginner)
							found->wholeThread = true;
						break;
					}
					found->subItems ++;
				}
				// dodajemy do listy znalezionych... mo¿emy sobie poczekaæ...
				hist.search.foundDir->addFoundItem(found);
				nextMatch = true;
			} // match
			SendMessage(hist.search.prgrsWnd , PBM_SETPOS , progress + Msg.rows[0]->pos , 0);
			Ctrl->WMProcess();
			if (nextMatch || fb.freadrow(0) != 0)
				break;
			if (Msg.getint(0 , MSGH_SESSION) == 0) {
				alreadyRead = true;
				break;
			}
		} while (hist.search.IsActive()); // czytamy kolejne wiadomoœci
	}
	fb.close();
	return true;
}

void cHistDirQueue::open() {
//  if (items.size()) {fill();return;}
  itemsClear();
  int c = Ctrl->DTgetCount(DTMSG);
  for (int  i=0; i < c; i++) {
	  int id = Ctrl->DTgetID(DTMSG , i);
    if (checkMsg(id)) {
      cHistItemQueue * hi = new cHistItemQueue;
      hi->parent = this;
      hi->pos = id;
      hi->subItems = 0;
      hi->dir = dir;
      hi->path = path;
      char * uid = (*Ctrl->DTgetStr(DTMSG , id , MSG_FROMUID))?Ctrl->DTgetStr(DTMSG , id , MSG_FROMUID):Ctrl->DTgetStr(DTMSG , id , MSG_TOUID);
      cnt = ICMessage(IMC_CNT_FIND , Ctrl->DTgetInt(DTMSG , id , MSG_NET)
                  , (int)uid);
      hi->name = cnt!=DT_NOROW?GETCNTC(cnt , CNT_DISPLAY):SAFECHAR(uid);
      cnt = Ctrl->DTgetID(DTCNT , cnt);
      hi->title = string(Ctrl->DTgetStr(DTMSG , id , MSG_BODY)).substr(0 , 50);
	  hi->ico = UIIcon(IT_LOGO , Ctrl->DTgetInt(DTMSG , id , MSG_NET) , 0, 0);
      unsigned int j;
      for (j=0; j<hi->title.size(); j++) {
          if ((unsigned char)hi->title[j] < 31) {hi->title.erase(j);}
      }
      if (j>=49) hi->title+="...";
      cTime64 t64;
      hi->sort = Ctrl->DTgetInt64(DTMSG , id , MSG_TIME);
      t64 = (__int64)hi->sort;
      hi->date = t64;
      items.push_back(hi);
    }
  }
  fill();
}

int cHistDirQueue::checkMsg(int pos) {
     int mflg = Ctrl->DTgetInt(DTMSG , pos , MSG_FLAG);
     int mnet = Ctrl->DTgetInt(DTMSG , pos , MSG_NET);
     int mtype = Ctrl->DTgetInt(DTMSG , pos , MSG_TYPE);
     if (
       (!net || (net == Ctrl->DTgetInt(DTMSG , pos , MSG_NET)))
       &&
       (!type || (type == Ctrl->DTgetInt(DTMSG , pos , MSG_TYPE)))
       &&
       (!flag || flag==-1 || (mflg & flag))
       &&
       (!(mflg & noflag))
       )
        return 1;
     return 0;   
}

enReturn cHistDirQueue::erase() {
    for (unsigned int i=0; i<this->items.size(); i++) {
        if (this->items[i]->erase() == retCancel) return retCancel;
    }
    return retFalse;
}


int cHistDirQueue::count() {
  int c = Ctrl->DTgetCount(DTMSG);
  int cnt = 0;
  for (int i=0; i < c; i++) {
        if (checkMsg(i)) cnt++;

  }
  return cnt;
}

bool cHistDirQueue::search() {
	if (!canSearch()) return false;
	int c = Ctrl->DTgetCount(DTMSG);
	for (int  i=0; i < c; i++) {
		if (!hist.search.IsActive())
			return true;
		int id = Ctrl->DTgetID(DTMSG , i);
		if (checkMsg(id)) {
			if (hist.search.opt.query.empty() == false)
				hist.search.preg.setSubject(Ctrl->DTgetStr(DTMSG , id , MSG_BODY));
			if ((!hist.search.opt.timeFrom || hist.search.opt.timeFrom <= Ctrl->DTgetInt64(DTMSG , id , MSG_TIME))
				&& (!hist.search.opt.timeTo || hist.search.opt.timeTo >= Ctrl->DTgetInt64(DTMSG , id , MSG_TIME))
				&& (hist.search.opt.query.empty() ||  hist.search.preg.match() > 0)
			) {
				cHistItemQueue * found = new cHistItemQueue;
				found->parent = this;
				found->pos = id;
				found->subItems = 0;
				found->dir = dir;
				found->path = path;
				char * uid = (*Ctrl->DTgetStr(DTMSG , id , MSG_FROMUID))?Ctrl->DTgetStr(DTMSG , id , MSG_FROMUID):Ctrl->DTgetStr(DTMSG , id , MSG_TOUID);
				cnt = ICMessage(IMC_CNT_FIND , Ctrl->DTgetInt(DTMSG , id , MSG_NET)
					, (int)uid);
				found->name = cnt!=DT_NOROW?GETCNTC(cnt , CNT_DISPLAY):SAFECHAR(uid);
				cnt = Ctrl->DTgetID(DTCNT , cnt);
				if (hist.search.opt.query.empty()) {
					found->title = CStdString(Ctrl->DTgetStr(DTMSG , id , MSG_BODY)).substr(0 , 50);
				} else {
					found->title = hist.search.preg.getSubjectRef().substr(hist.search.preg.getVector(0) , 50);
				}
			    found->ico = UIIcon(IT_LOGO , Ctrl->DTgetInt(DTMSG , id , MSG_NET) , 0, 0);
				unsigned int j;
				for (j=0; j<found->title.size(); j++) {
					if ((unsigned char)found->title[j] < 31) {found->title[j] = ' ';}
				}
				if (j>=49) found->title+="...";
				cTime64 t64(Ctrl->DTgetInt64(DTMSG , id , MSG_TIME));
				found->sort = __int64(t64) & 0xFFFFFFFFFFFF;
				found->date = t64;
				hist.search.foundDir->addFoundItem(found);
			} // match
		}
	}
}

void cHistDirSearch::open() {
    fill();
	if (hist.search.state != hist_s::search_s::stReady) return;
	hist.search.Reset();
	this->opt.SetInDialog(0);
	if (this->items.empty())
		hist.search.Show();
}

void cHistDirSearch::msgInsert(cMessage * msg , const char * display , bool scroll){
	if (!GETINT(CFG_UIHISTORY_MARKFOUND) || hist.search.opt.query.empty()) {
		cHistDir::msgInsert(msg , display , scroll);
		return;
	}
	cMessage m = *msg;
	CStdString body;
	if (!(m.flag & MF_HTML)) {
		m.flag |= MF_HTML;
		body = EncodeEntities(m.body);
		body.Replace("\n" , "<br/>");
		this->preg.setSubject(body);
	} else
		this->preg.setSubject(m.body);
	body = this->preg.replace("<span class=\"mark\">$&</span>");
	m.body = (char*) body.c_str();
	this->preg.setSubject("");
	cHistDir::msgInsert(&m , display , scroll);
}

void cHistDirSearch::addFoundItem(cHistItem * item) {
	item->parent = this;
	this->items.push_back(item);
	if (hist.dir_opened == this) {
		item->listPushBack();
		hist.listSynchronize();
	}
	if (hist.search.opt.findAll) {
		Ctrl->WMProcess();
	} else {
		// Czekamy na "ZnajdŸ nastêpny"
		this->select();
		item->select();
		EnableWindow(GetDlgItem(hist.search.wnd , IDOK) , true);
		while (hist.search.IsActive() && !hist.search.canGetNext) {
			Ctrl->WMProcess();
			SleepEx(100 , true);
		}
		hist.search.canGetNext = false;
		if (hist.search.state != hist_s::search_s::stCancel)
			EnableWindow(GetDlgItem(hist.search.wnd , IDOK) , false);
	}
}

enReturn cHistDirSearch::erase() {
	if (!items.size()) return retTrue;
	sDIALOG_choose sdc;
	sdc.title = "Usuwanie wyników wyszukiwania";
	sdc.info = "Wybierz co chcesz usun¹æ...";
	sdc.items = "Zapytanie\nZnalezione w¹tki";
	int r = ICMessage(IMI_DLGBUTTONS , (int)&sdc);
	if (!r) return retCancel;
	if (r==1) return retTrue;
	else {
		if (hist.dir_opened == this) 
			hist.listClear();
		while (!items.empty()) {
			enReturn r = items.back()->erase();
			if (r == retTrue) {
				delete items.back();
				items.pop_back();
			} else if (r == retCancel)
				break;
		}
	}
}
bool cHistDirSearch::saveBefore() {
	sDIALOG_choose sdc;
	sdc.title = "Zapisywanie wyników wyszukiwania";
	sdc.info = "Wybierz co chcesz zapisaæ...";
	sdc.items = "Zapytanie\nZnalezione w¹tki";
	int r = ICMessage(IMI_DLGBUTTONS , (int)&sdc);
	if (!r) return true;
	if (r==1) {
		sDIALOG_enter sde;
		sde.title = sdc.title;
		sde.info = "Podaj krótk¹ nazwê zapytania";
		sde.id = "UI/history/saveQuery";
		if (ICMessage(IMI_DLGENTER , (int)&sde) && *sde.value) {
			CStdString fn = sde.value;
			cPreg preg;
			fn = preg.replace("/[^\\w\\d]/" , "_" , fn);
			fn =  HISTSETTINGSDIR "search\\query-" + fn + ".xml";
			std::ofstream f;
			f.open(fn);
			if (f.is_open()) {
				f << "<?xml version=\"1.0\" encoding=\"Windows-1250\"?>\n<history>\n  <directory>\n";
				f << "	<name>" << EncodeEntities(sde.value) << "</name>" << endl;
				f << "	<info>" << EncodeEntities(this->info) << "</info>" << endl;
				f << "	<source>SEARCH</source>" << endl;
				f << "	<search>" << endl;
				f << "		<find_all>" << opt.findAll << "</find_all>" << endl;
				f << "		<only_current>" << opt.onlyCurrent << "</only_current>" << endl;
				f << "		<query>" << EncodeEntities(opt.query) << "</query>" << endl;
				f << "		<regexp>" << opt.regexp << "</regexp>" << endl;
				f << "		<thread>" << endl;
				f << "			<previous>" << opt.threadPrev << "</previous>" << endl;
				f << "			<next>" << opt.threadNext << "</next>" << endl;
				f << "		</thread>" << endl;
				f << "		<use_case>" << opt.useCase << "</use_case>" << endl;
				f << "		<words>" << opt.words << "</words>" << endl;
				f << "		<timeFrom>" << opt.timeFrom << "</timeFrom>" << endl;
				f << "		<timeTo>" << opt.timeTo << "</timeTo>" << endl;
				f << "	</search>" << endl;
				f << " </directory>" << endl;
				f << "</history>" << endl;
			}
			f.close();
		}
		return true;
	} else return false;
}

enReturn cHistItemDTB::erase() {
  IMLOG("* Erasing item %d+%d @ %s" , DT_UNMASKID(id) , subItems , path.c_str());

  if (!this->wholeThread) {
	  int r;
	  r = MessageBox(hwndHistory , "Próbujesz usun¹æ wiadomoœci, które s¹ czêœci¹ wiêkszego w¹tku.\r\nKontynuuj¹c mo¿esz spowodowaæ po³¹czenie dwóch ró¿nych w¹tków.\r\nCzy mimo wszystko je usun¹æ?" , "Usuwanie wiadomoœci z w¹tku" , MB_ICONWARNING | MB_YESNOCANCEL);
	  if (r == IDCANCEL) return retCancel;
	  if (r == IDNO) return retFalse;
  }

  hist.changeDT(path.c_str() , false);
/*  int pos = hist.DT.getrowpos(id);
  for (unsigned int i=0; i<subItems+1; i++) hist.DT.deleterow(pos);
  */

  hist.DTfb.fset(this->pos , SEEK_SET);
  for (unsigned int i=0; i<subItems; i++) {
  	  if (i >= this->limit-1) break; //
      if (hist.DTfb.fseterasedrow(true , i==0?this->id : 0) != 0)
		  return retFalse;
  }
  return retTrue;
}

enReturn cHistItemDTB::save(ofstream &file , int type) {
    // Przygotowywuje sie do czytania z DTB
    CdTable Msg;
    Msg.cxor_key = HIST_XOR_KEY;
    Msg.cols = h_desc;
    CdtFileBin fb;
    fb.assign(&Msg);
    if (fb.open(path.c_str() , DT_READ)) return retFalse;
    if (fb.freaddesc()) return retFalse; // Problemy z czytaniem deskryptora?
    Msg.cols = fb.fcols;
    fb.fset(pos , SEEK_SET);
	
    Msg.addrow(); // Czytanie odbywa sie na jednym wierszu...
	if (fb.freadrow(0)) return retFalse;
	cTime64 t64 (Msg.get64(0 , MSG_TIME)); 
	if (type == saveTXT) {
		file << "---------------------------------" << endl;
		file << t64.strftime("%H:%M %d %m'%y %A") << endl;
		file << "---------------------------------" << endl;
	} else if (type==saveXML) {
		if (this->subItems > 1) {
			file << "		<chat date=\"" << t64.strftime("%d-%m-%Y") << "\" time=\"" << t64.strftime("%H:%M") << "\">" << endl;
		}
	}
	int saved = 0;
    do {
        t64 = Msg.get64(0 , MSG_TIME); 
		if (type == saveTXT) {
			file << Msg.getch(0 , MSGH_DISPLAY) << t64.strftime(" [%H:%M]:") << endl;
			file << '\t' << Msg.getch(0 , MSG_BODY) << endl << endl;
		} else if (type == saveXML) {
			file << "			<message name=\"" << EncodeEntities(Msg.getch(0 , MSGH_DISPLAY)) << "\" "
				<< " date=\"" << t64.strftime("%d-%m-%Y") << "\" time=\"" << t64.strftime("%H:%M") << "\" "
				;
			if (GETINT(CFG_UIHISTORY_XMLFULL)) {
				file << " net=\"" << Msg.getint(0 , MSG_NET) << "\" "
					<< " type=\"" << Msg.getint(0 , MSG_TYPE) << "\" "
					<< " flag=\"" << Msg.getint(0 , MSG_FLAG) << "\" "
					<< " fromUid=\"" << EncodeEntities(Msg.getch(0 , MSG_FROMUID)) << "\" "
					<< " toUid=\"" << EncodeEntities(Msg.getch(0 , MSG_TOUID)) << "\" "
					<< " ext=\"" << EncodeEntities(Msg.getch(0 , MSG_EXT)) << "\" "
					;
			}
			file << ">" << EncodeEntities(Msg.getch(0 , MSG_BODY))
				<< "</message>" << endl;
		}
		if (++saved >= this->limit) break;
        if (fb.freadrow(0)) break;
    } while (Msg.getint(0 , MSGH_SESSION)); // Dopoki czytane wiadomosci naleza do wybranej sesji
	if (type == saveTXT) {
		file <<   "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl << endl;
	} else if (type == saveXML) {
		if (this->subItems > 1)
			file << "		</chat>" << endl;
	}
	return retTrue;
}

void cHistItemDTB::open() {
  // Przygotowywuje sie do czytania z DTB
  SendMessage(hist.prgrsWnd , PBM_SETRANGE , 0 , MAKELPARAM(0 , min((unsigned)subItems , limit)));
  SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);
  HWND item = hist.msgControl->hwnd;

  CdTable Msg;
  Msg.cxor_key = HIST_XOR_KEY;
  Msg.cols = h_desc;
  CdtFileBin fb;
  fb.assign(&Msg);
  if (fb.open(path.c_str() , DT_READ)) return;
  if (fb.freaddesc()) return;
  Msg.cols = fb.fcols;
  fb.fset(pos , SEEK_SET);
  Msg.addrow();
  if (fb.freadrow(0)) return;
  hist.msgControl->lock();
  unsigned int shown = 0;
  do {
    int type = Msg.getint(0 , MSG_TYPE);
    if (Msg.getint(0,MSG_FLAG) & MF_SEND) type |= 0x10000;
    cMessage m;
    m.id = Msg.getint(0 , MSG_ID);
    m.net = Msg.getint(0 , MSG_NET);
    m.fromUid = Msg.getch(0 , MSG_FROMUID);
    m.toUid = Msg.getch(0 , MSG_TOUID);
    m.flag = Msg.getint(0 , MSG_FLAG);
    m.type = Msg.getint(0 , MSG_TYPE);
    m.body = Msg.getch(0 , MSG_BODY);
    m.ext = Msg.getch(0 , MSG_EXT);
    m.time = Msg.get64(0 , MSG_TIME);
	(hist.dir_opened?hist.dir_opened:parent)->msgInsert(&m , Msg.getch(0 , MSGH_DISPLAY),false);
    SendMessage(hist.prgrsWnd , PBM_STEPIT , 0 , 0);
	if (++shown >= this->limit) break; // pokazaliœmy wystarczaj¹co du¿o...
    if (fb.freadrow(0)) break;
  } while (Msg.getint(0 , MSGH_SESSION)); // Dopoki czytane wiadomosci naleza do wybranej sesji
  hist.msgControl->unlock();
  fb.close();
  SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);

}

void cHistItemQueue::open() {
	if (Ctrl->DTgetPos(DTMSG,pos)==-1) return;
    cMessage m;
    Ctrl->DTlock(DTMSG , pos);
    m.id = Ctrl->DTgetInt(DTMSG , pos , MSG_ID);
    m.net = Ctrl->DTgetInt(DTMSG , pos , MSG_NET);
    m.fromUid = Ctrl->DTgetStr(DTMSG , pos , MSG_FROMUID);
    m.toUid = Ctrl->DTgetStr(DTMSG , pos , MSG_TOUID);
    m.flag = Ctrl->DTgetInt(DTMSG , pos , MSG_FLAG);
    m.type = Ctrl->DTgetInt(DTMSG , pos , MSG_TYPE);
    m.body = Ctrl->DTgetStr(DTMSG , pos , MSG_BODY);
    m.ext = Ctrl->DTgetStr(DTMSG , pos , MSG_EXT);
    m.time = Ctrl->DTgetInt64(DTMSG , pos , MSG_TIME);
	(hist.dir_opened?hist.dir_opened:parent)->msgInsert(&m , 0 , false);
    Ctrl->DTunlock(DTMSG , pos);
  }

enReturn cHistItemQueue::erase() {
    sMESSAGESELECT ms;
    ms.id = pos;
	ICMessage(IMC_MESSAGEREMOVE , (int)&ms);
    return retTrue;
}

void cHistItem::select() {
	if (hist.item_opened == this) return;
	// szukamy na liœcie...
	if (hist.selected != this) {
		hist_s::itemsOnList_it it = std::find(hist.itemsOnList.begin() , hist.itemsOnList.end() , this);
		if (it != hist.itemsOnList.end()) {
			int i = it - hist.itemsOnList.begin();
			ListView_SetSelectionMark(hist.listWnd , i);
			ListView_SetItemState(hist.listWnd , -1 , 0 , LVIS_SELECTED);
			ListView_SetItemState(hist.listWnd , i , LVIS_SELECTED | LVIS_FOCUSED , LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(hist.listWnd , i , FALSE);
		}
	}
	cHist::select();
}


void cHistItem::prepare() {
  if (hist.item_opened == this) return;
  hist.item_opened = this;
  hist.msgControl->clear();
  open();
}

const CStdString cHistItem::getColumnString(const cHistHeader * col) {
	if (col->id == "title")
		return this->title;
	else if (col->id == "first_uid")
		return this->first_uid;
	else if (col->id == "date") {
		return this->date.strftime( col->fmt.empty() ? "%H:%M %d %b'%y %A" : col->fmt.c_str() );
	} else
		return cHist::getColumnString(col);
}


// **************************************************
hist_s::hist_s() {
	selectedList = IDC_TREE;
    running = false;
}

  void hist_s::shred() {
      if (MessageBox(this->hwnd , "Czy na pewno chcesz usun¹æ CA£¥ zawartoœæ historii?" , "Historia" , MB_YESNO)==IDNO) return;
	  erase(0);
      refresh(true);
  }
  enReturn hist_s::erase(cHist * item) {
	  // Usuwa podkatalogi
     if (!item||item->type<HDT_ITEM) {
       for (unsigned int i=0; i<dirs.size(); i++)
		   if (dirs[i]->parent == item && dirs[i]!=item) {
				enReturn r = erase(dirs[i]);
				if (r == retTrue) i--;
				else if (r == retCancel) return r;
		   }
     }
     if (!item) return retFalse;
	 // Usuwa element
	 enReturn r = item->erase();
     if (r == retTrue) {
       if (item->type<HDT_ITEM) {
         TreeView_DeleteItem(GetDlgItem(hwnd , IDC_TREE) , ((cHistDir*)item)->treeItem);
         TreeView_SelectItem(GetDlgItem(hwnd , IDC_TREE) , 0);
       }
     }
     return r;
  }
  void hist_s::compact(void) {
      if (MessageBox(hwndHistory , "Kompaktowanie ma sens tylko, gdy niedawno zosta³y\r\n" 
          "usuniête jakieœ wiadomoœci\r\n\r\nKompaktowanie mo¿e chwilê potrwaæ!\r\nKontyunowaæ?", "Konnekt - kompaktowanie historii" , MB_YESNO|MB_ICONQUESTION) != IDYES) return;
      changeDT(0);
      EnableWindow(hwndHistory , 0);
      EnableWindow(this->msgControl->hwnd , 0);
      EnableWindow(GetDlgItem(hwndHistory , IDC_TREE) , 0);
      EnableWindow(hist.listWnd , 0);

      SendMessage(hist.prgrsWnd , PBM_SETRANGE , 0 , MAKELPARAM(0 , dirs.size()));
      SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);
	  IMLOG("- Kompaktowanie rozpoczête");
      for (unsigned int i=0; i<dirs.size(); i++) {
          dirs[i]->compact();
          SendMessage(hist.prgrsWnd , PBM_SETPOS , i , 0);
          Ctrl->WMProcess();
      }
	  IMLOG("- Kompaktowanie zakoñczone");
      SendMessage(hist.prgrsWnd , PBM_SETPOS , 0 , 0);

      EnableWindow(hwndHistory , 1);
      EnableWindow(this->msgControl->hwnd, 1);
      EnableWindow(GetDlgItem(hwndHistory , IDC_TREE) , 1);
      EnableWindow(hist.listWnd , 1);

  }
  void hist_s::changeDT(const char * file , bool useTemp) {
     IMLOG("hist.changeDT %d %s -> %s" , DTopened , DTopened?DTfb.fileName.c_str():"null" , file?file:"null");
     if (DTopened && DTfb.fileName==SAFECHAR(file)) return;
     if (DTopened) 
         if (DTtemp) DTfb.save(0);
         else DTfb.close();
     if (file) {
         if (useTemp) {
             DTfb.use_temp = true;
             DTfb.load(file);
         } else {
             DTfb.use_temp = false;
             DTfb.open(file , DT_WRITE|DT_READ);
         }
         DTopened=true;
     }
       else {DTopened=false;}
     IMLOG("hist.changeDT-> %s" , DTopened?DTfb.fileName.c_str():"null");
       
  }
void hist_s::eraseSelected() {
	if (!selected) return;
	if (MessageBox(this->hwnd , 
		(selectedList == IDC_TREE)?"Czy na pewno chcesz usun¹æ zaznaczony katalog i jego podkatalogi?":"Czy na pewno chcesz usun¹æ zaznaczone pozycje?"
		, "Historia" , MB_YESNO)==IDNO) return;
	hist.resetOpenedItem();
	if (selectedList == IDC_TREE) {
		// Na 100% katalog!
        erase(selected);
        listPrepare(0);
    } else {
		DTopened=false;
//       HWND item = GetDlgItem(hwnd , IDC_LIST);
		itemsOnList_it it = itemsOnList.begin();
		while (it != itemsOnList.end()) {
			cHist * item = (*it);
			if (item->listState & LVIS_SELECTED) {
				// Usuwamy zaznaczony element...
				cHistDir * parent = item->getParent();
				bool dir = item->type < HDT_ITEM;
				enReturn r = erase(item);
				if (r == retCancel) break;
				if (r == retTrue) {
					// kasujemy z listy ZANIM go zniszczymy...
					it = itemsOnList.erase(it);
			  // Usuwamy ew. wpisy u parenta
					if (parent) {
						items_it_t items_it = find(parent->items.begin() , parent->items.end() , item);
						if (items_it != parent->items.end()) {
							parent->items.erase(items_it);
						}
						if (!dir) delete item;
					}
					continue;
				}
			}
			it++;
		}
		changeDT(0);
		hist.listSort();
		int sel=ListView_GetSelectionMark(hist.listWnd);
//       refresh(false);
		if (sel >=0 && itemsOnList.size() < (unsigned) sel) {
			selected = itemsOnList[sel];
			ListView_SetItemState(hist.listWnd , sel , LVIS_SELECTED , LVIS_SELECTED);
		} else
			selected=0;
		
    }
}

  enReturn hist_s::save(ofstream & file , cHist * item , int type) {
	  /* MO¯NA USUN¥Æ */
     if (!item) return retFalse;
	 SetStatus("Zapisujê do pliku...");
     enReturn r = item->save(file , type);
	 if (r == retCancel) return r;
     if (!item||item->type<HDT_ITEM) {
       for (unsigned int i=0; i<dirs.size(); i++)
		   if (dirs[i]->parent == item && dirs[i]!=item) {
			   if (save(file , dirs[i] , type) == retCancel) return retCancel;
		   }
     }
	 SetStatus(0);
     return retTrue;
  }


  void hist_s::saveSelected() {
     if (!selected) return;
	 if (selected->saveBefore()) return;
     OPENFILENAME of;
     memset(&of , 0 , sizeof(of));
     of.lpstrTitle = "Zapisz historiê";
     of.hwndOwner = this->hwnd;
     of.lStructSize=sizeof(of)-12;
     of.lpstrFilter="Plik tekstowy\0*.txt\0XML\0*.xml\0";
	 of.nFilterIndex = cHist::saveXML;
     strcpy(TLS().buff , selected->name.c_str());
     //strcat(TLS().buff , ".txt");
     of.lpstrFile=(LPSTR)TLS().buff;
     of.nMaxFile=MAX_STRING;
//                          of.nFilterIndex=1;
     of.Flags=OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
     of.lpstrDefExt="";
     if (!GetSaveFileName(&of)) return;

     ofstream file;
	 int type = of.nFilterIndex;
     file.open(of.lpstrFile);
     if (!file.is_open()) return;
     cTime64 t64(true);
	 if (type == cHist::saveTXT) {
		file << "Zapis historii rozmów Konnekta" << endl << "  W kolejnoœci od najstarszego"<<endl;
		file << t64.strftime("%d-%m-%Y %H:%M") << endl;
		file << endl;
	 } else if (type == cHist::saveXML) {
		 file << "<?xml version=\"1.0\" encoding=\"Windows-1250\"?>"<<endl;
		 SetEnvironmentVariable("HistoryXSL", ExpandEnvironmentStrings(GETSTR(CFG_UIHISTORY_XMLXSL)).c_str());
		 file << ExpandEnvironmentStrings(GETSTR(CFG_UIHISTORY_XMLHEADER), 5000) << endl;
		 file << "<history>" << endl;
		 file << "	<info>" << endl;
		 file << "		<date>" << t64.strftime("%d-%m-%Y %H:%M") << "</date>" << endl;
		 file << "		<profile>" << (char*)ICMessage(IMC_GETPROFILE) << "</profile>" << endl;
		 file << "	</info>" << endl;
	 }

	 if (selectedList == IDC_TREE) { // katalog
         selected->save(file , type);
     } else {
		 for (itemsOnList_it item = itemsOnList.begin(); item != itemsOnList.end(); item++) {
			if ((*item)->listState & LVIS_SELECTED) {
				if ((*item)->save(file , type) == retCancel)
					break;
			}
       }
     }
	 if (type == cHist::saveXML) {
		 file << "</history>" << endl;
	 }
     file.close();

  }

	void hist_s::printSelected() {
		if (!selected) return;
		//if (selected->saveBefore()) return;
		CStdString filename = ExpandEnvironmentStrings(stringf("%%KonnektTemp%%\\history_print_%d.xml",time(0)).c_str());
		ofstream file;
		file.open(filename);
		if (!file.is_open()) return;
		cTime64 t64(true);
		file << "<?xml version=\"1.0\" encoding=\"Windows-1250\"?>"<<endl;
		SetEnvironmentVariable("HistoryXSL", ExpandEnvironmentStrings(GETSTR(CFG_UIHISTORY_XMLPRINTXSL)).c_str());
		file << ExpandEnvironmentStrings(GETSTR(CFG_UIHISTORY_XMLHEADER), 5000) << endl;
		file << "<history>" << endl;
		file << "	<info>" << endl;
		file << "		<date>" << t64.strftime("%d-%m-%Y %H:%M") << "</date>" << endl;
		file << "		<profile>" << (char*)ICMessage(IMC_GETPROFILE) << "</profile>" << endl;
		file << "	</info>" << endl;

		if (selectedList == IDC_TREE) { // katalog
			selected->save(file , cHist::saveXML);
		} else {
			for (itemsOnList_it item = itemsOnList.begin(); item != itemsOnList.end(); item++) {
				if ((*item)->listState & LVIS_SELECTED) {
					if ((*item)->save(file , cHist::saveXML) == retCancel)
						break;
				}
		   }
		}
		file << "</history>" << endl;
	    file.close();
		ShellExecute(0, "Open", "iexplore.exe", filename, "", SW_MAXIMIZE);
	}


  void hist_s::checkErase() {
    SendMessage(tbWnd , TB_ENABLEBUTTON , IDI_HISTB_DEL , (selected&&selected->canErase()));
  }
  void hist_s::checkSave() {
	  bool canSave = (selected&&selected->canSave());
      SendMessage(tbWnd , TB_ENABLEBUTTON , IDI_HISTB_SAVE , canSave);
      SendMessage(tbWnd , TB_ENABLEBUTTON , IDI_HISTB_PRINT , canSave);
  }


void hist_s::listPrepare(cHistHeaders * headers) {
	ListView_DeleteAllItems(listWnd);
	while (ListView_DeleteColumn(listWnd , 0)) {}
	if (headers) {
		for (unsigned int i=0; i<headers->hdrs.size(); i++) {
			headers->hdrs[i].index = ListView_AddColumn(listWnd , (char*)headers->hdrs[i].name.c_str() , headers->hdrs[i].width);
		}
	}
	this->headers = headers;
	hist.msgControl->clear();
}
void hist_s::listSynchronize() {
	ListView_SetItemCountEx(listWnd , this->itemsOnList.size() , LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
void hist_s::listClear() {
	ListView_SetItemCountEx(listWnd , 0 , 0);
	this->itemsOnList.clear();
}


  void hist_s::removeFromDirs(cHistDir * dir) {
     for (unsigned int i=0; i<dirs.size(); i++) {
       if (dirs[i]==dir) {dirs.erase(dirs.begin()+i);return;}
     }
  }


  void hist_s::refresh(bool all) {
/* TODO : Inny sposob refreszowania katalogow */
    listPrepare(0);
    if (all) {
      end();
      hist.dialogFill(hwndHistory);
    } else {
      if (dir_opened) {
        cHistDir * dir = dir_opened;
        dir_opened = 0;
        selected=0;
        dir->itemsClear();
        dir->prepare();
      }
    }
  }


void hist_s::start(int cnt) {
  if (!hwndHistory) {
      hwndHistory=CreateDialog(Ctrl->hDll() , MAKEINTRESOURCE(IDD_HISTORY)
            , /*hwndMain*/0 , (DLGPROC)HistoryDialogProc);
	  search.wnd = CreateDialog(Ctrl->hDll() , MAKEINTRESOURCE(IDD_HISTORY_SEARCH)
		  , hwndHistory , (DLGPROC)search.WndProc);
 	  search.ResetMRU();
	  msgControl = new cMsgControl(sUIAction(IMIG_HISTORYWND , Konnekt::UI::ACT::msg_ctrlview , 0) , 100 , 100 , 100 , 100 , hwndHistory , (HMENU)IDC_MSG);
      onSizeHistory(0,0);
      IMLOG("- History created");
  }
  if (cnt>0) {
    cnt = Ctrl->DTgetID(DTCNT , cnt);
	for (hist_s::dirs_it it=hist.dirs.begin(); it!=hist.dirs.end();it++)
       if ((*it)->cnt == cnt) {(*it)->select();break;}
  }
  ShowWindow(hwndHistory , SW_SHOW);
  SetForegroundWindow(hwndHistory);
}

void hist_s::resetOpenedItem() {
	this->msgControl->clear();
	this->item_opened = 0;
}

// *************************************************

bool hist_sort_compare(cHist * a , cHist * b) {
	if (hist.dir_opened->sortColumn == 0) {
		return a->sort > b->sort;
	}
	cHistHeader * col = &hist.dir_opened->headers->hdrs[abs(hist.dir_opened->sortColumn)-1];
	bool rev = hist.dir_opened->sortColumn > 0;
	if (a->type == HDT_ITEM && b->type == HDT_ITEM) {
		if (col->id == "date") {
			return rev ^ (static_cast<cHistItem*>(a)->date > static_cast<cHistItem*>(b)->date);
		} 
	}
	if (col->id == "count") {
		return rev ^ (a->count() > b->count());
	} else if (col->id == "size") {
		return rev ^ (a->size > b->size);
	}
	return rev ^ (a->getColumnString(col) > b->getColumnString(col));
}

void hist_s::listSort() {
	if (!itemsOnList.empty() && dir_opened && dir_opened->headers) {
		std::sort(itemsOnList.begin() , itemsOnList.end() , hist_sort_compare);
	}
//	ListView_RedrawItems(listWnd , 0 , itemsOnList.size()-1);
	ListView_SetItemCountEx(listWnd , 0 , 0);
	this->listSynchronize();
}

void hist_s::GetDispInfo(NMLVDISPINFO * disp) {
	if (disp->item.iItem < 0 || disp->item.iItem >= (signed)itemsOnList.size()) return;
	if (disp->item.mask & LVIF_STATE) {
		disp->item.state = itemsOnList[disp->item.iItem]->listState;
	}
	if (disp->item.mask & LVIF_IMAGE) {
		disp->item.iImage = Ico[itemsOnList[disp->item.iItem]->ico].index[0];
	}
	if (disp->item.mask & LVIF_TEXT) {
		// Zak³adam, ¿e o wszystko pyta g³ówny w¹tek...
		if (!dir_opened || !dir_opened->headers) {
			disp->item.pszText[0] = 0;
			return;
		}
//		IMLOG("GETDISPTEXT iolc=%d item=%d iol=%x d_o.h=%x subitem=%d hdrc=%d hdr=%x" , 
//			itemsOnList.size() , disp->item.iItem , itemsOnList[disp->item.iItem] , dir_opened->headers , disp->item.iSubItem , dir_opened->headers->hdrs.size() , &dir_opened->headers->hdrs[disp->item.iSubItem]);
		cHist * iol = itemsOnList[disp->item.iItem];
//		IMLOG("GETDISPTEXT iolc=%d item=%d iol=%x d_o.h=%x subitem=%d hdrc=%d hdr=%x  string=%s" , 
//			itemsOnList.size() , disp->item.iItem , itemsOnList[disp->item.iItem] , dir_opened->headers , disp->item.iSubItem , dir_opened->headers->hdrs.size() , &dir_opened->headers->hdrs[disp->item.iSubItem] , itemsOnList[disp->item.iItem]->getColumnString(&dir_opened->headers->hdrs[disp->item.iSubItem]).c_str());
		strncpy(disp->item.pszText , itemsOnList[disp->item.iItem]->getColumnString(&dir_opened->headers->hdrs[disp->item.iSubItem]) , disp->item.cchTextMax);
//		static CStdString buffer = itemsOnList[disp->item.iItem]->getColumnString(&dir_opened->headers->hdrs[disp->item.iSubItem]);
//		disp->item.pszText = (char*) buffer.c_str();
	}
	disp->item.mask &= ~LVIF_DI_SETITEM;
}
void hist_s::SetDispInfo(NMLVDISPINFO * disp) {
	if (disp->item.iItem < 0 || disp->item.iItem >= (signed)itemsOnList.size()) return;
	if (disp->item.mask & LVIF_STATE) {
		itemsOnList[disp->item.iItem]->listState = disp->item.state;
	}
}
void hist_s::ODStateChanged(NMLVODSTATECHANGE * state){
	if (state->iFrom >= (signed)itemsOnList.size() || state->iTo >= (signed)itemsOnList.size()) return;
//	IMLOG("STATERANGE: i = %d - %d st = %d" , state->iFrom , state->iTo , state->uNewState);
	itemsOnList_it it;
	int i;
/*
	for (it = itemsOnList.begin() , i = 0 
		; it != itemsOnList.end()
		; it++ , i++) {
		(*it)->listState = ( i>= state->iFrom && i < state->iTo ) ? state->uNewState : 0;
	}*/
	for (it = itemsOnList.begin() + state->iFrom , i = state->iFrom 
		; it != itemsOnList.end() && i <= state->iTo
		; it++ , i++) {
		(*it)->listState = state->uNewState;
	}
}
void hist_s::SetStatus(const char * status) {
	SendMessage(statusWnd , SB_SETTEXT , 0 , (int) (status ? status : ""));
    RedrawWindow(hist.prgrsWnd,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);
}



void hist_s::init() {
//  h_desc = *(CdtColDesc*)ICMessage(IMC_GETMSGCOLDESC);
  h_desc.setcolcount(11,0);
  h_desc.setcol(MSG_ID , DT_CT_INT , 0);
  h_desc.setcol(MSG_NET , DT_CT_INT , 0);
  h_desc.setcol(MSG_TYPE , DT_CT_INT , 0);
  h_desc.setcol(MSG_FROMUID , DT_CT_PCHAR  | DT_CT_CXOR , 0);
  h_desc.setcol(MSG_TOUID , DT_CT_PCHAR | DT_CT_CXOR , 0);
  h_desc.setcol(MSG_BODY , DT_CT_PCHAR | DT_CT_CXOR , 0);
  h_desc.setcol(MSG_EXT , DT_CT_PCHAR | DT_CT_CXOR , 0);
  h_desc.setcol(MSG_FLAG , DT_CT_INT , 0);
  h_desc.setcol(MSG_TIME , DT_CT_64 , 0);
  h_desc.setcol(MSGH_SESSION , DT_CT_INT , 0);
  h_desc.setcol(MSGH_DISPLAY , DT_CT_PCHAR | DT_CT_CXOR , 0);

  historyDir = profileDir + "history\\";
  iml = ImageList_Create(32 , 32 , ILC_COLOR32|ILC_MASK	 , 5 , 5);
  HICON icon;
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_REFRESH) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_SHRED) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_DEL) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_PRINT) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_SEARCH) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_RESEND) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_SAVE) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);
  icon = LoadIconEx(Ctrl->hDll() , MAKEINTRESOURCE(IDI_HISTB_COMPACT) , 32);
  ImageList_AddIcon(iml , icon);
  DestroyIcon(icon);

  hist.DT.cols = h_desc;
  DTfb.assign(&DT);
}

void hist_s::end() {
    changeDT(0);
    running = false;
  TreeView_DeleteAllItems(GetDlgItem(hwndHistory , IDC_TREE));
}



// **************************************************
HTREEITEM hist_makeTreeItem(HWND hwnd , cHistDir * hd) {
	TVINSERTSTRUCT tvs;
	TVITEMEX * tie = &tvs.itemex;
	tie->mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvs.hParent=hd->parent ? hd->parent->treeItem : TVI_ROOT;
	tvs.hInsertAfter=TVI_SORT;
	tie->iImage=tie->iSelectedImage=Ico[hd->ico].index[0];
    tie->lParam = (LPARAM) hd;
    tie->pszText = (char*)hd->name.c_str();
	hd->treeItem = (HTREEITEM)SendMessage(hwnd , TVM_INSERTITEM , 0 , (long)&tvs);
	return hd->treeItem;
}
// **************************************************
cHistDir * hist_makeTree(HWND hwnd , cHistDir * parent , string dir , bool shared);
cHistDir * hist_makeTree(HWND hwnd , cHistDir * parent , string dir , bool shared) {
//   if (!tItem) {IMLOG("! no ITEM!");return 0;}
   cXML XML;
   string path = (!shared?(profileDir + HISTDIR) : string(HISTSETTINGSDIR)) + dir;
   string name = "";
   IMLOG("path = %s" , path.c_str());
   WIN32_FIND_DATA fd;
   HANDLE hFile;
   BOOL found;
   found = ((hFile = FindFirstFile((path+"*.*").c_str(),&fd))!=INVALID_HANDLE_VALUE);
   while (found)
   {
      if (!(found = FindNextFile(hFile , &fd))) break; // jesli koniec , to zakoncz
      if (!strcmp(fd.cFileName , "..")) continue; // pomijamy niepotrzebne

      cHistDir * hd = 0;

      // Sprawdza czy istnieje juz w profilu
      if (shared && !access((string(profileDir)+HISTDIR+string(dir)+fd.cFileName).c_str() , 4)) {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // Jezeli jest katalogiem to szukamy glebiej
          cHistDir * phd = 0;
          // Trzeba znalezc go w drzewie
		  for (hist_s::dirs_it hist_dirs_it=hist.dirs.begin(); hist_dirs_it!=hist.dirs.end();hist_dirs_it++)
            if ((*hist_dirs_it)->dir == dir) {phd = *hist_dirs_it;}
          // Przeglada drzewko dalej ...
          if (phd) hist_makeTree(hwnd , phd , (dir + fd.cFileName) + "\\" , shared);
        }
        continue;
      }

      name = "";
      // KATALOG
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        hd = new cHistDir(parent);
        hd->type = HDT_DIR;
        // Laduje dir.xml z katalogu wspólnego
		CStdString xmlFileName = HISTSETTINGSDIR+dir+fd.cFileName+"\\dir.xml";
		if (access(xmlFileName.c_str() , 4))
			xmlFileName = HISTSETTINGSDIR "dir.xml";
        if (!access(xmlFileName.c_str() , 4)) {
            XML.loadFile(xmlFileName.c_str());
          name = XML.getText("history/info/name");
		  if (name.empty()) name = fd.cFileName;
          hd->list = XML.getText("history/info/list"); // typ listowania
          hd->sortColumn = chtoint(XML.getText("history/info/sort").c_str()); // typ listowania
		  hd->searchDisabled = chtoint(XML.getText("history/info/search_disabled").c_str()) != 0;
		  hd->ico = chtoint(XML.getText("history/info/icon").c_str());

		  // Laduje specyfikacje naglowkow dla katalogu
          string headers = XML.getContent("history/info/headers");
          if (headers == "") {
             hd->headers = parent?parent->headers:0;
          } else {
            hd->headers = new cHistHeaders;
            hd->headers_created=true;
            hd->headers->load(headers.c_str());
          }
          // Laduje specyfikacje naglowkow dla podkatalogow
          headers = XML.getContent("history/directory/headers");
          if (headers != "") {
            hd->dirheaders = new cHistHeaders;
            hd->dirheaders->load(headers.c_str());
          }
        } // xml
		else { // Gdy katalog jest bez XML'a...
			if (parent) { // kopiujemy nag³ówki historii
				if (parent->headers) {
					hd->headers = new cHistHeaders();
					*hd->headers = *parent->headers;
				}
				if (parent->dirheaders) {
					hd->dirheaders = new cHistHeaders();
					*hd->dirheaders = *parent->dirheaders;
				}
				hd->list = parent->list;
				hd->sortColumn = parent->sortColumn;
			}
		}
		if (!hd->ico)
			hd->ico = IDI_HIST_DIR;

        // PLIKI  *.*
      } else /*if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)*/ {
		  // Jezeli nie zadna inna - to bedzie tu akurat ta ikonka...
          // DTB
          if (stristr(fd.cFileName , ".dtb")) {
            cHistDirDTB * hdDTB = new cHistDirDTB(parent);
            hd = dynamic_cast<cHistDir*>(hdDTB);
            hd->headers = parent?parent->dirheaders?parent->dirheaders:parent->headers:0;
            // Rozpoznawanie nazwy
            if (name=="") {
               name = fd.cFileName;
               name.erase(name.find_last_of('.')); // kasujemy rozszerzenie
			   name = urlDecode(name , '#');
               unsigned int net;
               hd->cnt = -1;
               if ((net=name.find_last_of('.'))!=name.npos) { // Szukamy numeru sieci
                  net = atoi(name.substr(net+1).c_str());
				  // Skoro mamy net, mo¿emy zmieniæ ikonkê...
		          hd->ico = UIIcon(IT_LOGO , net , 0, 0);
                  name.erase(name.find_last_of('.')); // usuwamy oznaczenie sieci
                  char chr = name[0];
				  if (strchr("upced" , chr))
					  name.erase(0 , 1);
                  if (chr=='u') {
					  hd->cnt = ICMessage(IMC_CNT_FIND , net , (int)name.c_str());
                      if (hd->cnt!=DT_NOROW && *GETCNTC(hd->cnt , CNT_DISPLAY)) name = GETCNTC(hd->cnt , CNT_DISPLAY);
                  }
               }
            }
          }
          // XML
          else if (stristr(fd.cFileName , ".xml")) {
            XML.loadFile((path + fd.cFileName).c_str());
            name = XML.getText("history/directory/name");
            // Sprawdza Source , czyli typ xml'a
			CStdString source = XML.getText("history/directory/source");
            if (source=="QUEUE") {
              cHistDirQueue * hdQ = new cHistDirQueue(parent);
              hd = dynamic_cast<cHistDir*>(hdQ);
              hdQ->net = chtoint(XML.getText("history/directory/queue/net").c_str());
              hdQ->type = chtoint(XML.getText("history/directory/queue/type").c_str());
              hdQ->flag = chtoint(XML.getText("history/directory/queue/flag").c_str());
              hdQ->noflag = chtoint(XML.getText("history/directory/queue/noflag").c_str());
            } else if (source=="SEARCH") {
				cHistSearchOptions hso;
				cXML XML_S;
				XML_S.loadSource(XML.getNode("history/directory/search").c_str());
				hso.Load(XML_S);
                cHistDirSearch * hdS = new cHistDirSearch(parent , hso);
                hd = dynamic_cast<cHistDir*>(hdS);
			}

            if (hd) { // Operacje wspolne dla wszystkich XMLi
              hd->info = XML.getText("history/directory/info");
              hd->when_empty = XML.getText("history/directory/when_empty");
              hd->sortColumn = chtoint(XML.getText("history/directory/sort").c_str());
			  hd->searchDisabled = chtoint(XML.getText("history/directory/search_disabled").c_str()) != 0;
			  hd->ico = chtoint(XML.getText("history/directory/icon").c_str());
              string headers = XML.getContent("history/directory/headers");
              if (headers == "") {
                 hd->headers = parent?parent->dirheaders?parent->dirheaders:parent->headers:0;
              } else {
                hd->headers = new cHistHeaders;
                hd->headers_created=true;
                hd->headers->load(headers.c_str());
              } // if headers==""
            } // if hd
          } // XML
		  if (hd && !hd->ico)
		  	  hd->ico = IDI_HIST_SUB;

        } // else - PLIKI
        if (!hd) continue; // Jezeli hd==0 znaczy ze znaleziony plik nie jest obslugiwany
        hist.dirs.push_back(hd);
        hd->name = name==""?string(fd.cFileName) : name;
        hd->parent = parent;
        hd->path = (path + fd.cFileName);
        hd->dir  = dir;
        hd->size = fd.nFileSizeLow;
        if (parent) parent->subItems++;

		hist_makeTreeItem(hwnd , hd);
        if (!hd->treeItem) throw "History TreeNode couldn't be created.";
        if (hd->type == HDT_DIR) { // Przesukuje dalej.
           hd->path+="\\";
           hist_makeTree(hwnd , hd , (dir + fd.cFileName) + "\\" , shared);
        }
   }
   FindClose(hFile);
   // **************** pliki

   return 0;
}

// SEARCH


void cHistSearchOptions::GetFromDialog(HWND wnd) {
	if (!wnd) wnd = hist.search.wnd;
	this->findAll = IsDlgButtonChecked(wnd , IDC_FINDALL)!=0;
	this->useCase = IsDlgButtonChecked(wnd , IDC_CASE)!=0;
	this->onlyCurrent = IsDlgButtonChecked(wnd , IDC_ONLYCURRENT)!=0;
	this->words = IsDlgButtonChecked(wnd , IDC_WORDS)!=0;
	this->regexp = IsDlgButtonChecked(wnd , IDC_REGEXP)!=0;
	GetDlgItemText(wnd , IDC_SEARCH , this->query.GetBuffer(500) , 500);
	this->query.ReleaseBuffer();
	if (IsDlgButtonChecked(wnd , IDC_RADIO1)!=0) {
		this->threadNext = this->threadPrev = 0;	
	} else if (IsDlgButtonChecked(wnd , IDC_RADIO2)!=0) {
		this->threadPrev = 3;	
		this->threadNext = -1;
	} else if (IsDlgButtonChecked(wnd , IDC_RADIO3)!=0) {
		this->threadNext = -1;
		this->threadPrev = -1;	
	} else if (IsDlgButtonChecked(wnd , IDC_RADIO4)!=0) {
		this->threadPrev = GetDlgItemInt(wnd , IDC_PREV , 0 , true);	
		this->threadNext = GetDlgItemInt(wnd , IDC_NEXT , 0 , true);
	}
	SYSTEMTIME st;
	if (DateTime_GetSystemtime(GetDlgItem(wnd, IDC_DATE_FROM) , &st) == GDT_NONE) {
		this->timeFrom = 0;
	} else {
		this->timeFrom = cTime64(st);
	}
	if (DateTime_GetSystemtime(GetDlgItem(wnd, IDC_DATE_TO) , &st) == GDT_NONE) {
		this->timeTo = 0;
	} else {
		this->timeTo = cTime64(st);
	}

}
void cHistSearchOptions::SetHistoryWindow(HWND wnd) {
	
}

void cHistSearchOptions::SetInDialog(HWND wnd) {
	if (!wnd) wnd = hist.search.wnd;
	CheckDlgButton(wnd , IDC_FINDALL , this->findAll);
	CheckDlgButton(wnd , IDC_CASE , this->useCase);
	CheckDlgButton(wnd , IDC_ONLYCURRENT , this->onlyCurrent);
	CheckDlgButton(wnd , IDC_WORDS , this->words);
	CheckDlgButton(wnd , IDC_REGEXP , this->regexp);
	SetDlgItemText(wnd , IDC_SEARCH , this->query);
	CheckDlgButton(wnd , IDC_RADIO1 , BST_UNCHECKED);
	CheckDlgButton(wnd , IDC_RADIO2 , BST_UNCHECKED);
	CheckDlgButton(wnd , IDC_RADIO3 , BST_UNCHECKED);
	CheckDlgButton(wnd , IDC_RADIO4 , BST_UNCHECKED);

	if (this->threadNext == 0 && this->threadPrev == 0) {
		CheckDlgButton(wnd , IDC_RADIO1 , BST_CHECKED);
	} else if (this->threadNext == -1 && this->threadPrev == 3) {
		CheckDlgButton(wnd , IDC_RADIO2 , BST_CHECKED);
	} else if (this->threadNext == -1 && this->threadPrev == -1) {
		CheckDlgButton(wnd , IDC_RADIO3 , BST_CHECKED);
	} else {
		CheckDlgButton(wnd , IDC_RADIO4 , BST_CHECKED);
		SetDlgItemInt(wnd , IDC_PREV , this->threadPrev , 1);
		SetDlgItemInt(wnd , IDC_NEXT , this->threadNext , 1);
	}

	cDate64 dt(this->timeFrom);
	DateTime_SetSystemtime(GetDlgItem(wnd, IDC_DATE_FROM) , this->timeFrom?GDT_VALID:GDT_NONE , &(SYSTEMTIME)dt);
	dt = this->timeTo;
	DateTime_SetSystemtime(GetDlgItem(wnd, IDC_DATE_TO) , this->timeFrom?GDT_VALID:GDT_NONE , &(SYSTEMTIME)dt);

	hist.search.ResetMRU();
}
void cHistSearchOptions::Load(cXML & xml) {
	this->findAll = chtoint(xml.getText("search/find_all").c_str()) != 0;
	this->onlyCurrent = chtoint(xml.getText("search/only_current").c_str()) != 0;
	this->query = xml.getText("search/query").c_str();
	this->regexp = chtoint(xml.getText("search/regexp").c_str()) != 0;
	this->threadNext = chtoint(xml.getText("search/thread/next").c_str());
	this->threadPrev = chtoint(xml.getText("search/thread/previous").c_str());
	this->useCase = chtoint(xml.getText("search/use_case").c_str()) != 0;
	this->words = chtoint(xml.getText("search/words").c_str()) != 0;
	this->timeFrom = _atoi64(xml.getText("search/timeFrom").c_str());
	this->timeTo = _atoi64(xml.getText("search/timeTo").c_str());
	
}

CStdString cHistSearchOptions::GetName() {
	return query.substr(0 , 40) + "...";
}
CStdString cHistSearchOptions::GetInfo() {
	CStdString info;
	if (onlyCurrent) 
		info+="Tylko w " + CStdString(currentOpen ? currentOpen->name : "aktywnym");
	else
		info+="Wszêdzie";
	if (regexp) info+=", RegExp";
	if (useCase) info+=", Wielkoœæ liter";
	if (words) info+=", Ca³e wyrazy";
	return info;
}


void hist_s::search_s::Show() {
	Reset();
	ShowWindow(this->wnd , SW_SHOW);
}
void hist_s::search_s::Reset() {
	if (this->inProgress) {
		this->Stop();
		return; // Stop tu wraca!
	}
	SetWindowText(GetDlgItem(wnd , IDOK) , "Start");
	EnableWindow( GetDlgItem(wnd , IDOK) , true);
	SendMessage(hist.search.prgrsWnd , PBM_SETPOS , 0 , 0);
}
void hist_s::search_s::ResetMRU() {
	// £adujemy z MRU...
    HWND combo = (HWND) GetDlgItem(wnd , IDC_SEARCH);
	CStdString current;
	GetDlgItemText(wnd , IDC_SEARCH , current.GetBuffer(500) , 500);
	current.ReleaseBuffer();

	sMRU MRU;
    MRU.name = "UI/history/search";
    MRU.buffSize = 500;
    MRU.count = 10;
    MRU.values = new char * [10];
    int i;
    for (i = 0; i<10; i++) 
        MRU.values[i] = new char [500];
    MRU.count = Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_GET , &MRU));
	while (SendMessage(combo , CB_GETCOUNT , 0 , 0))
		SendMessage(combo , CB_DELETESTRING , 0 , 0);
    for (i=0; i<MRU.count; i++)
        SendMessage(combo , CB_ADDSTRING , 0 , (LPARAM)MRU.values[i]);
    for (i = 0; i<10; i++) 
        delete MRU.values[i];
    delete [] MRU.values;
	SetDlgItemText(wnd , IDC_SEARCH , current);
}
void hist_s::search_s::Stop() {
	if (!this->inProgress) return;
	this->state = stCancel;
	this->inProgress = false;
	this->Reset();
}
void hist_s::search_s::Next() {
	this->canGetNext = true;
}
void hist_s::search_s::Start() {
	this->Stop();
	if (!this->state == stReady) return;
	this->opt.GetFromDialog(wnd);
	this->state = stPreparing;
	if (opt.query.size() > 0 && !opt.regexp && opt.query.size() < 2) {
		MessageBox(wnd , "Zbyt krótkie zapytanie!" , "Przeszukiwanie historii" , MB_ICONERROR);
		this->state = stReady;
		return;
	}
	sMRU MRU;
	MRU.count = 10;
	MRU.flags = MRU_SET_LOADFIRST | MRU_GET_USETEMP;
	MRU.current = opt.query.c_str();
	MRU.name = "UI/history/search";
	Ctrl->IMessage(&sIMessage_MRU(IMC_MRU_SET , &MRU));
	ResetMRU();

	CStdString pattern = opt.query;
	if (pattern.empty() == false) {
		if (opt.regexp == 0) {
			pattern = preg.replace("/[\\/\\?\\[\\]().,!^$:+*|\\\\]/" , "\\$0" , pattern);
			if (opt.words) 
				pattern = "\\b" + pattern + "\\b";
			pattern = preg.replace("/ +/" , (this->opt.words ? "\\b|\\b" : "|") , pattern);
			pattern = "/" + pattern + "/s";
			if (!this->opt.useCase)
				pattern += "i";
		} else {
			if (!preg.match("/^([\\/#^%!]).+\\1[imsxADXU8BDE]*$/" , pattern))
				pattern = "/" + pattern + "/s";
		}
		preg.setPattern(pattern);
		preg.setSubject(" ");
		preg.match();
		if (preg.result < 0 && preg.lastError) {
			CStdString err;
			err.Format("Wzorzec zawiera b³¹d!\r\nWzorzec: %s\r\nZnak:%d\r\nKomunikat:%s" , pattern , preg.lastErrorPos , preg.lastError);
			MessageBox(wnd , err , "Przeszukiwanie historii" , MB_ICONERROR);
			this->state = stReady;
			return;
		}
	}
	// Szukamy pasuj¹cego, ju¿ istniej¹cego zapytania
	cHistDirSearch * found = 0;
	for (hist_s::dirs_it dir = hist.dirs.begin(); dir != hist.dirs.end(); dir++) {
		if ((*dir)->type != HDT_SEARCH) continue;
		if ((static_cast<cHistDirSearch*>(*dir))->opt == this->opt) {
			found = static_cast<cHistDirSearch*>(*dir);
			break;
		}
	}
	// Je¿eli nie ma, tworzymy nowy...
	if (!found) {
		// Szukamy katalogu /search/
		cHistDir * searchDir = 0;
		for (hist_s::dirs_it it = hist.dirs.begin(); it != hist.dirs.end(); it++) {
			if ((*it)->type == HDT_DIR &&  (*it)->path == HISTSETTINGSDIR "search\\") {
				searchDir = *it;
				break;
			}
		}
		if (!searchDir) {
			MessageBox(wnd , "Nie ma definicji dla katalogu historii /search/. Upewnij siê, czy masz odpowiedni¹ wersjê ca³ego programu!" , "Przeszukiwanie historii" , MB_ICONERROR);
			this->state = stReady;
			return;
		}

		found = new cHistDirSearch(searchDir , this->opt);
		found->dir = searchDir->dir;
		found->headers = searchDir->dirheaders;
		found->name = this->opt.GetName();
		found->info = this->opt.GetInfo();
		found->parent->subItems++;
		found->ico = IDI_HIST_QUERY;
		hist.dirs.push_back(found);
		hist_makeTreeItem(GetDlgItem(hwndHistory , IDC_TREE) , found);
	}
	found->itemsClear();
	found->preg = preg;
	cHistDir * lastOpened = hist.dir_opened;
	found->select();
	this->foundDir = found;
	this->inProgress = true;
	this->state = stSearch;

	EnableWindow(GetDlgItem(this->wnd , IDOK) , false);
	SetWindowText(GetDlgItem(this->wnd , IDOK) , "ZnajdŸ nastêpny");
    hist.SetStatus("Szukam...");

	// ok, mo¿emu "ju¿" szukaæ
	if (this->opt.onlyCurrent) {
		cHistDir * searchIn = lastOpened;
		if (lastOpened == found) {
			if (found->opt.currentOpen)
				searchIn = found->opt.currentOpen;
			else if (found->parent && found->parent->canSearch())
				found->opt.currentOpen = searchIn = found->parent;
		} else {
			found->opt.currentOpen = searchIn;
		}
		/* trzeba zaktualizowaæ info... */
		found->info = this->opt.GetInfo();

		SendMessage(hist.search.prgrsWnd , PBM_SETRANGE32 , 0 , searchIn->size ? searchIn->size : 100);
		searchIn->search();
		SendMessage(hist.search.prgrsWnd , PBM_SETPOS , searchIn->size ? searchIn->size : 100 , 0);
	} else {
		SendMessage(hist.search.prgrsWnd , PBM_SETPOS , 0 , 0);
		
		int size = 0;
		for (hist_s::dirs_it it = hist.dirs.begin(); it != hist.dirs.end(); it++) {
			size += (*it)->size ? (*it)->size : 100;
		}
		SendMessage(hist.search.prgrsWnd , PBM_SETRANGE32 , 0 , size);
		size = 0;
		for (hist_s::dirs_it it = hist.dirs.begin(); it != hist.dirs.end(); it++) {
			if (!IsActive()) break;
			(*it)->search();
			size += (*it)->size ? (*it)->size : 100; // dla pewnoœci ustawiamy rozmiar...
			SendMessage(hist.search.prgrsWnd , PBM_SETPOS , size , 0);
			Ctrl->WMProcess();
		}
	}
	if (found->count() == 0 && this->state == stSearch) {
		MessageBox(wnd , "Nic nie znalaz³em" , "Przeszukiwanie archiwum" , MB_ICONINFORMATION);
	}
	if (opt.findAll) {
		found->select();
		//found->fill();
	}
    hist.SetStatus(0);
	if (this->state != stCancel)
		EnableWindow(GetDlgItem(this->wnd , IDOK) , false); // blokujemy...
	this->inProgress = false;
	this->state = stReady;
}

LRESULT hist_s::search_s::WndProc(HWND wnd , int message , WPARAM wParam, LPARAM lParam) {
   switch (message)
   {
		case WM_INITDIALOG: {
			hist.search.prgrsWnd = GetDlgItem(wnd , IDC_PROGRESS1);
			CheckDlgButton(wnd , IDC_RADIO1 , true);
			SetDlgItemText(wnd , IDC_PREV , "3");
			SetDlgItemText(wnd , IDC_NEXT , "3");
			DateTime_SetFormat(GetDlgItem(wnd, IDC_DATE_FROM), "dd'-'MM'-'yyyy' 'hh':'mm");
			DateTime_SetFormat(GetDlgItem(wnd, IDC_DATE_TO), "dd'-'MM'-'yyyy' 'hh':'mm");
			cDate64 dt(_time64(0) - 7*24*60*60);
			DateTime_SetSystemtime(GetDlgItem(wnd, IDC_DATE_FROM) , GDT_VALID , &(SYSTEMTIME)dt);
			DateTime_SetSystemtime(GetDlgItem(wnd, IDC_DATE_FROM) , GDT_NONE , 0);
			DateTime_SetSystemtime(GetDlgItem(wnd, IDC_DATE_TO) , GDT_NONE , 0);

			break;}
        case WM_CLOSE:
            ShowWindow(wnd , SW_HIDE);
            break;
        case WM_NCDESTROY:
			hist.search.Stop();
            break;
        case WM_COMMAND:
			switch(HIWORD(wParam)) {
	        case BN_CLICKED: // Control
				switch(LOWORD(wParam)) {
				case IDOK:
					if (hist.search.inProgress)
						hist.search.Next();
					else 
						hist.search.Start();
					break;
				case IDCANCEL:
					if (hist.search.inProgress) {
						hist.search.state = stCancel;
						hist.search.Stop();
					} else
						ShowWindow(wnd , SW_HIDE);
					break;
				default:
					hist.search.Reset();
                }
                break;
			case EN_CHANGE: case CBN_EDITCHANGE: case CBN_SELCHANGE:
//				   if (LOWORD(wParam) == IDC_SEARCH) {
					  hist.search.Reset();
//				   }
				   break;
             } return 0;     
        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
			 if (nm->code == DTN_DATETIMECHANGE) {
				  hist.search.Reset();
			 }
             return 0;

   }
   return 0;
}

// -------------------------------------------------------------------------------
void onSizeHistory(int w , int h) {
  SendMessage(hist.statusWnd , WM_SIZE , 0,MAKELPARAM(w,h));

  HDWP hDwp;
  RECT rc , rc2;
//  if (!w && !h) {
  GetClientRect(hwndHistory , &rc);
//  }
  GetClientRect(hist.tbWnd , &rc2);
  int barHeight = rc2.bottom;
  rc.top += rc2.bottom + 2;
  rc.left+=2;
  rc.right-=2;
  GetClientRect(hist.statusWnd , &rc2);
  rc.bottom -= rc2.bottom + 2;

  w = rc.right - rc.left;
  h = rc.bottom - rc.top;

  hDwp=BeginDeferWindowPos(4);
  #define H_SPACE 0
  int W_TREE = (int)min(w / 4 , 200);
  int H_LIST = (int)min(h / 2.5 , 250);

  hDwp=DeferWindowPos(hDwp , GetDlgItem(hwndHistory , IDC_TREE) ,0,
         rc.left,rc.top, W_TREE ,h,SWP_NOZORDER);
  hDwp=DeferWindowPos(hDwp , hist.listWnd ,0,
         rc.left + W_TREE + 2,rc.top, w - W_TREE - 2 , H_LIST ,SWP_NOZORDER);
  if (hist.msgControl)
	  hDwp=DeferWindowPos(hDwp , hist.msgControl->hwnd ,0,
		     rc.left + W_TREE + 2, rc.top + H_LIST + H_SPACE  , w - W_TREE - 2 , h - H_LIST - H_SPACE  ,SWP_NOZORDER);
//  hDwp=DeferWindowPos(hDwp , hist.tbWnd ,0,
//         rc.left, rc.top  , w , barHeight ,SWP_NOZORDER);

  GetChildRect(hist.statusWnd , &rc2);
  hDwp=DeferWindowPos(hDwp , hist.prgrsWnd ,HWND_TOP,
         rc2.left + 205,rc2.top+4, 90 , 16 , SWP_NOZORDER);

  #undef H_SPACE
  EndDeferWindowPos(hDwp);
  RedrawWindow(hist.prgrsWnd,0,0,RDW_UPDATENOW|RDW_VALIDATE|RDW_INVALIDATE);

}


// -----------------------------

void HistoryDialogPrepare(HWND hwnd);
void HistoryDialogPrepare(HWND hwnd) {
   HWND item;
   hist.listWnd = item = GetDlgItem(hwnd , IDC_LIST);
   ListView_SetImageList(item , Ico.iml[0] , LVSIL_SMALL);
   SendMessage(item , LVM_SETEXTENDEDLISTVIEWSTYLE , 0 ,
                      LVS_EX_FULLROWSELECT);


   item = GetDlgItem(hwnd , IDC_TREE);
   TreeView_SetImageList(item , Ico.iml[0] , TVSIL_NORMAL);

   hist.statusWnd = CreateStatusWindow(SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
    "Historia",hwnd,IDC_STATUSBAR);
   int sbw [3]={/*200 , 300 ,*/ -1};
   SendMessage(hist.statusWnd , SB_SETPARTS , 1 , (LPARAM)sbw);
   hist.prgrsWnd = CreateWindowEx(0 , PROGRESS_CLASS , "" , WS_CHILD | WS_VISIBLE|PBS_SMOOTH
                   , 210 , 270 , 80 , 16 , hwnd , (HMENU)IDC_PROGRESS , hInst , 0);
   SendMessage(hist.prgrsWnd , PBM_SETSTEP , 1 , 0);
   // ****** TOOLBAR

   hist.tbWnd = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL,
        WS_CHILD | WS_CLIPCHILDREN |WS_CLIPSIBLINGS | WS_VISIBLE
        |TBSTYLE_TRANSPARENT
        |CCS_NODIVIDER
        | TBSTYLE_FLAT
//        | TBSTYLE_LIST
//        | CCS_NOPARENTALIGN
//        | CCS_NORESIZE
        | TBSTYLE_TOOLTIPS
        , 0, 0, 500, 30, hwnd,
        (HMENU)IDC_TOOLBAR, hInst, 0);
     // Get the height of the toolbar.
     SendMessage(hist.tbWnd , TB_SETEXTENDEDSTYLE , 0 ,
                       TBSTYLE_EX_MIXEDBUTTONS
                       /*| TBSTYLE_EX_HIDECLIPPEDBUTTONS*/
                       );
     SendMessage(hist.tbWnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
     SendMessage(hist.tbWnd, TB_SETIMAGELIST, 0, (LPARAM)hist.iml);
     // Set values unique to the band with the toolbar.
     TBBUTTON bb;
     bb.iBitmap=0;
     bb.idCommand=IDI_HISTB_REFRESH;
     bb.fsState=TBSTATE_ENABLED;
     bb.fsStyle=BTNS_AUTOSIZE | BTNS_BUTTON;
//TBSTYLE_SEP
     bb.iString=(int)"Odœwie¿";
     bb.dwData=0;
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.iBitmap=1;
     bb.idCommand=IDI_HISTB_SHRED;
     bb.iString=(int)"Kasuj wszystko";
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.fsStyle=BTNS_AUTOSIZE | BTNS_SEP;
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.fsState=0;
     bb.fsStyle=BTNS_AUTOSIZE | BTNS_BUTTON;
     bb.iBitmap=2;
     bb.idCommand=IDI_HISTB_DEL;
     bb.iString=(int)"Usuñ";
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.iBitmap=6;
     bb.idCommand=IDI_HISTB_SAVE;
     bb.iString=(int)"Zapisz";
     // ukryte
     bb.fsState=0;
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.iBitmap=3;
     bb.idCommand=IDI_HISTB_PRINT;
     bb.iString=(int)"Drukuj";
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.fsStyle=BTNS_AUTOSIZE | BTNS_SEP;
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
//     bb.fsState=TBSTATE_ENABLED;
     bb.fsStyle=BTNS_AUTOSIZE | BTNS_BUTTON;
     bb.iBitmap=4;
	 bb.fsState = TBSTATE_ENABLED;
     bb.idCommand=IDI_HISTB_SEARCH;
     bb.iString=(int)"Szukaj";
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.fsState=TBSTATE_ENABLED;
     bb.iBitmap=5;
     bb.idCommand=IDI_HISTB_RESEND;
     bb.iString=(int)"Przeœlij";
     SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb);
     bb.iBitmap=7;
     bb.idCommand=IDI_HISTB_COMPACT;
     bb.iString=(int)"Kompaktuj";
	 IMLOG("- Historia - przycisk kompaktowania %d",
		SendMessage(hist.tbWnd, TB_INSERTBUTTON, 0x3FFF , (LPARAM)&bb)
		);
	 


     SendMessage(hist.tbWnd, TB_AUTOSIZE, 0, 0);


}

void hist_s::dialogFill(HWND hwnd) {
   dirs.clear();
   dir_opened = 0;
   item_opened = 0;
   selected=0;
   headers = 0;
   HWND item = GetDlgItem(hwnd , IDC_TREE);
   hist_makeTree(item , 0 , "" , false);
   hist_makeTree(item , 0 , "" , true);
   IMLOG("- History filled");
   running = true;
}

int CALLBACK HistoryDialogProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
//   static char str_buff2 [MAX_STRBUFF];
   int a;

   switch (message)
   {
        case WM_INITDIALOG:
                hwndHistory = hwnd;
                hist.hwnd = hwnd;
                SendMessage(hwnd , WM_SETICON , ICON_SMALL , (LPARAM)Ico[IDI_WND_HISTORY].ico);
                SendMessage(hwnd , WM_SETICON , ICON_BIG , (LPARAM)Ico[IDI_WND_HISTORY].ico_);
				HistoryDialogPrepare(hwnd);
                hist.dialogFill(hwnd);
                Act[IMIG_HISTORYWND].ghandle = hwnd;
                Act.Mod.loadWindow(Act[IMIG_HISTORYWND]);
                onSizeHistory(0,0);
                break;
        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
        case WM_NCDESTROY:
			DestroyWindow(hist.search.wnd);
                Act.Mod.saveWindow(Act[IMIG_HISTORYWND]);
                Act[IMIG_HISTORYWND].ghandle = 0;
                hwndHistory = 0;
                hist.changeDT(0);
                delete hist.msgControl;
                break;
        case WM_SIZING:
             clipSize(wParam , (RECT *)lParam , 400 , 300);
             return 1;
        case WM_SIZE:
//             IMessage(IMC_LOG , 0,0,(int)"WM_SIZE" , wParam);
             onSizeHistory(LOWORD(lParam) , HIWORD(lParam));
             return 1;
        case WM_COMMAND:
             switch(HIWORD(wParam)) {
               case BN_CLICKED: // Control
                  switch(LOWORD(wParam)) {
                    case IDI_HISTB_REFRESH :
                      hist.refresh(true);
                      break;
                    case IDI_HISTB_SHRED:
                      hist.shred();
                      break;
                    case IDI_HISTB_DEL:
                      hist.eraseSelected();
                      break;
                    case IDI_HISTB_SAVE:
                      hist.saveSelected();
                      break;
                    case IDI_HISTB_PRINT:
                      hist.printSelected();
                      break;
                    case IDCANCEL:
                      DestroyWindow(hwnd);
                      break;
                    case IDI_HISTB_RESEND:
                        ICMessage(IMC_MESSAGEQUEUE , (int)&sMESSAGESELECT(NET_BC , 0 , -1 , MF_SEND));
                        break;
                    case IDI_HISTB_COMPACT:
                        hist.compact();
						break;
					case IDI_HISTB_SEARCH:
						hist.search.Show();
						break;
                  }
                  break;
             } return 0;     
        case WM_NOTIFY:
             NMHDR * nm;nm=(NMHDR*)lParam;
             NMTREEVIEW * nmtv;
             NMLISTVIEW * nmlv;
//             curCnt = 0;
             switch (nm->idFrom) {
               case IDC_TREE:
                 switch (nm->code) {
                   case TVN_DELETEITEM:
                     nmtv=(NMTREEVIEW*)lParam;
                     if (nmtv->itemOld.lParam) {
                       hist.removeFromDirs((cHistDir*)nmtv->itemOld.lParam);
                       delete (cHistDir*)nmtv->itemOld.lParam;
                     }  
                     break;
                   case NM_CLICK: {
                       HTREEITEM hti = TreeView_GetSelection(nm->hwndFrom);
                       if (hti) {
                            TVITEMEX ie;
                            ie.hItem = hti;
                            ie.mask = TVIF_PARAM;
                            TreeView_GetItem(nm->hwndFrom , &ie);
                            hist.selected = (cHist*)ie.lParam;
                       } else hist.selected = 0;
					   hist.selectedList = IDC_TREE;
					   hist.resetOpenedItem();
                       hist.checkErase();
                       hist.checkSave();
	   				   hist.search.Reset();
                       break;}
                   case TVN_SELCHANGED:
                       if (!hist.running) break;
                     nmtv=(NMTREEVIEW*)lParam;
                     if (!nmtv->itemNew.lParam) return 0;
                     ((cHist*)nmtv->itemNew.lParam)->prepare();
                     hist.selected = (cHist*)nmtv->itemNew.lParam;
					 hist.selectedList = IDC_TREE;
				 	 hist.resetOpenedItem();
                     hist.checkErase();
                     hist.checkSave();
//                     ((cHist*)nmtv->itemNew.lParam)->open();
                     break;
                 } break;
               case IDC_LIST:
					switch(nm->code) {
				    case LVN_COLUMNCLICK:
						nmlv = (NMLISTVIEW *)lParam;
						if (!hist.dir_opened) 
							break;
						if (abs(hist.dir_opened->sortColumn) == nmlv->iSubItem+1)
							hist.dir_opened->sortColumn = -hist.dir_opened->sortColumn;
						else
							hist.dir_opened->sortColumn = nmlv->iSubItem+1;
						hist.sort();
						break;
					case LVN_GETDISPINFO: {
						hist.GetDispInfo((NMLVDISPINFO*)lParam);
						break;}
					case LVN_SETDISPINFO: {
						hist.SetDispInfo((NMLVDISPINFO*)lParam);
						break;}
					case LVN_ODSTATECHANGED: {
						hist.ODStateChanged((NMLVODSTATECHANGE*)lParam);
						break;}
					case LVN_DELETEALLITEMS:
						hist.listClear();
						break;
					case LVN_DELETEITEM:
						nmlv = (NMLISTVIEW *)lParam;
						if ((unsigned)nmlv->iItem < hist.itemsOnList.size())
							hist.itemsOnList.erase(hist.itemsOnList.begin() + nmlv->iItem);
						break;

					case LVN_ITEMCHANGED: {
						nmlv = (NMLISTVIEW *)lParam;
						if (!hist.running || !(nmlv->uChanged & LVIF_STATE)) break;
//						IMLOG("STATECHANGE: i = %d st = %d old = %d cur = %d" , nmlv->iItem , nmlv->uNewState , nmlv->uOldState , ListView_GetItemState(nm->hwndFrom , nmlv->iItem , 0xF));
						if (nmlv->iItem < 0) {
							for (hist_s::itemsOnList_it it = hist.itemsOnList.begin(); it != hist.itemsOnList.end(); it++) {
								(*it)->listState =  nmlv->uNewState;
							}
						} else if (nmlv->iItem < hist.itemsOnList.size()) {
							hist.itemsOnList[nmlv->iItem]->listState = ListView_GetItemState(nm->hwndFrom , nmlv->iItem , 0xF);
						}
						if (!(nmlv->uNewState & LVIS_SELECTED)) break;
//                   case LVN_KEYDOWN:
						}
					case NM_CLICK:
					case NM_DBLCLK: {
						a = ListView_GetSelectionMark(nm->hwndFrom);
						if (a < 0 || a >= hist.itemsOnList.size()) break;
						cHist * ch = hist.itemsOnList[a];
						hist.selected = ch;
						hist.selectedList = IDC_LIST;
						hist.checkErase();
						hist.checkSave();
						if (ch->type < HDT_ITEM && nm->code != NM_DBLCLK) break;
						ch->prepare();
	//                     ch->open();
						break;}
				} break;  
               case IDC_MSG:
                 switch(nm->code) {
                 case EN_LINK:
                   return UIMessageProc(0,hwnd , message , wParam , lParam);
                 } break;  

                    

             }
             return 0;
	case WM_ENTERIDLE:
	    return UIMessageProc(0,hwnd , message , wParam , lParam);

	// do UI
	case WM_DRAWITEM: case WM_MEASUREITEM:
		return UIMessageProc(0, hwnd, message, wParam, lParam);

   }
   return 0;
}

void hist_start(int cnt) {hist.start(cnt);}
void hist_init() {hist.init();}
