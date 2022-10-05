#ifndef LPM_EVENT_H
#define LPM_EVENT_H

//!
//! \file
//!
#include "Reference.h"

#include "Defs.h"

#define DEFAULT_MIN_USER_ID 1
#define DEFAULT_MIN_TIMESTAMP 1
#define DEFAULT_MIN_LOCATIONSTAMP 1

namespace lpm {

//!
//! \brief Represents an event (abstract base class for all events)
//!
//! \see ActualEvent, ExposedEvent, ObservedEvent
//!

class Event : public Reference<Event> 
{
  private:
    static volatile ull nextIndex;

    ull index;


  public:
    Event();

    virtual ~Event();

    virtual EventType GetType() const = 0;

};

} // namespace lpm
#endif
