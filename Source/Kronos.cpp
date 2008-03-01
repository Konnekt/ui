/**
  *  Kronos main file
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

using namespace Kronos;

int __stdcall DllMain(void* hinstDLL, unsigned long fdwReason, void* lpvReserved) {
  return true;
}

int __stdcall IMessageProc(sIMessage_base* msgBase) {
  return Controller::getInstance()->dispatch(msgBase);
}