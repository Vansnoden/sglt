//!
//! \file
//!
#include "../include/Trace.h"
#include "../include/Event.h"

namespace lpm {

Trace::Trace(ull u) 
{
  // Bouml preserved body begin 00023D11

	user = u;

  // Bouml preserved body end 00023D11
}

Trace::~Trace() 
{
  // Bouml preserved body begin 00023D91

	foreach_const(list<Event*>, events, iter)
	{
		(*iter)->Release();
	}

	events.clear();

  // Bouml preserved body end 00023D91
}

bool Trace::AddEvent(const Event* event) 
{
  // Bouml preserved body begin 00023C91

	if(event == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	// TODO: add check for duplicate events (depending on how this is done, it may be inefficient)

	events.push_back(const_cast<Event*>(event));

	const_cast<Event*>(event)->AddRef();

	return true;

  // Bouml preserved body end 00023C91
}

bool Trace::RemoveEvent(const Event* event) 
{
  // Bouml preserved body begin 00043F91

	if(event == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	events.remove(const_cast<Event*>(event));
	const_cast<Event*>(event)->Release();

	return true;

  // Bouml preserved body end 00043F91
}

ull Trace::GetUser() const 
{
  // Bouml preserved body begin 00035D91

	return user;

  // Bouml preserved body end 00035D91
}

void Trace::GetEvents(vector<Event*>& ret) const 
{
  // Bouml preserved body begin 00035F11

	ret.resize(events.size());
	copy(events.begin(), events.end(), ret.begin());

  // Bouml preserved body end 00035F11
}


} // namespace lpm
