//!
//! \file
//!
#include "../include/ObservedEvent.h"

namespace lpm {

ObservedEvent::ObservedEvent(ull nym) 
{
  // Bouml preserved body begin 0003D111

	pseudonym = nym;
	timestamp = set<ull>();
	locationstamp = set<ull>();

  // Bouml preserved body end 0003D111
}

ObservedEvent::~ObservedEvent() 
{
  // Bouml preserved body begin 0003D191

	timestamp.clear();
	locationstamp.clear();

  // Bouml preserved body end 0003D191
}

EventType ObservedEvent::GetType() const 
{
  // Bouml preserved body begin 00039591

	return Observed;

  // Bouml preserved body end 00039591
}

ull ObservedEvent::GetPseudonym() const 
{
  // Bouml preserved body begin 00036091

	return pseudonym;

  // Bouml preserved body end 00036091
}

bool ObservedEvent::AddTimestamp(ull time) 
{
  // Bouml preserved body begin 0003D211

	ull min = 0; ull max = 0;
	DEBUG_VERIFY(Parameters::GetInstance()->GetTimestampsRange(&min, &max) == true);

	if(time < min || time > max)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	if(timestamp.find(time) == timestamp.end())
	{
		timestamp.insert(time);
		return true;
	}

	return true;

  // Bouml preserved body end 0003D211
}

bool ObservedEvent::AddLocationstamp(ull loc) 
{
  // Bouml preserved body begin 0003D291

	ull min = 0; ull max = 0;
	DEBUG_VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&min, &max) == true);

	if(loc < min || loc > max)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	if(locationstamp.find(loc) == locationstamp.end())
	{
		locationstamp.insert(loc);
		return true;
	}

	return true;


  // Bouml preserved body end 0003D291
}

void ObservedEvent::GetTimestamps(set<ull>& ret) const 
{
  // Bouml preserved body begin 00040911

	ret = timestamp;

  // Bouml preserved body end 00040911
}

void ObservedEvent::GetLocationstamps(set<ull>& ret) const 
{
  // Bouml preserved body begin 00040991

	ret = locationstamp;

  // Bouml preserved body end 00040991
}

ull ObservedEvent::GetEventIndex() const 
{
  // Bouml preserved body begin 000CD811

	return eventIndex;

  // Bouml preserved body end 000CD811
}

void ObservedEvent::SetEventIndex(ull index) 
{
  // Bouml preserved body begin 000D4091

	eventIndex = index;

  // Bouml preserved body end 000D4091
}


} // namespace lpm
