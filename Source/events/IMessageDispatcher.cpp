/**
  *  IMessage Dispatcher class
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

#include "IMessageDispatcher.h"
#include "IMEvent.h"

namespace Konnekt {
  /**
   * Dispatch incoming IMessage
   *
   * @param sIMessage_base*
   *
   * @return oEvent
   */
  oEvent IMessageDispatcher::dispatch(sIMessage_base* msgBase) {
    // create event object
    oIMEvent ev = new IMEvent(msgBase);

    // get imessage id
    int id = ev->getID();

    // look for static values
    if (_static_values.find(id) != _static_values.end()) {
      ev->setReturnValue(_static_values[id]);
    } else {
      // notify global listeners
      notify(*ev);
    }

    if (Ctrl) {
      if (ev->isProcessed()) {
        // log IMessage
        Ctrl->logMsg(logEvent, "IMessageDispatcher", "dispatch", ev->toString().c_str());
      } else {
        // set error if no result
        Ctrl->setError(errorNoResult);
      }
    }
    // return event
    return ev;
  }
}