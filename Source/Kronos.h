/**
  *  Kronos header file
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

#pragma once

#ifndef __KRONOS_H__
#define __KRONOS_H__

namespace Kronos {
  const unsigned int net = 6;
  const char sig[] = "KRONOS";

  namespace api {
    const unsigned int api = net * 1000 + IM_USER;

  }

  namespace im {
    const unsigned int im = net * 1000 + IM_USER + 100;

    const unsigned int historyAdd = im + 10;

  }

  namespace ui {
    const unsigned int ui = net * 1000;

    const unsigned int historyWindow = ui + 1;

  }

  namespace ico {
    const unsigned int ico = net * 1000 + 100;

    const unsigned int main = ico + 1;
    const unsigned int compact = ico + 2;
    const unsigned int delete_ = ico + 3;
    const unsigned int dir = ico + 4;
    const unsigned int print = ico + 5;
    const unsigned int refresh = ico + 6;
    const unsigned int resend = ico + 7;
    const unsigned int save = ico + 8;
    const unsigned int search = ico + 9;
    const unsigned int shred = ico + 10;
    const unsigned int sub = ico + 11;
  }

  namespace act {
    const unsigned int act = net * 1000 + 200;

    const unsigned int historyCnt = act + 1;
    const unsigned int historyMain = act + 2;
  }

  // dynamiczne akcje
  namespace dynAct {
    const unsigned int dynAct = net * 1000 + 300;

  }

  namespace cfg {
    const unsigned int cfg = net * 1000 + 500;

    const unsigned int logHistory = cfg + 1;
    const unsigned int markFound = cfg + 2;

    const unsigned int xmlHeader = cfg + 10;
    const unsigned int xmlXSL = cfg + 11;
    const unsigned int xmlPrintXSL = cfg + 12;
    const unsigned int xmlFull = cfg + 13;
  }
}

#endif // __KRONOS_H__
