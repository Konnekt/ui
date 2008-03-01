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

    c.setColumn(tableConfig, CFG_UIHISTORY_XMLHEADER, ctypeString, "<?xml-stylesheet type=\"text/xsl\" href=\"%HistoryXSL%\"?>", "history/XMLHeader");
    c.setColumn(tableConfig, CFG_UIHISTORY_XMLXSL, ctypeString, "%KonnektData%\\history\\exported renderer.xsl", "history/XMLHeader/XSLPath");
    c.setColumn(tableConfig, CFG_UIHISTORY_XMLPRINTXSL, ctypeString, "%KonnektData%\\history\\print.xsl", "history/XMLHeader/XSLPrintPath");
    c.setColumn(tableConfig, CFG_UIHISTORY_XMLFULL, ctypeInt, 1, "history/XMLFullExport");
    c.setColumn(tableConfig, CFG_UIHISTORY_MARKFOUND, ctypeInt, 1, "history/markFound");
  }

  void Controller::_initialize(IMEvent& ev) {
    ev.setSuccess();
  }

  void Controller::_prepareUI(IMEvent& ev) {
    Ctrl->ICMessage(IMI_ICONRES, ico::main, IDI_MAIN);

    ev.setSuccess();
  }

  void Controller::_addMessage(IMEvent& ev) {
    ev.setSuccess();
  }
}
