//=========================================================
//  MusE
//  Linux Music Editor
//    $Id: midieditor.cpp,v 1.2.2.2 2009/02/02 21:38:00 terminator356 Exp $
//  (C) Copyright 1999 Werner Schweer (ws@seh.de)
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

#include "midieditor.h"
#include "midiedit/ecanvas.h"
#include "waveedit/waveview.h"
#include "scrollscale.h"
#include "mtscale.h"
#include "xml.h"
#include "part.h"
#include "track.h"
#include "song.h"

#include "trackinfo_layout.h"
#include "icons.h"
#include "mstrip.h"
#include "gconfig.h"
#include "app.h"

#include <QRect>
#include <QColor>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>

namespace MusEGui {

//---------------------------------------------------------
//   MidiEditor
//---------------------------------------------------------

MidiEditor::MidiEditor(ToplevelType t, int r, MusECore::PartList* pl,
   QWidget* parent, const char* name) : TopWin(t, parent, name)
      {
      setAttribute(Qt::WA_DeleteOnClose);
      _pl = pl;
      if (_pl)
            for (MusECore::iPart i = _pl->begin(); i != _pl->end(); ++i)
                  _parts.insert(i->second->sn());
      _raster  = r;
      canvas   = 0;
      
      trackInfoWidget = 0;
      selected = 0;
      
      //wview    = 0;
      _curDrumInstrument = -1;
      mainw    = new QWidget(this);
      
      ///mainGrid = new QGridLayout(mainw);
      mainGrid = new QGridLayout();
      mainw->setLayout(mainGrid);
      
      mainGrid->setContentsMargins(0, 0, 0, 0);
      mainGrid->setSpacing(0);  
      setCentralWidget(mainw);
      
      connect(MusEGlobal::song, SIGNAL(newPartsCreated(const std::map< const MusECore::Part*, std::set<const MusECore::Part*> >&)), SLOT(addNewParts(const std::map< const MusECore::Part*, std::set<const MusECore::Part*> >&)));
      }

//---------------------------------------------------------
//   genPartlist
//---------------------------------------------------------

void MidiEditor::genPartlist()
      {
      if(!_pl)
        return;

      _pl->clear();
      for (std::set<int>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
            MusECore::TrackList* tl = MusEGlobal::song->tracks();
            for (MusECore::iTrack it = tl->begin(); it != tl->end(); ++it) {
                  MusECore::PartList* pl = (*it)->parts();
                  MusECore::iPart ip;
                  for (ip = pl->begin(); ip != pl->end(); ++ip) {
                        if (ip->second->sn() == *i) {
                              _pl->add(ip->second);
                              break;
                              }
                        }
                  if (ip != pl->end())
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   genTrackInfo
//---------------------------------------------------------

void MidiEditor::genTrackInfo(TrackInfoWidget* trackInfo)
      {
      noTrackInfo          = new QWidget(trackInfo);
      noTrackInfo->setAutoFillBackground(true);
      QPixmap *noInfoPix   = new QPixmap(160, 1000);
      const QPixmap *logo  = new QPixmap(*museLeftSideLogo);
      noInfoPix->fill(noTrackInfo->palette().color(QPalette::Window) );
      QPainter p(noInfoPix);
      p.drawPixmap(10, 0, *logo, 0,0, logo->width(), logo->height());

      QPalette palette;
      palette.setBrush(noTrackInfo->backgroundRole(), QBrush(*noInfoPix));
      noTrackInfo->setPalette(palette);
      noTrackInfo->setGeometry(0, 0, 65, 200);
      noTrackInfo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));

      trackInfo->addWidget(noTrackInfo,   0);
      trackInfo->addWidget(0, 1);
      }

//---------------------------------------------------------
//   switchInfo
//---------------------------------------------------------

void MidiEditor::switchInfo(int n)
      {
      const int idx = 1;
      if(n == idx) {
            MidiStrip* w = (MidiStrip*)(trackInfoWidget->getWidget(idx));
            if (w == 0 || selected != w->getTrack()) {
                  if (w)
                  {
                        //fprintf(stderr, "MidiEditor::switchInfo deleting strip\n");
                        delete w;
                        //w->deleteLater();
                  }
                  w = new MidiStrip(trackInfoWidget, static_cast <MusECore::MidiTrack*>(selected));
                  // Leave broadcasting changes to other selected tracks off.
                  
                  // Set focus yielding to the canvas.
                  if(MusEGlobal::config.smartFocus)
                  {
                    w->setFocusYieldWidget(canvas);
                    //w->setFocusPolicy(Qt::WheelFocus);
                  }

                  // We must marshall song changed instead of connecting to the strip's song changed
                  //  otherwise it crashes when loading another song because track is no longer valid
                  //  and the strip's songChanged() seems to be called before Pianoroll songChanged()
                  //  gets called and has a chance to stop the crash.
                  //connect(MusEGlobal::song, SIGNAL(songChanged(MusECore::SongChangedStruct_t)), w, SLOT(songChanged(MusECore::SongChangedStruct_t)));
                  
                  connect(MusEGlobal::muse, SIGNAL(configChanged()), w, SLOT(configChanged()));
                  w->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
                  trackInfoWidget->addWidget(w, idx);
                  w->show();
                  //setTabOrder(midiTrackInfo, w);
                  }
            }
      if (trackInfoWidget->curIdx() == n)
            return;
      trackInfoWidget->raiseWidget(n);
      }

//---------------------------------------------------------
//   trackInfoSongChange
//---------------------------------------------------------

void MidiEditor::trackInfoSongChange(MusECore::SongChangedStruct_t flags)
{
  if(!selected)
    return;
  
  if(selected->isMidiTrack()) 
  {
    MidiStrip* w = static_cast<MidiStrip*>(trackInfoWidget->getWidget(1));
    if(w)
      w->songChanged(flags);
  }
}

//---------------------------------------------------------
//   updateTrackInfo
//---------------------------------------------------------

void MidiEditor::updateTrackInfo()
{
      MusECore::Part* part = curCanvasPart();
      if(part)
        selected = part->track();
      else
        selected = 0;
      
      if (selected == 0) {
            switchInfo(0);
            return;
            }
      if (selected->isMidiTrack()) 
            switchInfo(1);
}

//---------------------------------------------------------
//   checkTrackInfoTrack
//---------------------------------------------------------

void MidiEditor::checkTrackInfoTrack()
{
  const int idx = 1;
  {
    Strip* w = static_cast<Strip*>(trackInfoWidget->getWidget(idx));
    if(w)
    {
      MusECore::Track* t = w->getTrack();
      if(t)
      {
        MusECore::TrackList* tl = MusEGlobal::song->tracks();
        MusECore::iTrack it = tl->find(t);
        if(it == tl->end())
        {
          delete w;
          trackInfoWidget->addWidget(0, idx);
          selected = 0;
          switchInfo(0);
        } 
      }   
    } 
  }
}
        
//---------------------------------------------------------
//   movePlayPointerToSelectedEvent
//---------------------------------------------------------

void MidiEditor::movePlayPointerToSelectedEvent()
{
    const MusECore::EventList & evl = curCanvasPart()->events();
    int tickPos = -1;
    for (MusECore::ciEvent ev=evl.begin(); ev!=evl.end(); ev++) {
       if (ev->second.selected()) {
        tickPos = ev->second.tick();
        printf("found selected event, moving to pos %d\n", tickPos);
        break;
       }
    }
    if (tickPos > -1)
    {
        MusECore::Pos p0(curCanvasPart()->tick() + tickPos, true);
        MusEGlobal::song->setPos(0, p0);
    }
}

//---------------------------------------------------------
//   addPart
//---------------------------------------------------------

void MidiEditor::addPart(MusECore::Part* p)
{
  if(!_pl || !p)
    return;
  _pl->add(p);
  _parts.insert(p->sn());
}

// REMOVE Tim. citem. Added.
// void MidiEditor::getAllSelectedItems(CItemSet& list) const
// {
//   if(!canvas)
//     return;
//   
//   // These two steps use the tagging features to mark the objects (events)
//   //  as having been visited already, to avoid duplicates in the list.
//   canvas->getAllSelectedItems(list);
//   //
//   for(ciCtrlEdit i = ctrlEditList.begin(); i != ctrlEditList.end(); ++i)
//     (*i)->getAllSelectedItems(list);
//   
//   // Now that we have gathered all the selected items we need, and avoided
//   //  duplicates by using the tagging features, reset all the tagged flags now!
//   for(ciCItemSet i = list.begin(); i != list.end(); ++i)
//     (*i)->setObjectTagged(false);
// }

// REMOVE Tim. citem. Added.
//---------------------------------------------------------
//   itemsAreSelected
//---------------------------------------------------------

bool MidiEditor::itemsAreSelected() const
{
  bool res = false;
  if(canvas && canvas->itemsAreSelected())
    res = true;
  for(ciCtrlEdit i = ctrlEditList.begin(); i != ctrlEditList.end(); ++i)
    if((*i)->itemsAreSelected())
      res = true;
  return res;
}

// REMOVE Tim. citem. Added.
//---------------------------------------------------------
//   tagAllSelectedItems
//---------------------------------------------------------

void MidiEditor::tagAllSelectedItems(bool range, bool rangeSelectedOnly,
        const MusECore::Pos& p0, const MusECore::Pos& p1) const
{
  // These two steps use the tagging features to mark the objects (events)
  //  as having been visited already, to avoid duplicates in the list.
  if(canvas)
    canvas->tagAllSelectedItems(range, rangeSelectedOnly, p0, p1);
  for(ciCtrlEdit i = ctrlEditList.begin(); i != ctrlEditList.end(); ++i)
    (*i)->tagAllSelectedItems(range, rangeSelectedOnly, p0, p1);
}

//---------------------------------------------------------
//   MidiEditor
//---------------------------------------------------------

MidiEditor::~MidiEditor()
      {
      if (_pl)
            delete _pl;
      }


//---------------------------------------------------------
//   readStatus
//---------------------------------------------------------

void MidiEditor::readStatus(MusECore::Xml& xml)
      {
      if (_pl == 0)
            _pl = new MusECore::PartList;

      for (;;) {
            MusECore::Xml::Token token = xml.parse();
            QString tag = xml.s1();
            switch (token) {
                  case MusECore::Xml::Error:
                  case MusECore::Xml::End:
                        return;
                  case MusECore::Xml::TagStart:
                        if (tag == "raster")
                              _raster = xml.parseInt();
                        else if (tag == "topwin")
                              TopWin::readStatus(xml);
                        else
                              xml.unknown("MidiEditor");
                        break;
                  case MusECore::Xml::TagEnd:
                        if (tag == "midieditor")
                              return;
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   writePartList
//---------------------------------------------------------

void MidiEditor::writePartList(int level, MusECore::Xml& xml) const
      {
      if(!_pl)
        return;
      for (MusECore::ciPart p = _pl->begin(); p != _pl->end(); ++p) {
            MusECore::Part* part   = p->second;
            MusECore::Track* track = part->track();
            int trkIdx   = MusEGlobal::song->tracks()->index(track);
            int partIdx  = track->parts()->index(part);
            
            if((trkIdx == -1) || (partIdx == -1))
              printf("MidiEditor::writePartList error: trkIdx:%d partIdx:%d\n", trkIdx, partIdx);
              
            xml.put(level, "<part>%d:%d</part>", trkIdx, partIdx);
            }
      }

//---------------------------------------------------------
//   writeStatus
//---------------------------------------------------------

void MidiEditor::writeStatus(int level, MusECore::Xml& xml) const
      {
      xml.tag(level++, "midieditor");
      TopWin::writeStatus(level, xml);
      xml.intTag(level, "raster", _raster);
      xml.tag(level, "/midieditor");
      }

//---------------------------------------------------------
//   songChanged
//---------------------------------------------------------

void MidiEditor::songChanged(MusECore::SongChangedStruct_t type)
      {
      
      if (type._flags) {
            if (type._flags & (SC_PART_REMOVED | SC_PART_MODIFIED
               | SC_PART_INSERTED | SC_TRACK_REMOVED)) {
                  genPartlist();
                  // close window if editor has no parts anymore
                  if (parts()->empty()) {
                        close();
                        return;
                        }
                  }
            if (canvas)
                  canvas->songChanged(type);
            //else if (wview)
            //      wview->songChanged(type);

            if (type._flags & (SC_PART_REMOVED | SC_PART_MODIFIED
               | SC_PART_INSERTED | SC_TRACK_REMOVED)) {
                  
                  updateHScrollRange();
                  
                  if (canvas)
                        setWindowTitle(canvas->getCaption());
                  //else if (wview)
                  //      setWindowTitle(wview->getCaption());
                  if (type._flags & SC_SIG)
                        time->update();
                        
              }        
            }
      }

//---------------------------------------------------------
//   setCurDrumInstrument
//---------------------------------------------------------

void MidiEditor::setCurDrumInstrument(int instr)
      {
      _curDrumInstrument = instr;
      emit curDrumInstrumentChanged(_curDrumInstrument);
      }

//---------------------------------------------------------
//   curCanvasPart
//---------------------------------------------------------

MusECore::Part* MidiEditor::curCanvasPart() 
{ 
  if(canvas) 
    return canvas->part(); 
  else 
    return 0; 
}

//---------------------------------------------------------
//   setCurCanvasPart
//---------------------------------------------------------

void MidiEditor::setCurCanvasPart(MusECore::Part* part) 
{ 
  if(canvas) 
    canvas->setCurrentPart(part); 
}

void MidiEditor::addNewParts(const std::map< const MusECore::Part*, std::set<const MusECore::Part*> >& param)
{
  if(!_pl)
    return;
  
  using std::map;
  using std::set;
  
  for (map< const MusECore::Part*, set<const MusECore::Part*> >::const_iterator it = param.begin(); it!=param.end(); it++)
    if (_pl->index(it->first) != -1)
      for (set<const MusECore::Part*>::const_iterator it2=it->second.begin(); it2!=it->second.end(); it2++)
        addPart(const_cast<MusECore::Part*>(*it2)); // FIXME make this const-correct!
}

} // namespace MusEGui
