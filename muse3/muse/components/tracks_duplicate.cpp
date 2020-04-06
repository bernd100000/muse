//=========================================================
//  MusE
//  Linux Music Editor
//    tracks_duplicate.cpp
//  (C) Copyright 2011 Tim E. Real (terminator356 on sourceforge.net)
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

#include "tracks_duplicate.h"

namespace MusEGui {
  
DuplicateTracksDialog::DuplicateTracksDialog(
  bool audio, bool /*midi*/, bool /*drum*/, bool newdrum,
  QWidget* parent,
  bool copies, bool allRoutes, bool defaultRoutes,
  bool noParts, bool duplicateParts, bool copyParts, bool cloneParts
  )
  : QDialog(parent)
{
  setupUi(this);
  
  standardCtrlsCheckBox->setVisible(audio);
  pluginsCheckBox->setVisible(audio);
  pluginCtrlsCheckBox->setVisible(audio);
  copyDrumlistCheckBox->setVisible(newdrum);

  copiesSpinBox->setVisible(copies);
  copiesLabel->setVisible(copies);
  
  allRoutesRadioButton->setVisible(allRoutes);
  defaultRoutesRadioButton->setVisible(defaultRoutes);
  routesFrame->setVisible(allRoutes || defaultRoutes);
  
  noPartsRadioButton->setVisible(noParts);
  duplicatePartsRadioButton->setVisible(duplicateParts);
  copyPartsRadioButton->setVisible(copyParts);
  clonePartsRadioButton->setVisible(cloneParts);
  partsFrame->setVisible(noParts || duplicateParts || copyParts || cloneParts);

  connect(okPushButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(reject()));
}

} // namespace MusEGui
