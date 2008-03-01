/*

  Konnekt UI

  Nag³ówek ui_actions.cpp

*/

#include "include\Richhtml.h"

void HTMLInfoStyleCB(const CStdString & token , const CStdString & styleClass , cRichEditFormat::cSetStyle & ss);

UIEXTERN ToolTipX tooltip;

#include <hash_map>

class cUIAction {
  public:
   int type;   // typ przypisanej kontrolki
   unsigned int index;
   unsigned int parent; // identyfikator rodzica
   unsigned int id;     // identyfikator
   unsigned int cnt;
   class cUIGroup * pparent; // wskaznik do rodzica
   string txt; // text
//   string tip; // tip
   string params; // dodatkowe parametry...
   int status; // flagi statusu
   int exstyle;  // dodatkowe flagi
   void * handle; // uchwyt
   tPluginId owner;  // identyfikator wtyczki , ktora dodala akcje
   int userParam;  // parametr uzytkownika
   int p1,p2;  // p1 - parametr typu grupy (np. konfiguracji)
               // w menu - ikonka , w cfg - kolumna do automatycznego zapisywania
               // p2 - parametr kontrolki
   short w,h;
   #ifdef __DEBUG
   HTREEITEM dbg;
   #endif
   class cUIGroup * getRoot();
   class cUIGroup & getGroup();

   sUIActionNotify notify(unsigned int code , int notify1,int notify2);
   int call(unsigned int code , int notify1 ,int notify2,int cnt=AC_CURRENT);
   int call(sUIActionNotify_base * an);
   virtual void getInfo(sUIActionInfo * ai);
   virtual void setInfo(sUIActionInfo & ai, bool update=true);

   void setType(int t) {type&=~ACT_MASK; type|=t & ACT_MASK;}
   int getType() {return type & ACT_MASK;}
   inline int statusType() {return status & ACTM_TYPE;}
   virtual bool setCnt(unsigned int _cnt , bool recursive=false);
   sUIAction act() {return sUIAction(parent , id , cnt);}
   sUIACTION act(int _cnt) {return sUIAction(parent , id , _cnt);}

   ToolTipX::iTip * getTipObject(bool useText=false);
   bool hasTipObject(bool useText=false);


   CStdString getParam(const string & name) {return GetActParam(this->params, name);}
   void setParam(const string & name, const string & value) {this->params = SetActParam(this->params, name, value);}
   bool paramExists(const string & name) {return ActParamExists(this->params, name);}
   void addParams(const string & add);

   cUIAction();
   virtual ~cUIAction() {}
};

class cUIAction_cfg: public cUIAction {
   public:
   short x,y;
   cUIAction_cfg() {x=y=0;}
   virtual void getInfo(sUIActionInfo * ai);
   virtual void setInfo(sUIActionInfo & ai, bool update=true);
   void setComboItems(const char * txt);
   void setComboValue(const char * txt);
   void getComboValue(char * txt , size_t size);

   void setRadio(const char * txt, bool update=false);
   void setRadioValue(const char * txt);
   void getRadioValue(char * txt , size_t size);

   void setSlider(const char * txt, bool update=false);
   void setSliderValue(const char * txt);
   void getSliderValue(char * txt , size_t size);

   void setSpinner(const char * txt, bool update=false);

   void setButton(const char * txt, bool update=false);

   void setTimeValue(const char * txt);
   void getTimeValue(char * txt , size_t size);
   void setImage(const char * txt, bool update=false);
};


class cUIGroup : public cUIAction {
  public:
  int count;
  typedef deque <cUIAction*> tAct;
  tAct Act;
  typedef deque <cUIAction*>::iterator t_Act_it;
  typedef t_Act_it Act_it_t ;
  cUIGroup ():cUIAction() {count=0;handle=ghandle=0;owner=(tPluginId)0;}
  ~cUIGroup();
  cUIAction & operator [](int pos);
  cUIAction & operator ()(int id);

  int size() {return Act.size();}

  cUIAction * insert (int id , int pos=-1 , const char * txt="", int status=0,int p1=0 , short w = 0 , short h = 0 , int p2=0, int param=0 , int exstyle=0 , tPluginId owner = pluginUI);
  cUIAction * cfginsert (int id , int pos=-1 , const char * txt="", int status=0,int p1=0 , short x=0 , short y=0 , short w = 0 , short h = 0 , int p2=0, int param=0 , int exstyle=0 , tPluginId owner = pluginUI);
  cUIAction * insert (sUIActionInfo * ai , tPluginId owner);
  void move(int old_pos , int pos); // Przenosi akcje na nowa pozycje
  int remove (int pos);
  bool setCnt(unsigned int _cnt  , bool recursive=false);
  HANDLE ghandle;

  void setInfo(sUIActionInfo & ai, bool update=true);
  
  int find (int id); // Znajduje podakcje i podaje jej pozycje
  cUIAction * findAction (int id); // Znajduje podakcje szukajac wglab
  int findIndex (int index); // Znajduje podakcje i podaje jej pozycje
  int findHandle (void * handle);
  bool exists(int id) {return find(id)!=-1;} // Sprawdza czy posiada taka akcje
  cUIAction * getIndex (int index); // Pobiera poprzez indeks
  // zwraca domyslna akcje (ACTN_DEFAULT)
  cUIAction * getDefaultAction(void);
};

// -------------------------------------------
// Tablica modyfikacji
#define AMOD_ACTION    0  // 64
#define AMOD_MASK      1
#define AMOD_STATUS_AND 2
#define AMOD_STATUS_OR 3
#define AMOD_STYLE_AND 4
#define AMOD_STYLE_OR  5
#define AMOD_POS       6
#define AMOD_TITLE     7
#define AMOD_P1        8
#define AMOD_XY        9
#define AMOD_WH        10

// Maski nalozonych modyfikacji
#define MODM_STATUS    1
#define MODM_STYLE     2
#define MODM_POS       4
#define MODM_TITLE     8
#define MODM_P1        0x10
#define MODM_XY        0x20
#define MODM_WH        0x40


class cUIActMod {
public:
    cUIActMod();/*:loaded(false)*/ 
    void load(); // Laduje ustawienia do pamieci
    void save(); // Zapisuje ust. do pliku
    class CdTable * DT; // Tablica z danymi
    int createEntry(cUIAction & act);
    int getEntry(cUIAction & act);


    void changeStatus(cUIAction & act , int mask , int value);

    /* TODO: Przy okazji zamykania akcji w klasy, to tez MUSI byc zmienione! */

    // Zapamietywanie pozycji...
    
    void loadWindow(cUIGroup & grp); // ustawia okno
    void saveWindow(cUIGroup & grp); // zapamietuje okno
    void loadAction(cUIAction & act);
    void loadBar(cUIGroup & grp); // ustawia caly toolbar
    void saveBar(cUIGroup & grp); // zapisuje caly toolbar


private:
    //bool loaded;
};


typedef stdext::hash_map <int , cUIGroup*>::iterator t_Grp_it;
typedef stdext::hash_map <int , cUIGroup*>::reverse_iterator t_Grp_rit;

class cUIActions {
  public:
  stdext::hash_map <int , cUIGroup*> Grp;
  cUIActMod Mod; // Zmiany wprowadzone przez uzytkownika
  // zwraca wskaznik do grupy
  unsigned int lastIndex;
  cUIActions(); 
  ~cUIActions();
  cUIGroup & operator [](int n);
  cUIAction & operator ()(sUIAction & uia);
  cUIAction & operator ()(int id);
  cUIAction & operator ()(int parent , int id) {return (*this)[parent](id);}
  cUIAction * getIndex (int index);
  cUIAction * findId (int id);

  cUIGroup * getRoot() {return Grp[0];}
  bool exists(int parent_id) {return Grp.find(parent_id)!=Grp.end();}
  bool exists(int parent , int id) {return exists(parent) && Grp[parent]->find(id)>=0;}
  bool exists(sUIAction & act) {return exists(act.parent) && Grp[act.parent]->find(act.id)>=0;}
  cUIAction * insert (sUIActionInfo * ai , tPluginId owner) {
    if (!exists(ai->act.parent)) return 0;
    return Grp[ai->act.parent]->insert(ai , owner);
  }
/*  cUIAction * set (sUIActionInfo * ai , int owner) {
    Grp[ai->act.parent].insert(ai , owner);
  }
  */
  int size() {return Grp.size();}

  // ustawia / dodaje grupe
//  set(int n,cUIGroup & v) {return Grp[n]=v;}

  // zwraca/ustawia akcje
//  bool get(int g , int id , cUIAction & val); // id grupy , id akcji , referencja
//  bool set(int g , int id , cUIAction & val); // id grupy , id akcji , referencja

//  bool addGroup(int g , int id , string txt);
//  bool addAction(int g , int id, string txt , int pos=-1,bool after=0);

};



// ----------------------------------------


class sUIIcon {
//ImageList_Add
  public:
  HBITMAP bmp;
  HICON ico , ico_;
  int index [4];
  HIMAGELIST * iml;

  bool isIcon;
  sUIIcon();
  ~sUIIcon();
  int draw(HDC hDC , int x , int y , int style=0 , int state=0 , int rop=0 , int frame=0 , COLORREF effect=0);
  int draw(IML_enum what , HDC hDC , int x , int y , int style=0 , int state=0 , int rop=0 , int frame=0 , COLORREF effect=0);
  int measure(int * w , int * h);
  int measure(IML_enum what , int * w , int * h);
  IML_enum getType();
  HICON getIcon();

};



typedef map <int , sUIIcon>::iterator Ico_it_t;

class cUIIcons {
  public:
  map <int , sUIIcon> Ico;
//  pair <int , cUIIcon> Ico_pair;
  cUIIcons();
  ~cUIIcons();
  sUIIcon & operator [] (int);
  bool find (int);
  int count (int id);
  bool isAnim(int id) {return this->find(id | 1);}
  int erase (int id);
  int iconRes(int id , int res , HINSTANCE inst=0 , IML_enum target=IML_16 , sUIIconRegister::tIconParams * ip=0);
  int iconRes(int id , const char * res=0 , HINSTANCE inst=0 , IML_enum target=IML_16 , sUIIconRegister::tIconParams * ip=0);
  int bmpRes(int id , int res=0 , HINSTANCE inst=0 ,IML_enum target=IML_16);
  bool iconAdd(IML_enum target , string URL);
  bool iconRegister(sUIIconRegister * ir , HINSTANCE hDll=0);
  bool iconRegisterList(sUIIconRegisterList * irl);
  bool iconExists(int ID , IML_enum target);
  bool iconUnregister(int ID , IML_enum target);
  HANDLE iconGet(int ID , IML_enum target);
  int getIconIndex(int ID , IML_enum target);
  HIMAGELIST getImageList(IML_enum target);
  void addHandleToList(sUIIcon & _ico , int type , int imlIndex , HANDLE handle , HBITMAP mask);
  static HANDLE getImageFromURL(string URL , int & imageType , HINSTANCE dll=0 , sUIIconRegister::tIconParams * ip=0);
  HIMAGELIST iml [4];
//  HBITMAP getBmp();
//  doneBmp();
};


class TargetAction: public ToolTipX::Target {
public:
	TargetAction():Target() {_act = 0; _useText = false;}
	ToolTipX::TipPtr getTip(ToolTipX * ttx);
	bool operator==(const iTarget & b) const {return false;}
	const char *getClass() const {return "TargetAction";}
protected:
	cUIAction * _act;
	bool _useText;
};

class TargetActionRebar: public TargetAction {
public:
	bool mouseAtTarget(ToolTipX * ttx, HWND window, POINT screen, POINT local);
};

class TargetActionTipButton: public ToolTipX::TargetFixed {
public:
	TargetActionTipButton(ToolTipX::TipPtr tip, HWND button):ToolTipX::TargetFixed(tip) {
		this->_button = button;
	}
//	ToolTipX::enPositioning getPositioning() {return (ToolTipX::enPositioning)(ToolTipX::positionFixed | ToolTipX::positionRefresh);}
	void attach(ToolTipX * ttx);
	void detach(ToolTipX * ttx);
	const char * getClass() {return "TargetActionTipButton";}
	bool operator==(const iTarget & b) const {return iTarget::operator==(b) && static_cast<const TargetActionTipButton&>(b)._button == _button;}
protected:
	HWND _button;
};


class TipAction: public ToolTipX::iTipCallBack {
public:
	TipAction(cUIAction & act):_act(&act){}
	void fillCache();
private:
	cUIAction * _act;
};


class TipActionTaskBar: public ToolTipX::iTipCallBack {
public:
	TipActionTaskBar(int id):_id(id){}
	void fillCache();
private:
	int _id;
};

class TargetActionTaskbar: public ToolTipX::Target {
public:
	TargetActionTaskbar(int id):Target(ToolTipX::TipPtr(new TipActionTaskBar(id))), _id(id){  }
	bool mouseAtTarget(ToolTipX * ttx, HWND window, POINT screen, POINT local);
	void onHide(ToolTipX * ttx);
	void onShow(ToolTipX * ttx);
	ToolTipX::enPositioning getPositioning() {return ToolTipX::positionFixed;}
	int getDelay(ToolTipX * ttx) {return ttx->enterWait * 2;}
private:
	int _id;
	POINT _screen;
};




//---------------------------------------------
// Zmienne globalne

UIEXTERN cUIActions Act;
UIEXTERN cUIIcons Ico;

//---------------------------------------------
// F-CJE

int CALLBACK	PropClassProc(HWND, UINT, WPARAM, LPARAM);
void resizeProp(HWND hwndParent);
void clipPropSize(HWND hwndParent, int edge, RECT & rc, int minW, int minH, int maxW=0x7FFF, int maxH=0x7FFF);
void clipPropSize(cUIGroup & g, int edge, RECT & rc, int minW, int minH, int maxW=0x7FFF, int maxH=0x7FFF);

void openCfgAction(cUIGroup & g);
HWND makeToolBar(HWND hwndOwner , cUIGroup & g);
void saveCfg(cUIAction * a = 0);
HMENU makeActionsMenu(cUIGroup &  , int startFrom=0 , bool create=true);
void makeCfgTree(cUIGroup & g);
String checkCfgValue(cUIAction & a, const StringRef& v);
void setCfgValue(cUIAction & a , StringRef v);
String getCfgValue(cUIAction & a);
String getActionValue(cUIAction & act, bool convert = false);
String setActionValue(cUIAction & act , StringRef buff , bool convert = false);
int ActionStatus(sUIAction & a , int status=-1 , char * text=0 , int ico=-1);


#ifdef __DEBUG
    void DbgActionsSet(cUIAction * act , bool showup);
#endif




 int fillProfiles(int where);
 HMENU makeActionsMenu(cUIGroup & g , int startFrom , bool create);
 void destroyActionsMenu(cUIGroup & g);
void makeCfgTree(cUIGroup & g , HWND tree);

String getCfgValue(cUIAction & a);

void setCfgValue(cUIAction & a , StringRef v);

String checkCfgValue(cUIAction & a,const StringRef& v);

HWND getCfgHandle(cUIAction & act);
HWND makeCfgProp(cUIGroup & g );
void openCfgAction(cUIGroup & g);

void destroyCfgTree(cUIGroup * g = 0);
void destroyActionsTree(cUIGroup & g);


void saveCfg(cUIAction * a);

void SetCfgTip(HWND hwnd , const char * tip , void * handle);

HWND makeToolBar(HWND hwndOwner , cUIGroup & g);

HWND makeBar(HWND hwndOwner , cUIGroup & g);

int ActionSet(sUIActionInfo & ai);

int ActionStatus(sUIAction & act , int status, char * text , int ico);

 void makeActions();

 void makeCfg();

 void destroyActions();


INT_PTR CALLBACK ConfigProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam);
int CALLBACK PropClassProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
cUIAction * UIPopupMenu(cUIGroup & act , UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT *prcRect , int start = 0);
int UIMessageProc(int curCnt , HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam);
int CALLBACK ReBarControlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);



void DbgActionsRefresh(cUIGroup * start);
INT_PTR CALLBACK DbgActionsProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

void registerActions();
void registerActionsAfter();
void unregisterActions(); 

HICON makeIconFromBitmap(HBITMAP bmp);
