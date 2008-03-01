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

namespace KImage {
  Controller::Controller() {
    IMessageDispatcher& d = getIMessageDispatcher();
    ActionDispatcher& a = getActionDispatcher();
    Config& c = getConfig();

    d.setStaticValue(IM_PLUG_TYPE, imtUI | imtConfig | imtAllMessages);
    d.setStaticValue(IM_PLUG_PRIORITY, priorityHigh);
    d.setStaticValue(IM_PLUG_NAME, (int) "Kronos");
    d.setStaticValue(IM_PLUG_SIG, (int) sig);
    d.setStaticValue(IM_PLUG_NET, net);

    d.connect(IM_UI_PREPARE, bind(&Controller::_initialize, this, _1));
    d.connect(IM_UI_PREPARE, bind(&Controller::_prepareUI, this, _1));
    // d.connect(IM_ALLPLUGINSINITIALIZED, bind(&Controller::_validate, this, _1));

    a.connect(UI::ACT::msg_ctrlsend, bind(&Controller::_msgCtrlSend, this, _1));
    a.subclass(UI::ACT::msg_ctrlsend);

    c.setColumn(tableConfig, cfg::dragndrop, ctypeInt, 1, "KImage/dragndrop");
  }

  void Controller::_initialize(IMEvent& ev) {
    ev.setSuccess();
  }

  void Controller::_prepareUI(IMEvent& ev) {
    Ctrl->ICMessage(IMI_ICONRES, ico::logo, IDI_LOGO);

    ev.setSuccess();
  }

  void Controller::_msgCtrlSend(ActionEvent& ev) {
    if (!Config::get(cfg::dragndrop).to_i()) {
      return ev.forward();
    }
    ev.forward();
  }
}
