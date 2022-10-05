#ifndef LPM_TRACE_H
#define LPM_TRACE_H

//!
//! \file
//!
#include <list>
using namespace std;
#include "Reference.h"
#include <vector>
using namespace std;

#include "Defs.h"
#include "NoDepend.h"

namespace lpm { class Event; } 

namespace lpm {

struct TraceVector 
{
    ull offset;

    ull length;

    ull* trace;

};
//!
//! \brief Represents a trace
//!
//! A trace is a set of events of the same type (one of EventType) for a given user.
//!

class Trace : public Reference<Trace> 
{
  private:
    list<Event*> events;


  public:
    explicit Trace(ull u);

    virtual ~Trace();

    bool AddEvent(const Event* event);

    bool RemoveEvent(const Event* event);


  private:
    ull user;


  public:
    ull GetUser() const;

    void GetEvents(vector<Event*>& ret) const;

};

} // namespace lpm
#endif
