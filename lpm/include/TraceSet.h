#ifndef LPM_TRACESET_H
#define LPM_TRACESET_H

//!
//! \file
//!
#include <map>
using namespace std;
#include "Reference.h"
#include <vector>
using namespace std;

#include "Defs.h"
#include "Event.h"
#include "ActualEvent.h"
#include "ExposedEvent.h"
#include "ObservedEvent.h"

namespace lpm { class Trace; } 
namespace lpm { class Event; } 

namespace lpm {

//!
//! \brief Represents a set of traces
//!
//! \see Trace
//!

class TraceSet : public Reference<TraceSet> 
{
  private:
    TraceType type;


  public:
    explicit TraceSet(TraceType traceType);

    virtual ~TraceSet();


  private:
    map<ull, Trace*> mapping;


  public:
    //! 
    //! \brief Adds the specified event to the trace set
    //!
    //! \param[in] event 	Event*, the event to add.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool AddEvent(const Event* event);

    TraceType GetTraceType() const;

    //! 
    //! \brief Retrieves the mapping of usersID to their traces
    //!
    //! Returns the mapping (user -> trace) that is contained in the TraceSet.
    //!
    //! \param[in,out] ret map<ull, Trace*>, a map which will contain the mapping after the call returns.
    //!
    //! \return nothing
    //!
    void GetMapping(map<ull, Trace*>& ret) const;

    //! 
    //! \brief Returns whether the trace set is empty (i.e. contains no events)
    //!
    //! \return true, if the trace set is empty, false, otherwise
    //!
    bool IsEmpty() const;

    void GetAllEvents(vector<Event*>& events) const;

};

} // namespace lpm
#endif
