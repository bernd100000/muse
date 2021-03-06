//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: conf.h,v 1.4.2.1 2006/09/28 19:22:25 spamatica Exp $
//
//  (C) Copyright 1999-2003 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//=========================================================

#ifndef __CONF_H__
#define __CONF_H__

#include "ui_configmidifilebase.h"

#include <QDialog>

#include "xml.h"

// REMOVE Tim. newdrums. Added.
// Adds the ability to override at instrument level.
// But it just makes things too complex for the user.
// And in a way is unnecessary and overkill, since we
//  already allow modifying an instrument.
//#define _USE_INSTRUMENT_OVERRIDES_

namespace MusEGui {

//---------------------------------------------------------
//   MidiFileConfig
//    config properties of exported midi files
//---------------------------------------------------------

class MidiFileConfig : public QDialog, public Ui::ConfigMidiFileBase {
      Q_OBJECT

   private slots:
      void okClicked();
      void cancelClicked();

   public:
      MidiFileConfig(QWidget* parent=0);
      void updateValues();
      };

} // namespace MusEGui

namespace MusECore {
extern bool readConfiguration();
extern bool readConfiguration(const char *configFile);
extern void readConfiguration(Xml& xml, bool doReadMidiPortConfig, bool doReadGlobalConfig);
extern void writeConfigurationColors(int level, MusECore::Xml& xml, bool partColorNames = true);
}

#endif

