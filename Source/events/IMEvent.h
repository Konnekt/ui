/**
  *  IMessage Event class
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

#ifndef __IMEVENT_H__
#define __IMEVENT_H__

#include "Event.h"

namespace Konnekt {
  class IMEvent : public Event {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(IMEvent, Event, Version(0,1,0,0));

  public:
    /**
     * Constructs a new IMEvent.
     *
     * @param sIMessage_base*
     */
    IMEvent(sIMessage_base* msgBase): Event(msgBase->id, msgBase), _im(static_cast<sIMessage*>(msgBase)) { }

  public:
    /**
     * Returns pointer to sIMessage structure.
     *
     * @return sIMessage*
     */
    inline sIMessage* getIMessage() const {
      return _im;
    }

    /**
     * Returns sender plugin ID.
     *
     * @return tPluginId
     */
    inline tPluginId getSender() const {
      return getIMessage()->sender;
    }

    /**
     * Returns sender plugin name.
     *
     * @return String
     */
    inline String getSenderName() const {
      return SAFECHAR((char*) Ctrl->IMessageDirect(IM_PLUG_NAME, getSender()));
    }

    /**
     * Returns param 1.
     *
     * @return int
     */
    inline int getP1() const {
      return getIMessage()->p1;
    }

    /**
     * Returns param 2.
     *
     * @return int
     */
    inline int getP2() const {
      return getIMessage()->p2;
    }

    /**
     * Sets the return value to success (1).
     */
    inline void setSuccess() {
      setReturnValue(1);
    }

    /**
     * Sets the return value to failure (-1).
     */
    inline void setFailure() {
      setReturnValue(-1);
    }

    /**
     * Returns true if return value is set to '1'
     */
    inline bool isSuccess() {
      return getReturnValue() == 1;
    }

    /**
     * Returns true if return value is set to '-1'
     */
    inline bool isFailure() {
      return getReturnValue() == -1;
    }

    /**
     * Sets the return value for this event.
     *
     * @param int The return value
     */
    inline void setReturnValue(int value) {
      Event::setReturnValue(value);
      Event::setProcessed(true);
    }

    /**
     * Sets the return value for this event.
     *
     * @param const StringRef& The return value
     *
     * @warning this method uses internal buffer which can be overridden by
     * commonly used functions like @c GETSTR in all flavors.
     */
    inline void setReturnValue(const StringRef& value) {
      // creating temp buffer
      char* buff = (char*) Ctrl->GetTempBuffer(value.size() + 1);

      // cleaning memory
      memset(buff, 0, value.size() + 1);

      // copying data
      memcpy(buff, value.a_str(), value.size());

      // setting return value to pointer to internal, temp buffer
      setReturnValue((int) buff);
    }

  protected:
    /**
     * Sets pointer to sIMessage structure.
     *
     * @param sIMessage*
     */
    inline void setIMessage(sIMessage* im) {
      _im = im;
    }

  protected:
    sIMessage* _im;
  };

  // smart pointer type
  typedef SharedPtr<IMEvent> oIMEvent;
}

#endif // __IMEVENT_H__