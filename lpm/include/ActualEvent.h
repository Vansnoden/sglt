#ifndef LPM_ACTUALEVENT_H
#define LPM_ACTUALEVENT_H

//!
//! \file
//!
#include "Event.h"

#include "Defs.h"

namespace lpm {

//!
//! \brief Represents an actual event (i.e. a tuple containing a user ID, timestamp, locationstamp)
//!
//! \note Technically, ExposeEvent derives from ActualEvent.
//! For this reason, the IsExposed() method allows to determine whether an object of type ActualEvent is really exposed.
//!
//! \see Event, ExposedEvent, IsExposed()
//!

class ActualEvent : public Event 
{
  protected:
    ull user;

    ull timestamp;

    ull locationstamp;


  public:
    ActualEvent(ull u, ull t, ull l);

    virtual ~ActualEvent();

    ActualEvent(const ActualEvent& source);

    virtual EventType GetType() const;

    ull GetUser() const;

    ull GetTimestamp() const;

    ull GetLocationstamp() const;

    //! 
    //! \brief Returns whether the event is exposed (i.e. whether it is an instance of ActualEvent or ExposedEvent)
    //!
    //! \return true, if the event is exposed, false, otherwise
    //!
    bool IsExposed() const;

};

} // namespace lpm
#endif
