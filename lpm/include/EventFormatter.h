#ifndef LPM_EVENTFORMATTER_H
#define LPM_EVENTFORMATTER_H

//!
//! \file
//!
#include "Singleton.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class ActualEvent; } 
namespace lpm { class ObservedEvent; } 

namespace lpm {

//!
//! \brief Formats events into lines with a specific format
//!
//! Singleton class which provides convenient methods to format events.
//!

class EventFormatter : public Singleton<EventFormatter> 
{
  public:
    bool FormatActualEvent(const ActualEvent* event, string& line, bool forceExposedOutputFormat = false);

    bool FormatObservedEvent(const ObservedEvent* event, string& line);

};

} // namespace lpm
#endif
