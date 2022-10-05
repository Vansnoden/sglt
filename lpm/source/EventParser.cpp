//!
//! \file
//!
#include "../include/EventParser.h"
#include "../include/ActualEvent.h"
#include "../include/ObservedEvent.h"

namespace lpm {

bool EventParser::ParseActualEvent(string line, ActualEvent** event) const 
{
  // Bouml preserved body begin 00034191

	VERIFY(event != NULL);

	bool exposed = false;
	size_t pos = 0;
	vector<ull> values = vector<ull>();

	// try parse 4 (exposed event)
	bool parse4 = LineParser<ull>::GetInstance()->ParseFields(line, values, 4, &pos);

	if(parse4 == false)
	{
		values.clear();
		if(LineParser<ull>::GetInstance()->ParseFields(line, values, 3, &pos) == false || pos != string::npos)
		{
			SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
			return false;
		}
	}
	else if(pos != string::npos || (values[3] != 0 && values[3] != 1)) // parse4 but more than 4 values
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
		return false;
	}

	ull user = values[0];
	ull timestamp = values[1];
	ull locationstamp = values[2];

	if(parse4 == true){ exposed = (values[3] == 0) ? false : true; }

	*event = (exposed == true) ? new ExposedEvent(user, timestamp, locationstamp) : new ActualEvent(user, timestamp, locationstamp);

	return true;

  // Bouml preserved body end 00034191
}

bool EventParser::ParseObservedEvent(string line, ObservedEvent** event) 
{
  // Bouml preserved body begin 0003B211

	VERIFY(event != NULL && line.empty() == false);

	ull nym = 0; size_t pos = 0;
	if(LineParser<ull>::GetInstance()->ParseValue(line, &nym, &pos) == false || pos == string::npos)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
		return false;
	}

	size_t lastPos = line.rfind(DEFAULT_FIELDS_DELIMITER);
	size_t firstPos = line.rfind(DEFAULT_FIELDS_DELIMITER, lastPos - 1);

	pos = line.find(DEFAULT_FIELDS_DELIMITER);

	if(line.find(DEFAULT_FIELDS_DELIMITER) != firstPos || firstPos == string::npos || lastPos == string::npos || pos == string::npos)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
		return false;
	}

	string timestampsString = (line.substr(0, lastPos)).substr(pos + 1);
	string locationsString = line.substr(lastPos + 1);

	vector<ull> timestamps = vector<ull>();
	if(LineParser<ull>::GetInstance()->ParseFields(timestampsString, timestamps, ANY_NUMBER_OF_FIELDS, NULL, DEFAULT_FIELD_VALUES_DELIMITER) == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
		return false;
	}

	vector<ull> locations = vector<ull>();
	if(locationsString.empty() == false && LineParser<ull>::GetInstance()->ParseFields(locationsString, locations, ANY_NUMBER_OF_FIELDS, NULL, DEFAULT_FIELD_VALUES_DELIMITER) == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
		return false;
	}

	ObservedEvent* observedEvent = new ObservedEvent(nym);

	foreach_const(vector<ull>, timestamps, iter)
	{
		if(observedEvent->AddTimestamp(*iter) == false)
		{
			SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
			observedEvent->Release();

			return false;
		}
	}

	foreach_const(vector<ull>, locations, iter)
	{
		if(observedEvent->AddLocationstamp(*iter) == false)
		{
			SET_ERROR_CODE(ERROR_CODE_INVALID_FORMAT);
			observedEvent->Release();

			return false;
		}
	}

	*event = observedEvent;

	return true;

  // Bouml preserved body end 0003B211
}


} // namespace lpm
