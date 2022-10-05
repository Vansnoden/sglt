//!
//! \file
//!
#include "../include/ExposedEvent.h"

namespace lpm {

ExposedEvent::ExposedEvent(ull u, ull t, ull l) : ActualEvent(u, t, l) 
{
  // Bouml preserved body begin 0003B391
  // Bouml preserved body end 0003B391
}

ExposedEvent::ExposedEvent(const ActualEvent& source) : ActualEvent(source)
{
  // Bouml preserved body begin 0003B191
  // Bouml preserved body end 0003B191
}

ExposedEvent::~ExposedEvent() 
{
  // Bouml preserved body begin 0003B411
  // Bouml preserved body end 0003B411
}

EventType ExposedEvent::GetType() const 
{
  // Bouml preserved body begin 0003B111

	return Exposed;

  // Bouml preserved body end 0003B111
}


} // namespace lpm
