#ifndef LPM_OBSERVEDEVENT_H
#define LPM_OBSERVEDEVENT_H

//!
//! \file
//!
#include "Event.h"

#include "Defs.h"
#include "NoDepend.h"

namespace lpm {

//!
//! \brief Represents an observed event (i.e. a tuple containing a user ID, a set of timestamps, and, a set of locationstamps)
//!
//! \see Event, ActualEvent, ExposedEvent
//!

class ObservedEvent : public Event 
{
  private:
    ull eventIndex;

    ull pseudonym;

    set<ull> timestamp;

    set<ull> locationstamp;


  public:
    explicit ObservedEvent(ull nym);

    virtual ~ObservedEvent();

    virtual EventType GetType() const;

    virtual ull GetPseudonym() const;

    bool AddTimestamp(ull time);

    bool AddLocationstamp(ull loc);

    void GetTimestamps(set<ull>& ret) const;

    void GetLocationstamps(set<ull>& ret) const;

    ull GetEventIndex() const;

    void SetEventIndex(ull index);

};

} // namespace lpm
#endif
