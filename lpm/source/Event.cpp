//!
//! \file
//!
#include "../include/Event.h"

namespace lpm {

volatile ull Event::nextIndex = 0;

Event::Event() 
{
  // Bouml preserved body begin 0001FD91

	index = nextIndex++;

  // Bouml preserved body end 0001FD91
}

Event::~Event() 
{
  // Bouml preserved body begin 0001FE11
  // Bouml preserved body end 0001FE11
}


} // namespace lpm
