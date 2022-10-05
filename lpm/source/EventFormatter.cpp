//!
//! \file
//!
#include "../include/EventFormatter.h"
#include "../include/ActualEvent.h"
#include "../include/ObservedEvent.h"

namespace lpm {

bool EventFormatter::FormatActualEvent(const ActualEvent* event, string& line, bool forceExposedOutputFormat) 
{
  // Bouml preserved body begin 0003B291

	if(event == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	stringstream ss("");

	ss << event->GetUser() << DEFAULT_FIELDS_DELIMITER << ' ';
	ss << event->GetTimestamp() << DEFAULT_FIELDS_DELIMITER << ' ';
	ss << event->GetLocationstamp();

	if(event->GetType() == Exposed) { ss << DEFAULT_FIELDS_DELIMITER  << ' ' << '1'; }
	else if(forceExposedOutputFormat == true) { ss << DEFAULT_FIELDS_DELIMITER  << ' ' << '0'; }

	line = ss.str();

	return true;

  // Bouml preserved body end 0003B291
}

bool EventFormatter::FormatObservedEvent(const ObservedEvent* event, string& line) 
{
  // Bouml preserved body begin 0003B311

	if(event == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	stringstream ss("");

	ss << event->GetPseudonym() << DEFAULT_FIELDS_DELIMITER;

	set<ull> timestamps = set<ull>();
	event->GetTimestamps(timestamps);

	foreach_const(set<ull>, timestamps, iter)
	{
		set<ull>::const_iterator next = iter;
		next++;

		ss << *iter;
		if(next != timestamps.end()) { ss << DEFAULT_FIELD_VALUES_DELIMITER; }
	}

	ss << DEFAULT_FIELDS_DELIMITER;

	set<ull> locationstamps = set<ull>();
	event->GetLocationstamps(locationstamps);

	foreach_const(set<ull>, locationstamps, iter)
	{
		set<ull>::const_iterator next = iter;
		next++;

		ss << *iter;
		if(next != locationstamps.end()) { ss << DEFAULT_FIELD_VALUES_DELIMITER; }
	}

	line = ss.str();

	return true;

  // Bouml preserved body end 0003B311
}


} // namespace lpm
