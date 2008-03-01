/**
  *  Action Dispatcher class
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2005-2008 Sijawusz Pur Rahnama
  *  @link         svn://konnekt.info/kaway2/ kAway2 plugin SVN Repo
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy: sija $
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#include "stdafx.h"

#include "ActionDispatcher.h"
#include "ActionEvent.h"

namespace Konnekt {
  /**
   * Dispatches incoming Action IMessage
   *
   * @param sIMessage_base*
   *
   * @return oEvent
   */
  oEvent ActionDispatcher::dispatch(sIMessage_base* msgBase) {
    // create event object
    oActionEvent ev = new ActionEvent(msgBase, *this);

    // notify action listeners
    notify(*ev);

    try {
      if (ev->getSubclassInfo().autoForward()) {
        ev->forward(); // forward to previous owner
      }
    } catch (NotSubclassedException&) { }

    // log IMessage
    if (Ctrl) {
      Ctrl->logMsg(logEvent, "ActionDispatcher", "dispatch", ev->toString().c_str());
    }
    // return event
    return ev;
  }
}