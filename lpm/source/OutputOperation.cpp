//!
//! \file
//!
#include "../include/OutputOperation.h"

namespace lpm {

OutputOperation::OutputOperation(string name) : Operation<TraceSet, File>(name)
{
  // Bouml preserved body begin 00021D91
  // Bouml preserved body end 00021D91
}

OutputOperation::~OutputOperation() 
{
  // Bouml preserved body begin 00021E11
  // Bouml preserved body end 00021E11
}

//! \brief Executes the output operation 
//! 
//! \param[in] input 	TraceSet*, input object 
//! \param[in,out] output 	File*, output file. 
//!
//! \note The implementation can format and write to file all three kinds of traces (i.e. actual, exposed, and, observed). 
//! 
//! \return true or false, depending on whether the call is successful
bool OutputOperation::Execute(const TraceSet* input, File* output) 
{
  // Bouml preserved body begin 00029391

	if(input == NULL || output == NULL || output->IsGood() == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	map<ull, Trace*> mapping = map<ull, Trace*>();
	input->GetMapping(mapping);

	pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
	{
		Trace* trace = mapIter->second;
		vector<Event*> events = vector<Event*>();
		trace->GetEvents(events);

		foreach_const(vector<Event*>, events, iter)
		{
			Event* event = *iter;
			string line = "";

			EventType type = event->GetType();
			switch(type)
			{
				case Actual:
				case Exposed:
				{
					bool forceExposedOutputFormat = (input->GetTraceType() == ExposedTrace); // if we have an exposed trace, force "exposed" output format

					if(EventFormatter::GetInstance()->FormatActualEvent((ActualEvent*)event, line, forceExposedOutputFormat) == false) { return false; }
					break;
				}
				case Observed:
				{
					if(EventFormatter::GetInstance()->FormatObservedEvent((ObservedEvent*)event, line) == false) { return false; }
					break;
				}
				default:
					CODING_ERROR; break;
			}

			output->WriteLine(line);
		}
	}

	return true;

  // Bouml preserved body end 00029391
}

string OutputOperation::GetDetailString() 
{
  // Bouml preserved body begin 0002ED91

	return "OutputOperation: " + operationName + "()";

  // Bouml preserved body end 0002ED91
}


} // namespace lpm
