//!
//! \file
//!
#include "../include/FilterOperation.h"
#include "../include/Context.h"
#include "../include/Event.h"

namespace lpm {

FilterOperation::FilterOperation(string name) : Operation<TraceSet, TraceSet>(name)
{
  // Bouml preserved body begin 00021B91

	context = NULL;

  // Bouml preserved body end 00021B91
}

FilterOperation::~FilterOperation() 
{
  // Bouml preserved body begin 00021C11

	if(context != NULL) { context->Release(); }

  // Bouml preserved body end 00021C11
}

//! 
//! \brief Executes the filter operation
//!
//! \param[in] input 	TraceSet* whose events are to be filtered.
//! \param[in,out] output 	TraceSet* the filtered output object.
//!
//! \return true or false, depending on whether the call is successful
//!
bool FilterOperation::Execute(const TraceSet* input, TraceSet* output) 
{
  // Bouml preserved body begin 00021F91

	TraceSet* inputTraceSet = const_cast<TraceSet*>(input);
	TraceSet* outputTraceSet = const_cast<TraceSet*>(output);

	map<ull, Trace*> mapping = map<ull, Trace*>();
	inputTraceSet->GetMapping(mapping);

	pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
	{
		Trace* trace = mapIter->second;
		vector<Event*> events = vector<Event*>();
		trace->GetEvents(events);

		foreach_const(vector<Event*>, events, iter)
		{
			Event* inEvent = *iter;
			Event* outEvent = NULL;

			if(Filter(context, inEvent, &outEvent) == true && outEvent != NULL)
			{
				if(outputTraceSet->AddEvent(outEvent) == false)
				{
					outEvent->Release();
					return false;
				}
			}
			VERIFY(outEvent != NULL);

			if(outEvent != NULL) { outEvent->Release(); }
		}
	}

	return true;

  // Bouml preserved body end 00021F91
}

//! 
//! \brief Sets the context (background knowledge of the adversary)
//!
//! \param[in] newContext 	Context*, the new context
//!
//! \return nothing
//!
void FilterOperation::SetContext(Context* newContext) 
{
  // Bouml preserved body begin 00047991

	context = newContext;
	context->AddRef();

  // Bouml preserved body end 00047991
}


} // namespace lpm
