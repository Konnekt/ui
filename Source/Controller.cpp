/**
  *  Kronos Controller class
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2002-2008 Stamina
  *  @copyright    Copyright (c) 2008 Sijawusz Pur Rahnama
  *  @link         svn://konnekt.info/kronos/
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy: sija $
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#include "stdafx.h"
#include "Controller.h"
#include "HistoryTable.h"
// #include "HistoryWnd.h"

namespace Kronos {
  Controller::Controller() {
    IMessageDispatcher& d = getIMessageDispatcher();
    ActionDispatcher& a = getActionDispatcher();
    Config& c = getConfig();

    // IMT_ALL & ~(IMT_MSGUI | IMT_NETSEARCH | IMT_NETUID);
    d.setStaticValue(IM_PLUG_TYPE, imtUI | imtConfig | imtAllMessages);
    d.setStaticValue(IM_PLUG_PRIORITY, priorityHigh);
    d.setStaticValue(IM_PLUG_NAME, (int) "Kronos");
    d.setStaticValue(IM_PLUG_SIG, (int) sig);
    d.setStaticValue(IM_PLUG_NET, net);

    d.connect(IM_UI_PREPARE, bind(&Controller::_initialize, this, _1));
    d.connect(IM_UI_PREPARE, bind(&Controller::_prepareUI, this, _1));

    d.connect(IMI_HISTORY_ADD, bind(&Controller::_addMessage, this, _1));

    c.setColumn(tableConfig, cfg::xmlHeader, ctypeString, "<?xml-stylesheet type=\"text/xsl\" href=\"%HistoryXSL%\"?>", "history/XMLHeader");
    c.setColumn(tableConfig, cfg::xmlXSL, ctypeString, "%KonnektData%\\history\\exported renderer.xsl", "history/XMLHeader/XSLPath");
    c.setColumn(tableConfig, cfg::xmlPrintXSL, ctypeString, "%KonnektData%\\history\\print.xsl", "history/XMLHeader/XSLPrintPath");
    c.setColumn(tableConfig, cfg::xmlFull, ctypeInt, 1, "history/XMLFullExport");
    c.setColumn(tableConfig, cfg::markFound, ctypeInt, 1, "history/markFound");
  }

  void Controller::_initialize(IMEvent& ev) {
    HistoryTable* h = HistoryTable::getInstance();
    h->init();

    ev.setSuccess();
  }

  void Controller::_prepareUI(IMEvent& ev) {
    /* Registering icons */
    IconRegister((IML_enum) IML_16_32, ico::main, (HINSTANCE) 0, IDI_MAIN);
    IconRegister((IML_enum) IML_16_32, ico::dir, (HINSTANCE) 0, IDI_HIST_DIR);
    IconRegister((IML_enum) IML_16_32, ico::sub, (HINSTANCE) 0, IDI_HIST_SUB);
    IconRegister((IML_enum) IML_16_32, ico::search,(HINSTANCE)  0, IDI_HIST_QUERY);
    IconRegister((IML_enum) IML_16_32, ico::refresh,(HINSTANCE)  0, IDI_HISTB_REFRESH);
    IconRegister((IML_enum) IML_16_32, ico::shred,(HINSTANCE)  0, IDI_HISTB_SHRED);
    IconRegister((IML_enum) IML_16_32, ico::delete_, (HINSTANCE) 0, IDI_HISTB_DEL);
    IconRegister((IML_enum) IML_16_32, ico::print,(HINSTANCE)  0, IDI_HISTB_PRINT);
    IconRegister((IML_enum) IML_16_32, ico::search,(HINSTANCE)  0, IDI_HISTB_SEARCH);
    IconRegister((IML_enum) IML_16_32, ico::resend,(HINSTANCE)  0, IDI_HISTB_RESEND);
    IconRegister((IML_enum) IML_16_32, ico::save, (HINSTANCE) 0, IDI_HISTB_SAVE);
    IconRegister((IML_enum) IML_16_32, ico::compact, (HINSTANCE) 0, IDI_HISTB_COMPACT);

    /* Cfg */
    UIGroupAdd(IMIG_CFG_UI_MSG, cfg::cfg, ACTS_GROUP, "Historia", ico::main);

    /* Plugin info box */
    char header[400];
    sprintf_s<400>(header, "<span class='note'>Powered by: <b>%s</b></span><br/>"
      "<span class='note'>Skompilowano: <b>%s</b> [<b>%s</b>]</span><br/>"
      "Informacje o wtyczce na forum Konnekta "
      "(http://konnekt.info/forum/)<br/><br/>"
      "Copyright © 2002-2008 <b>Stamina</b><br/>"
      "Copyright © 2008 <b>KPlugins Team</b>",
      "X", __DATE__, __TIME__);

    char desc[] = "Wszystkie wiadomoœci które s¹ odbierane b¹dŸ wysy³ane zostaj¹ zapisane w historii. Dziêki temu mo¿na pó¿niej przeczytaæ starsze rozmowy, czy wyszukaæ konkretne informacje w nich zawarte, takie jak np. adresy stron czy emaile.";

    UIActionCfgAddPluginInfoBox2(cfg::cfg, desc, header, "res://dll/107.ico", -4);
    UIActionAdd(cfg::cfg, 0, ACTT_GROUP, "");

    if (ShowBits::checkBits(ShowBits::showInfoNormal)) {
      UIActionCfgAdd(cfg::cfg, 0, ACTT_TIPBUTTON | ACTSC_INLINE, AP_TIP "" AP_TIPIMAGEURL "file://%KonnektData%\\img\\ui_historywindow.png" AP_ICONSIZE "16");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_COMMENT, "Jak korzystaæ z historii");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_SEPARATOR);
    }

    UIActionCfgAdd(cfg::cfg, 0, ACTT_CHECK, "Zapisuj wiadomoœci w historii", CFG_LOGHISTORY);
    if (ShowBits::checkLevel(ShowBits::levelNormal)) {
      UIActionCfgAdd(cfg::cfg, 0, ACTT_CHECK, "Podkreœlaj znaleziony tekst", CFG_UIHISTORY_MARKFOUND);
    }
    UIActionCfgAdd(cfg::cfg, 0, ACTT_GROUPEND);

    if (ShowBits::checkLevel(ShowBits::levelAdvanced)) {
      UIActionCfgAdd(cfg::cfg, 0, ACTT_GROUP, "Export do XMLa");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_COMMENT, "Nag³ówek wstawiany do XMLa:");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_TEXT, "" AP_TIP "Mo¿esz u¿ywaæ zmiennych œrodowiskowych jak %HistoryXSL% czy %KonnektPath%", cfg::xmlHeader);
      UIActionCfgAdd(cfg::cfg, 0, ACTT_COMMENT | ACTSC_INLINE, "Plik XSL do formatowania exportu");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_EDIT | ACTSC_FULLWIDTH, "" AP_TIP "Mo¿esz u¿ywaæ zmiennych œrodowiskowych jak %KonnektData% czy %KonnektPath%", cfg::xmlXSL);
      UIActionCfgAdd(cfg::cfg, 0, ACTT_COMMENT | ACTSC_INLINE, "Plik XSL do formatowania wydruku");
      UIActionCfgAdd(cfg::cfg, 0, ACTT_EDIT | ACTSC_FULLWIDTH, "" AP_TIP "Mo¿esz u¿ywaæ zmiennych œrodowiskowych jak %KonnektData% czy %KonnektPath%", cfg::xmlPrintXSL);
      UIActionCfgAdd(cfg::cfg, 0, ACTT_CHECK, "Zapisuj pe³ne informacje o wiadomoœciach", cfg::xmlFull);
      UIActionAdd(cfg::cfg, 0, ACTT_GROUPEND);
    }

    UIActionAdd(IMIG_MSGTB, act::historyCnt, 0, "Historia", ico::main);
    UIActionInsert(IMIG_CNT, act::historyCnt, UIActionGetPos(IMIG_CNT, IMIA_CNT_INFO) + 1, 0 ,"Historia", ico::main);
    UIActionAdd(IMIG_MAIN_CNT, act::historyMain, 0 ,"Historia rozmów", ico::main);

    /* History window */
    UIGroupAdd(0, ui::historyWindow, ACTS_GROUP | ACTR_SETCNT , "IMIG_HISTORYWND");
    UIActionAdd(ui::historyWindow, UI::ACT::msg_ctrlview, ACTT_HWND | ACTR_SETCNT, "UI::ACT::msg_view");
    // oWindow w = HistoryWindow::create("Historia", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 100, 100, 500, 200, (HWND)0);

    ev.setSuccess();
  }

  void Controller::_addMessage(IMEvent& ev) {
    ev.setSuccess();
  }
}
