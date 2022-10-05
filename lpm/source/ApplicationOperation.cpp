//!
//! \file
//!
#include "../include/ApplicationOperation.h"
#include "../include/Context.h"
#include "../include/Event.h"
#include "../include/ActualEvent.h"

namespace lpm {

ApplicationOperation::ApplicationOperation(string name) : FilterOperation(name)
{
  // Bouml preserved body begin 00022091
  // Bouml preserved body end 00022091
}

ApplicationOperation::~ApplicationOperation() 
{
  // Bouml preserved body begin 00022111
  // Bouml preserved body end 00022111
}

bool ApplicationOperation::Filter(const Context* context, const Event* inEvent, Event** outEvent) 
{
  // Bouml preserved body begin 0003B591

	return const_cast<ApplicationOperation*>(this)->Filter(context, (const ActualEvent*)inEvent, (ActualEvent**)outEvent);

  // Bouml preserved body end 0003B591
}

double ApplicationOperation::PDF(const Context* context, const Event* inEvent, const Event* outEvent) const 
{
  // Bouml preserved body begin 00027891

	 return PDF(context, (const ActualEvent*)inEvent, (const ActualEvent*)outEvent);

  // Bouml preserved body end 00027891
}

string ApplicationOperation::GetDetailString() 
{
  // Bouml preserved body begin 0002EE11

	return "ApplicationOperation: " + operationName + "()";

  // Bouml preserved body end 0002EE11
}

double ApplicationOperation::PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const 
{
  // Bouml preserved body begin 000D7511

	CODING_ERROR; // if we get here someone is trying to call this PDF (for LPPMs with pseudonym change) on an application

	return 0.0;

  // Bouml preserved body end 000D7511
}


} // namespace lpm
