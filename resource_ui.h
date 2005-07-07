#include "resource.h"

#define IDS_INF_CNTRESET  100
#define IDS_URL                         12
#define IDS_URL_SUPPORT                 14
#define IDS_URL_FORUM                 15
#define IDS_URL_DOWNLOADINFO                 16
#define IDS_APPNAME                     10

//#define IDI_MAIN_ICON  1  //
#define IDI_TRAY       2  //logo w trayu

#define IDI_BLANK      "BLANK"


#define IDI_WND_MAIN   60
#define IDI_WND_CONFIG   61
#define IDI_WND_INFO   62
#define IDI_WND_MSG   63
#define IDI_WND_SEARCH   64
#define IDI_WND_HISTORY  65
#define IDI_WND_MSG_NEW   66
#define IDI_WND_HISTORY_SEARCH  67

#define IDI_TB_MAIN    101
#define IDI_TB_ADD    102
#define IDI_TB_OPTIONS    103
#define IDI_TB_PLUGINS    104

#define IDI_CLFILTER_ON      220
#define IDI_CLFILTER_OFF     221
#define IDI_CLFILTER_OFFLINE 222
#define IDI_CLFILTER_OTHERS  223


#define IDI_NET_NONE    300

#define IDI_ST_OFFLINE    310
#define IDI_ST_ONLINE     311
#define IDI_ST_AWAY       312
#define IDI_ST_COMPOSING  313
#define IDI_ST_NA  314
#define IDI_ST_UNKNOWN  315
#define IDI_ST_CHAT  316
#define IDI_ST_DND  317
#define IDI_ST_HIDDEN  318
#define IDI_ST_BLOCKING  319

#define IDI_NEW_USER      0x200

#define IDI_MT_MESSAGE    400
#define IDI_MT_MAIL    401
#define IDI_MT_SMS    402
#define IDI_MT_SERVEREVENT    403
#define IDI_MT_FILE    404
#define IDI_MT_URL    405

#define IDI_TIP_MAIL   500
#define IDI_TIP_PHONE  501
#define IDI_TIP_NAME   502
#define IDI_TIP_REASON 503
#define IDI_TIP_INFO 504

#define IDI_HIST_DIR  600
#define IDI_HIST_SUB  601
#define IDI_HIST_QUERY 602

#define IDI_HISTB_REFRESH 610
#define IDI_HISTB_SHRED 611
#define IDI_HISTB_DEL 612
#define IDI_HISTB_PRINT 613
#define IDI_HISTB_SEARCH 614
#define IDI_HISTB_RESEND 615
#define IDI_HISTB_SAVE 616
#define IDI_HISTB_COMPACT 617

#define IDI_OVR_SMS   410
#define IDI_OVRB_SMS   411
#define IDI_OVR_MAIL   412
#define IDI_OVRB_MAIL   413
#define IDI_OVR_INFO 414
#define IDI_OVRB_INFO 415

#define IDI_OVR_NOTINLIST 416
#define IDI_OVR_IGNORE 417
#define IDI_OVR_HIDEMYSTATUS 418
#define IDI_OVR_ACTIVE 419
#define IDI_OVR_HIDESHISSTATUS 420
#define IDI_OVR_HIDEBOTH 421

#define IDI_NFO_SAVE 700
#define IDI_NFO_CANCEL 701
#define IDI_NFO_REFRESH 702


#define IDA_LONG       100
#define IDA_LONGRECV       101
#define IDA_LONGSEND       102
#define IDA_LONGINET       103

// Dialogs
#define IDD_INFO 100
  #define IDB_SAVE     100
  #define IDB_CANCEL   101
  #define IDB_REFRESH  102
  #define IDB_RESET    103
  #define IDG_NET      301
  #define IDG_DISPLAY  302
  #define IDE_DISPLAY  22
  #define IDE_NET      10
  #define IDE_UID      12
  #define IDE_CELLPHONE  13
  #define IDE_PHONE    14
  #define IDE_EMAIL    15
  #define IDE_INFO     16
  #define IDE_COUNTRY  17
  #define IDE_CITY     18
  #define IDE_NAME     19
  #define IDE_SURNAME 20
  #define IDE_NICK     21
  #define IDE_BORN     23
  #define IDE_GENDER   24
  #define IDE_GROUP    25

  #define IDC_STATIC   200
  #define IDC_STATIC2   202
  #define IDC_STATIC3   203
  #define IDC_STATIC4   204
  #define IDC_STATIC5   205
  #define IDC_STATIC6   206
  #define IDC_STATIC7   207
  #define IDC_STATIC8   208
  #define IDC_STATIC9   209
  #define IDC_STATIC10   210
  #define IDC_STATIC11   211
  #define IDC_STATIC12   212
  #define IDC_STATIC13   213
  #define IDC_STATIC14   214


#define IDD_MSG 101
  #define IDC_EDIT 251
  #define IDC_BAR  123
  #define IDC_MSG  250
#define IDD_CONFIG 102
  #define IDAPPLY      10
  #define IDG_GROUP  100
  #define IDC_TREE   101
  #define IDC_TIP 99
  #define IDC_TOOLTIP  120

// internal controls
#define IDC_LIST  1000
#define IDC_TAB   1001

#define IDD_PASS  105
#define IDD_LOGIN  106
#define IDD_SETPASS  107
  #define IDC_CHECK 125
  #define IDC_PASS1 126
  #define IDC_PASS2 127
#define IDD_ENTER 108

#define IDD_LONGPROC 109
  #define IDC_ANIMATE 128
  #define IDC_PROGRESS 129

#define IDD_ABOUT    110
#undef  IDC_ICON
#define IDC_ICON    130
#define IDD_SEARCH   111
#define IDD_SEARCH_2   120

#define IDD_GROUPS   112
#define IDD_IGNORE   113

#define IDD_SERVEREVENT 114

  #define IDC_COMBO 131
#define IDD_HISTORY  116
#define IDD_HISTORY_SEARCH  119

  #define IDC_STATIC1 132

#define IDD_BUTTONS 118



#define IDC_STATUSBAR 133
#define IDC_TOOLBAR 134
#define IDD_DBGACTIONS 135

#define IDD_GETTOKEN  140
#define IDC_INFO 141
#define IDC_TOKEN 142
// debug

//#define IDB_REFRESH 100

