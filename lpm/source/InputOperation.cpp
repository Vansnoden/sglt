//!
//! \file
//!
#include "../include/InputOperation.h"
#include "../include/Event.h"

namespace lpm {

InputOperation::InputOperation(string name) : Operation<File, TraceSet>(name)
{
  // Bouml preserved body begin 00021C91

	inputInfo.minUserID = ULONG_LONG_MAX; inputInfo.maxUserID = 1;
	inputInfo.minTimestamp = ULONG_LONG_MAX; inputInfo.maxTimestamp = 1;
	inputInfo.minLocationstamp = ULONG_LONG_MAX; inputInfo.maxLocationstamp = 1;

  // Bouml preserved body end 00021C91
}

InputOperation::~InputOperation() 
{
  // Bouml preserved body begin 00021D11
  // Bouml preserved body end 00021D11
}

//! \brief Executes the input operation
//!
//! \param[in] input 	File*, input file.
//! \param[in,out] output 	TraceSet* output object.
//!
//! \note The implementation can read all three kinds of traces (i.e. actual, exposed, and, observed). 
//!
//! \return true or false, depending on whether the call is successful
bool InputOperation::Execute(const File* input, TraceSet* output) 
{
  // Bouml preserved body begin 00029311

	TraceSet* outputTraceSet = output;

	if(input == NULL || outputTraceSet == NULL || input->IsGood() == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	EventType eventType = Actual;
	TraceType traceType = outputTraceSet->GetTraceType();
	switch(traceType)
	{
		case ActualTrace: eventType = Actual; break;
		case ExposedTrace: eventType = Exposed; break;
		case ObservedTrace: eventType = Observed; break;
		default:
			CODING_ERROR; break;
	}

	while(input->IsEOF() == false)
	{
		Event* event = NULL;
		string line = "";

		if(input->ReadNextLine(line) == false && input->IsEOF() == false && line.empty() == false)
		{ return false; }

		// it may be that the file ends with a lot of empty lines.
		while(line.empty() == true)
		{
			// if line is not empty (and we have no reached EOF), we have a read error!
			if(input->ReadNextLine(line) == false && input->IsEOF() == false && line.empty() == false)
			{ return false; }

			if(line.empty() == false && input->IsEOF() == false)
			{
				SET_ERROR_CODE(ERROR_CODE_INCOMPLETE_INPUT_FILE);
				string errorMsg = "InputOperation::Execute() - Trace, input file incomplete (unexpected empty line(s))";
				Log::GetInstance()->Append(errorMsg, Log::errorLevel);
				return false;
			}

			if(input->IsEOF() == true) { VERIFY(line.empty() == true); break; }
		}

		if(line.empty() == true && input->IsEOF() == true) { continue; }

		if(ParseEvent(line, &event, outputTraceSet->GetTraceType()) == false)
		{
			return false;
		}

		VERIFY(event != NULL);
		if((traceType != ExposedTrace && event->GetType() != eventType) || (traceType == ExposedTrace && event->GetType() == Observed))
		{
			event->Release();

			SET_ERROR_CODE(ERROR_CODE_TRACE_INPUT_FILE_MISMATCH);
			string errorMsg = "InputOperation::Execute() - Trace, input file mismatch (expected: ";
			errorMsg += STRINGIFY(traceType) ")!";
			Log::GetInstance()->Append(errorMsg, Log::errorLevel);

			return false;
		}

		if(ShouldBeIncluded(event) == false) { event->Release(); continue; } // skip this event

		if(outputTraceSet->AddEvent(event) == false)
		{
			event->Release();
			return false;
		}

		const_cast<InputOperation*>(this)->UpdateInputInfo(event);

		event->Release();
	}

	if(outputTraceSet->IsEmpty() == true)
	{
		SET_ERROR_CODE(ERROR_CODE_EMPTY_TRACE);
		return false;
	}

	if(outputTraceSet->GetTraceType() == ActualTrace || outputTraceSet->GetTraceType() == ExposedTrace)
	{
		map<ull, Trace*> mapping = map<ull, Trace*>();
		outputTraceSet->GetMapping(mapping);

		ull paramsMinTimestamp = 0; ull paramsMaxTimestamp = 0;
		VERIFY(Parameters::GetInstance()->GetTimestampsRange(&paramsMinTimestamp, &paramsMaxTimestamp) == true);

		ull minTimestamp = inputInfo.minTimestamp;
		ull maxTimestamp = inputInfo.maxTimestamp;

		VERIFY(paramsMinTimestamp <= minTimestamp && maxTimestamp <= paramsMaxTimestamp);

		set<ull> intersect = set<ull>();
		for(ull i = minTimestamp; i <= maxTimestamp; i++){ intersect.insert(i); }

		pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
		{
			Trace* trace = mapIter->second;
			vector<Event*> events = vector<Event*>();
			trace->GetEvents(events);
			set<ull> userSet = set<ull>();

			foreach_const(vector<Event*>, events, iter)
			{
				ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(const_cast<Event*>(*iter));
				ull timestamp = actualEvent->GetTimestamp();

				if(intersect.find(timestamp) != intersect.end())
				{
					userSet.insert(timestamp);
				}
				else
				{
					trace->RemoveEvent(actualEvent);
				}
			}

			intersect.clear();
			intersect = userSet;
		}

		if(intersect.empty() == true)
		{
			SET_ERROR_CODE(ERROR_CODE_INCOMPLETE_INPUT_FILE);
			return false;
		}

		ull first = 0; ull last = 0;
		foreach_const(set<ull>, intersect, iter)
		{
			ull val = *iter;
			if(first == 0) { first = last = val; continue; }

			if(val == last + 1) { last++; }
			else
			{
				SET_ERROR_CODE(ERROR_CODE_INCOMPLETE_INPUT_FILE);
				return false;
			}
		}
		// add time restriction

		intersect.clear();

		pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
		{
			Trace* trace = mapIter->second;
			vector<Event*> events = vector<Event*>();
			trace->GetEvents(events);

			foreach_const(vector<Event*>, events, iter)
			{
				ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(const_cast<Event*>(*iter));
				ull timestamp = actualEvent->GetTimestamp();

				if(timestamp < first || timestamp > last)
				{
					trace->RemoveEvent(actualEvent);
				}
			}
		}

		if(first == last) // need at least 2 time instances
		{
			SET_ERROR_CODE(ERROR_CODE_INCOMPLETE_INPUT_FILE);
			return false;
		}

		VERIFY(Parameters::GetInstance()->SetTimestampsRange(first, last) == true);
	}

	return true;

  // Bouml preserved body end 00029311
}

string InputOperation::GetDetailString() 
{
  // Bouml preserved body begin 0002EC91

	return "InputOperation: " + operationName + "()";

  // Bouml preserved body end 0002EC91
}

bool InputOperation::ParseEvent(string line, Event** event, TraceType type) const 
{
  // Bouml preserved body begin 00034111

	VERIFY(event != NULL);

	bool ret = false;
	switch(type)
	{
		case ActualTrace:
		case ExposedTrace:
			ret = EventParser::GetInstance()->ParseActualEvent(line, (ActualEvent**)event);
			break;
		case ObservedTrace:
			ret = EventParser::GetInstance()->ParseObservedEvent(line, (ObservedEvent**)event);
			break;
		default:
			CODING_ERROR;
	}

	return ret;

  // Bouml preserved body end 00034111
}

void InputOperation::UpdateInputInfo(const Event* event) 
{
  // Bouml preserved body begin 00042311

	VERIFY(event != NULL);

	if(event->GetType() == Actual || event->GetType() == Exposed)
	{
		ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(const_cast<Event*>(event));

		ull user = actualEvent->GetUser();
		if(user < inputInfo.minUserID) { inputInfo.minUserID = user; }
		if(user > inputInfo.maxUserID) { inputInfo.maxUserID = user; }

		ull timestamp = actualEvent->GetTimestamp();
		if(timestamp < inputInfo.minTimestamp) { inputInfo.minTimestamp = timestamp; }
		if(timestamp > inputInfo.maxTimestamp) { inputInfo.maxTimestamp = timestamp; }

		ull locationstamp = actualEvent->GetLocationstamp();
		if(locationstamp < inputInfo.minLocationstamp) { inputInfo.minLocationstamp = locationstamp; }
		if(locationstamp > inputInfo.maxLocationstamp) { inputInfo.maxLocationstamp = locationstamp; }
	}

  // Bouml preserved body end 00042311
}

bool InputOperation::ShouldBeIncluded(const Event* event) const 
{
  // Bouml preserved body begin 00042391

	VERIFY(event != NULL);

	// ull minUserID = 0; ull maxUserID = 0;
	ull minTimestamp = 0; ull maxTimestamp = 0;

	// GUR: ### VERIFY(Parameters::GetInstance()->GetUsersRange(&minUserID, &maxUserID) == true);
	Parameters* params = Parameters::GetInstance();
	VERIFY(params->GetTimestampsRange(&minTimestamp, &maxTimestamp) == true);

	if(event->GetType() == Actual || event->GetType() == Exposed)
	{
		ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(const_cast<Event*>(event));

		ull user = actualEvent->GetUser();
		ull timestamp = actualEvent->GetTimestamp();

		// GUR: ###if(user < minUserID || user > maxUserID) { return false; }
		if(params->UserExists(user) == false) { return false; }
		if(timestamp < minTimestamp || timestamp > maxTimestamp) { return false; }
	}

	return true;

  // Bouml preserved body end 00042391
}


} // namespace lpm
