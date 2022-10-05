//!
//! \file
//!
#include "../include/ActualEvent.h"

namespace lpm {

ActualEvent::ActualEvent(ull u, ull t, ull l) 
{
  // Bouml preserved body begin 00034411

	user = u;
	timestamp = t;
	locationstamp = l;

  // Bouml preserved body end 00034411
}

ActualEvent::~ActualEvent() 
{
  // Bouml preserved body begin 00037A91
  // Bouml preserved body end 00037A91
}

ActualEvent::ActualEvent(const ActualEvent& source) 
{
  // Bouml preserved body begin 0003B511

	user = source.user;
	timestamp = source.timestamp;
	locationstamp = source.locationstamp;

  // Bouml preserved body end 0003B511
}

EventType ActualEvent::GetType() const 
{
  // Bouml preserved body begin 00039511

	return Actual;

  // Bouml preserved body end 00039511
}

ull ActualEvent::GetUser() const 
{
  // Bouml preserved body begin 00034211

	return user;

  // Bouml preserved body end 00034211
}

ull ActualEvent::GetTimestamp() const 
{
  // Bouml preserved body begin 00034291

	return timestamp;

  // Bouml preserved body end 00034291
}

ull ActualEvent::GetLocationstamp() const 
{
  // Bouml preserved body begin 00034311

	return locationstamp;

  // Bouml preserved body end 00034311
}

//! 
//! \brief Returns whether the event is exposed (i.e. whether it is an instance of ActualEvent or ExposedEvent)
//!
//! \return true, if the event is exposed, false, otherwise
//!
bool ActualEvent::IsExposed() const 
{
  // Bouml preserved body begin 00034391

	return (GetType() == Exposed);

  // Bouml preserved body end 00034391
}


} // namespace lpm
