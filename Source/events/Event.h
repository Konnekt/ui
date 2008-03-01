/**
  *  Generic Event class
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

#ifndef __EVENT_H__
#define __EVENT_H__

namespace Konnekt {
  class Event : public SharedObject<iSharedObject> {
  public:
    /**
     * Class version macro
     */
    STAMINA_OBJECT_CLASS_VERSION(Event, iSharedObject, Version(0,1,0,0));

  public:
    /**
     * Constructs a new Event.
     *
     * @param id     The event id
     * @param void*  The subject
     */
    Event(int id, void * subject = 0): _value(0), _processed(false), _id(id), _subject(subject) { }

    /**
     * Constructs a empty Event object.
     *
     * @param id     The event id
     * @param void*  The subject
     */
    Event(): _value(0), _processed(false), _id(0), _subject(0) { }

    /**
     * Returns string represantation of the event
     *
     * @return string 
     */
    virtual inline string toString() const {
      string str = stringf("id = %i", getID());

      // add return value if event was processed
      if (isProcessed()) {
        str += stringf(", returnValue = %i", getReturnValue());
      }
      return str;
    }

  public:
    /**
     * Returns the subject.
     *
     * @return void* The subject
     */
    inline void * getSubject() const {
      return _subject;
    }

    /**
     * Returns the event id.
     *
     * @return id The event id
     */
    inline int getID() const {
      return _id;
    }

    /**
     * Sets the return value for this event.
     *
     * @param int The return value
     */
    inline void setReturnValue(int value) {
      _value = value;
    }

    /**
     * Returns the return value.
     *
     * @return int The return value
     */
    inline int getReturnValue() const {
      return _value;
    }

    /**
     * Sets the processed flag.
     *
     * @param bool The processed flag value
     */
    inline void setProcessed(bool processed) {
      _processed = processed;
    }

    /**
     * Returns whether the event has been processed by a listener or not.
     *
     * @return bool true if the event has been processed, false otherwise
     */
    inline bool isProcessed() const {
      return _processed;
    }

  protected:
    /**
     * Sets the ID for this event.
     *
     * @param int The new ID
     */
    inline void setID(int id) {
      _id = id;
    }

  protected:
    int _value;
    bool _processed;
    void * _subject;
    int _id;
  };

  // smart pointer type
  typedef SharedPtr<Event> oEvent;
}

#endif // __EVENT_H__