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

#pragma once

#ifndef __IMESSAGEDISPATCHER_H__
#define __IMESSAGEDISPATCHER_H__

#include "EventDispatcher.h"
#include "IMEvent.h"

namespace Konnekt {
  class IMessageDispatcher : public EventDispatcher<IMEvent> {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(IMessageDispatcher, EventDispatcher<IMEvent>, Version(0,1,0,0));

  public:
    typedef hash_map<int, int> tStaticValues;

  public:
    /**
     * Constructs a new IMessageDispatcher object.
     */
    inline IMessageDispatcher() {
      // set default values for common messages, some of them should be overriden
      setStaticValue(IM_PLUG_NETNAME, 0);
      setStaticValue(IM_PLUG_PRIORITY, 0);
      setStaticValue(IM_PLUG_SDKVERSION, KONNEKT_SDK_V);
      setStaticValue(IM_PLUG_VERSION, 0);
      setStaticValue(IM_PLUG_CORE_V, (int) "W98");
      setStaticValue(IM_PLUG_UI_V, 0);
    }

  public:
    /**
     * Sets the static value for a given IMessage id
     *
     * @param int IMessage id
     * @param int value
     */
    inline void setStaticValue(int id, int value) {
      _static_values[id] = value;
    }

    /**
     * Dispatch incoming IMessage
     *
     * @param sIMessage_base*
     *
     * @return oEvent
     */
    oEvent dispatch(sIMessage_base* msgBase);

  protected:
    tStaticValues _static_values;
  };
}

#endif // __IMESSAGEDISPATCHER_H__