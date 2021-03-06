//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: tempo.h,v 1.2.2.1 2006/09/19 19:07:09 spamatica Exp $
//
//  (C) Copyright 1999/2000 Werner Schweer (ws@seh.de)
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

#ifndef __TEMPO_H__
#define __TEMPO_H__

#include <map>
#include <vector>

#include "large_int.h"

#ifndef MAX_TICK
#define MAX_TICK (0x7fffffff/100)
#endif

// Tempo ring buffer size
#define TEMPO_FIFO_SIZE    1024


namespace MusECore {

class Xml;

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      int tempo;
      unsigned tick;    // new tempo at tick
      unsigned frame;   // precomputed time for tick in sec

      int read(Xml&);
      void write(int, Xml&, int) const;

      TEvent() { }
      TEvent(unsigned t, unsigned tk) {
            tempo = t;
            tick  = tk;
            frame = 0;
            }
      };

//---------------------------------------------------------
//   TempoList
//---------------------------------------------------------

typedef std::map<unsigned, TEvent*, std::less<unsigned> > TEMPOLIST;
typedef TEMPOLIST::iterator iTEvent;
typedef TEMPOLIST::const_iterator ciTEvent;
typedef TEMPOLIST::reverse_iterator riTEvent;
typedef TEMPOLIST::const_reverse_iterator criTEvent;

class TempoList : public TEMPOLIST {
    
   friend struct PendingOperationItem;
    
      int _tempoSN;           // serial no to track tempo changes
      bool useList;
      int _tempo;             // tempo if not using tempo list
      int _globalTempo;       // %percent 50-200%

      void add(unsigned tick, int tempo, bool do_normalize = true);
      void add(unsigned tick, TEvent* e, bool do_normalize = true);
      void del(iTEvent, bool do_normalize = true);
      void del(unsigned tick, bool do_normalize = true);

   public:
      TempoList();
      ~TempoList();

      // Makes a copy of the source list including all allocated items.
      // This clears and deletes existing items in the destination list.
      void copy(const TempoList& src);

      void normalize();
      void clear();
      void eraseRange(unsigned stick, unsigned etick);

      void read(Xml&);
      void write(int, Xml&) const;
      void dump() const;

      // Returns the actual Beats Per Minute at given tick or the static tempo value if master (useList) is off.
      float bpm(unsigned tick) const;
      // Bypass the master (useList) flag and return the actual Beats Per Minute at the given tick.
      float bpmAt(unsigned tick) const;
      // Returns a tempo value from the list at the given tick if master (useList) is on, or else the static tempo value.
      // This is not in the same units as BPM.
      int tempo(unsigned tick) const;
      // Bypass the master (useList) flag and return a tempo value directly from the list at the given tick.
      // This is not in the same units as BPM.
      int tempoAt(unsigned tick) const;
      
      
      //-------------------------------------------------------------------------------------------------------
      // Normally do not round these tick to frame methods down since (audio) frame resolution is higher than tick
      //  resolution. Other rounding methods are provided for convenience, since tick to frame is a 'broad' question.
      //-------------------------------------------------------------------------------------------------------
      // Returns the number of frames contained in the given ticks,
      //  at the tempo at the tick given by tempoTick. Honours useList.
      unsigned ticks2frames(unsigned ticks, unsigned tempoTick, LargeIntRoundMode round_mode = LargeIntRoundUp) const;
// TODO
//       // Returns the number of ticks contained in the given frames,
//       //  at the tempo at the tick given by tempoTick. Honours useList.
//       unsigned frames2ticks(unsigned frames, unsigned tempoTick) const;
      unsigned tick2frame(unsigned tick, unsigned frame, int* sn, LargeIntRoundMode round_mode = LargeIntRoundUp) const;
      unsigned tick2frame(unsigned tick, int* sn = 0, LargeIntRoundMode round_mode = LargeIntRoundUp) const;
      unsigned deltaTick2frame(unsigned tick1, unsigned tick2, int* sn = 0, LargeIntRoundMode round_mode = LargeIntRoundUp) const;
      
      //-------------------------------------------------------------------------------------------------------
      // Normally do not round these frame to tick methods up since (audio) frame resolution is higher than tick
      //  resolution. Other rounding methods are provided for convenience, since frame to tick is a 'broad' question.
      // Only round up or to nearest if you know what you are doing. It is OK to do that for example
      //  to VISUALLY 'snap' the frame to a tick from either side of the tick (the MTScale time scale widgets).
      // But for most other uses especially timing, leave it alone at rounding down. Timing ticks need to change
      //  to the next value ONLY when there are enough frames to count as one whole tick. Otherwise tick-based
      //  events such as midi notes might be processed slightly too soon (or too late in some circumstances).
      //-------------------------------------------------------------------------------------------------------
      unsigned frame2tick(unsigned frame, int* sn = 0, LargeIntRoundMode round_mode = LargeIntRoundDown) const;
      unsigned frame2tick(unsigned frame, unsigned tick, int* sn, LargeIntRoundMode round_mode = LargeIntRoundDown) const;
      unsigned deltaFrame2tick(unsigned frame1, unsigned frame2, int* sn = 0, LargeIntRoundMode round_mode = LargeIntRoundDown) const;
      
      int tempoSN() const { return _tempoSN; }
      // Sets the tempo value in the list if master is on, or else the static tempo value.
      void setTempo(unsigned tick, int newTempo);
      // Returns the static tempo value (the one used when the master is off).
      int staticTempo() const           { return _tempo; }
      // Sets the static tempo value (the one used when the master is off).
      void setStaticTempo(int newTempo);
      void addTempo(unsigned t, int tempo, bool do_normalize = true);
      void delTempo(unsigned tick, bool do_normalize = true);
      bool masterFlag() const { return useList; }
      bool setMasterFlag(unsigned tick, bool val);
      int globalTempo() const           { return _globalTempo; }
      void setGlobalTempo(int val);
      };

//---------------------------------------------------------
//   Tempo Record Event
//---------------------------------------------------------

struct TempoRecEvent {
      int tempo;
      unsigned tick;    
      TempoRecEvent() { }
      TempoRecEvent(unsigned tk, unsigned t) {
            tick  = tk;
            tempo = t;
            }
      };

class TempoRecList : public std::vector<TempoRecEvent >
{
  public:
    void addTempo(int tick, int tempo)    { push_back(TempoRecEvent(tick, tempo)); }
    void addTempo(const TempoRecEvent& e) { push_back(e); }
};

//---------------------------------------------------------
//   TempoFifo
//---------------------------------------------------------

class TempoFifo {
      TempoRecEvent fifo[TEMPO_FIFO_SIZE];
      volatile int size;
      int wIndex;
      int rIndex;

   public:
      TempoFifo()  { clear(); }
      bool put(const TempoRecEvent& event);   // returns true on fifo overflow
      TempoRecEvent get();
      const TempoRecEvent& peek(int = 0);
      void remove();
      bool isEmpty() const { return size == 0; }
      void clear()         { size = 0, wIndex = 0, rIndex = 0; }
      int getSize() const  { return size; }
      };
      
} // namespace MusECore

namespace MusEGlobal {
extern MusECore::TempoList tempomap;
extern MusECore::TempoRecList tempo_rec_list;
}

#endif
