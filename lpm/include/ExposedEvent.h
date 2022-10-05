#ifndef LPM_EXPOSEDEVENT_H
#define LPM_EXPOSEDEVENT_H

//!
//! \file
//!
#include "ActualEvent.h"

#include "Defs.h"

namespace lpm {

//!
//! \brief Represents an exposed event
//!
//! \note Technically, ExposeEvent derives from ActualEvent.
//! For this reason, the \a IsExposed() method allows to determine whether an object of type ActualEvent is really exposed.
//!
//! \see Event, ActualEvent
//!

class ExposedEvent : public ActualEvent 
{
  public:
    ExposedEvent(ull u, ull t, ull l);

    explicit ExposedEvent(const ActualEvent& source);

    ~ExposedEvent();

    virtual EventType GetType() const;

};

} // namespace lpm
#endif
