#ifndef LPM_EVENTPARSER_H
#define LPM_EVENTPARSER_H

//!
//! \file
//!
#include <string>
using namespace std;
#include "Singleton.h"

#include "Defs.h"
#include "Private.h"

namespace lpm { class ActualEvent; } 
namespace lpm { class ObservedEvent; } 

namespace lpm {

//!
//! \brief Parses Event objects
//!
//! Singleton class which provides convenient methods to parse events (e.g. strings read from file).
//!

class EventParser : public Singleton<EventParser> 
{
  public:
    bool ParseActualEvent(string line, ActualEvent** event) const;

    bool ParseObservedEvent(string line, ObservedEvent** event);

};

} // namespace lpm
#endif
