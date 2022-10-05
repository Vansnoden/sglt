//!
//! \file
//!
#include "../include/TraceSet.h"
#include "../include/Trace.h"

namespace lpm {

TraceSet::TraceSet(TraceType traceType) 
{
  // Bouml preserved body begin 0001FC11

	type = traceType;
	mapping = map<ull, Trace*>();

  // Bouml preserved body end 0001FC11
}

TraceSet::~TraceSet() 
{
  // Bouml preserved body begin 0001FC91

	pair_foreach_const(map<ull, Trace*>, mapping, iter)
	{
		iter->second->Release();
	}

	mapping.clear();

  // Bouml preserved body end 0001FC91
}

//! 
//! \brief Adds the specified event to the trace set
//!
//! \param[in] event 	Event*, the event to add.
//!
//! \return true or false, depending on whether the call is successful
//!
bool TraceSet::AddEvent(const Event* event) 
{
  // Bouml preserved body begin 00023E11

	if(event == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	ull userOrPseudonym = 0;

	if(event->GetType() == Actual || event->GetType() == Exposed)
	{
		userOrPseudonym = (dynamic_cast<ActualEvent*>(const_cast<Event*>(event)))->GetUser();
	}
	else if(event->GetType() == Observed)
	{
		userOrPseudonym = dynamic_cast<const ObservedEvent*>(event)->GetPseudonym();
	}
	else { CODING_ERROR; }

	map<ull, Trace*>::iterator iter = mapping.find(userOrPseudonym);

	if(iter == mapping.end()) // not found
	{
		mapping.insert(pair<ull, Trace*>(userOrPseudonym, new Trace(userOrPseudonym)));
		iter = mapping.find(userOrPseudonym);

		DEBUG_VERIFY(iter != mapping.end());
	}

	Trace* trace = iter->second;

	return trace->AddEvent(event);;

  // Bouml preserved body end 00023E11
}

TraceType TraceSet::GetTraceType() const 
{
  // Bouml preserved body begin 00032611

	return type;

  // Bouml preserved body end 00032611
}

//! 
//! \brief Retrieves the mapping of usersID to their traces
//!
//! Returns the mapping (user -> trace) that is contained in the TraceSet.
//!
//! \param[in,out] ret map<ull, Trace*>, a map which will contain the mapping after the call returns.
//!
//! \return nothing
//!
void TraceSet::GetMapping(map<ull, Trace*>& ret) const 
{
  // Bouml preserved body begin 00035E11

	ret = mapping;

  // Bouml preserved body end 00035E11
}

//! 
//! \brief Returns whether the trace set is empty (i.e. contains no events)
//!
//! \return true, if the trace set is empty, false, otherwise
//!
bool TraceSet::IsEmpty() const 
{
  // Bouml preserved body begin 00042611

	return mapping.empty();

  // Bouml preserved body end 00042611
}

void TraceSet::GetAllEvents(vector<Event*>& events) const 
{
  // Bouml preserved body begin 000CF291

	events.clear();

	pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
	{
		Trace* trace = mapIter->second;

		vector<Event*> traceEvents = vector<Event*>();
		trace->GetEvents(traceEvents);

		foreach_const(vector<Event*>, traceEvents, eventsIter)
		{
			events.push_back(*eventsIter);
		}
	}

  // Bouml preserved body end 000CF291
}


} // namespace lpm
