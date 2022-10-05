//!
//! \file
//!
//! \brief Provides some implementation of examples LPPM operations by implementing the LPPMOperation class
//!
#include "../include/ExampleLPPMOperations.h"
#include "../include/ExposedEvent.h"
#include "../include/ObservedEvent.h"
#include "../include/Context.h"
#include "../include/ActualEvent.h"
#include "../include/Event.h"

namespace lpm {

double DefaultServiceQualityMetricDistance::ComputeDistance(const ExposedEvent* exposedEvent, const ObservedEvent* observedEvent) const 
{
  // Bouml preserved body begin 000D9311

	VERIFY(exposedEvent->GetType() == Exposed);

	ull tm = exposedEvent->GetTimestamp();
	ull loc = exposedEvent->GetLocationstamp();

	set<ull> tmSet = set<ull>();
	observedEvent->GetTimestamps(tmSet);

	if(tmSet.size() != 1) { return 1.0; }
	ull observedtm = *(tmSet.begin());
	if(observedtm != tm) { return 1.0; }

	set<ull> locSet = set<ull>();
	observedEvent->GetLocationstamps(locSet);

	if(locSet.size() != 1) { return 1.0; }
	ull observedloc = *(locSet.begin());
	if(observedloc != loc) { return 1.0; }

	return 0.0;

  // Bouml preserved body end 000D9311
}

DefaultLPPMOperation::DefaultLPPMOperation(ushort obfuscationLevel, double fakeInjectionProbability,
		const FakeInjectionAlgorithm fakeInjectionAlgorithm, double hidingProbability, bool anonymize)
		: LPPMOperation("Default LPPMOperation")
{
  // Bouml preserved body begin 0003EF11

	this->obfuscationLevel = obfuscationLevel;
	this->fakeInjectionProbability = fakeInjectionProbability;
	this->fakeInjectionAlgorithm = fakeInjectionAlgorithm;
	this->hidingProbability = hidingProbability;

	this->flags = (anonymize == true) ? Anonymization : NoFlags;

  // Bouml preserved body end 0003EF11
}

DefaultLPPMOperation::~DefaultLPPMOperation() 
{
  // Bouml preserved body begin 0003EF91
  // Bouml preserved body end 0003EF91
}

void DefaultLPPMOperation::ObfuscateLocation(ull location, set<ull>& obfucatedSet) const 
{
  // Bouml preserved body begin 00050711

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);

	obfucatedSet.clear();
	ull numObf = (ull)pow(2, obfuscationLevel);

	ull firstObfLoc = location - ((location - minLoc) % numObf);
	for(ull i = 0; i < numObf; i++)
	{
		ull obf = firstObfLoc + i;

		if(obf >= minLoc && obf <= maxLoc)
		{
			obfucatedSet.insert(obf);
		}
	}

	VERIFY(obfuscationLevel != 0 || (obfuscationLevel == 0 && obfucatedSet.size() == 1 && firstObfLoc == location));

  // Bouml preserved body end 00050711
}

void DefaultLPPMOperation::ComputeGeneralStatistics(ull tp, double** avg) const 
{
  // Bouml preserved body begin 00050791

	VERIFY(avg != NULL);

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = (maxLoc - minLoc + 1);

	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	ull sizeInBytes = numLoc * sizeof(double);
	double* tmp = *avg = (double*)Allocate(sizeInBytes);
	memset(tmp, 0, sizeInBytes);

	// for each user
	pair_foreach_const(map<ull, UserProfile*>, profiles, iter)
	{
		UserProfile* profile = iter->second;
		VERIFY(profile != NULL);

		double* steadyStateVector = NULL;
		VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true);

		// get the proper sub-chain steady-state vector according to the time period of the event
		double* subChainSteadyStateVector = NULL;
		VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true);

		for(ull loc = minLoc; loc <= maxLoc; loc++)
		{
			ull locIndex = loc - minLoc;
			tmp[locIndex] += subChainSteadyStateVector[locIndex];
		}

		Free(subChainSteadyStateVector); // free the sub-chain steady-state vector
	}

	// normalization
	ull profileCount = profiles.size();
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		tmp[loc - minLoc] /= (double)profileCount;
	}

  // Bouml preserved body end 00050791
}

bool DefaultLPPMOperation::Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent) 
{
  // Bouml preserved body begin 0003ED11

	if(context == NULL || inEvent == NULL || outEvent == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	VERIFY(inEvent->GetType() == Actual || inEvent->GetType() == Exposed);

	ull pseudonym = inEvent->GetUser();
	if(CONTAINS_FLAG(flags, Anonymization) == true) { pseudonym = GetPseudonym(inEvent->GetUser()); }

	ObservedEvent* event = *outEvent = new ObservedEvent(pseudonym);
	ull timestamp = inEvent->GetTimestamp();
	event->AddTimestamp(timestamp);

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = (maxLoc - minLoc + 1);

	ull location = inEvent->GetLocationstamp();

	if(inEvent->GetType() == Actual)
	{
		// no fake injection
		if(fakeInjectionProbability == 0.0 || RNG::GetInstance()->GetUniformRandomDouble() <= (1.0 - fakeInjectionProbability))
		{
			//stringstream info("");
			//info << "Obs: " << nym << ", " << timestamp << ", {}, 0";
			//Log::GetInstance()->Append(info.str());

			VERIFY(ABS(PDF(context, inEvent, *outEvent) - (1.0 - fakeInjectionProbability)) <= EPSILON);

			return true;
		}

		ull fakeLocation = 0;

		switch(fakeInjectionAlgorithm)
		{
			case UniformSelection:
				fakeLocation = RNG::GetInstance()->GetUniformRandomULLBetween(minLoc, maxLoc);
				break;
			case GeneralStatisticsSelection:
				{
					double* avg = NULL;
					ull tp = Parameters::GetInstance()->LookupTimePeriod(timestamp);
					if(tp == INVALID_TIME_PERIOD)
					{
						SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
						return false;
					}

					ComputeGeneralStatistics(tp, &avg);

					fakeLocation = minLoc + RNG::GetInstance()->SampleIndexFromVector(avg, numLoc);

					Free(avg); // Free

					VERIFY(fakeLocation >= minLoc && fakeLocation <= maxLoc);
				}
				break;
			default:
				CODING_ERROR; break;
		}

		location = fakeLocation;
	}
	else if(hidingProbability != 0.0 && RNG::GetInstance()->GetUniformRandomDouble() <= hidingProbability) // event is exposed and we should hide it
	{
		VERIFY(inEvent->GetType() == Exposed && ABS(PDF(context, inEvent, *outEvent) - hidingProbability) < EPSILON);

		return true; // outEvent has no location set
	}

	// location obfuscation
	set<ull> obfucatedLocations = set<ull>();
	ObfuscateLocation(location, obfucatedLocations);

	//stringstream info("");
	//info << "Obs: " << nym << ", " << timestamp << ", {";

	foreach_const(set<ull>, obfucatedLocations, iter)
	{
		event->AddLocationstamp(*iter);
		//info << *iter << ", ";
	}
	obfucatedLocations.clear();

	//info << "}, " << ((inEvent->GetType() == Actual) ? 0 : 1);
	//Log::GetInstance()->Append(info.str());

	VERIFY(PDF(context, inEvent, *outEvent) != 0.0);

	return true;

  // Bouml preserved body end 0003ED11
}

double DefaultLPPMOperation::PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const 
{
  // Bouml preserved body begin 00042591

	VERIFY(context != NULL && inEvent != NULL && outEvent != NULL);

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	ull trueTimestamp = inEvent->GetTimestamp();

	set<ull> timestamps = set<ull>();
	outEvent->GetTimestamps(timestamps);

	if((CONTAINS_FLAG(flags, Anonymization) == false) && (inEvent->GetUser() != outEvent->GetPseudonym())) { return 0.0; }

	if(timestamps.size() != 1 || timestamps.find(trueTimestamp) == timestamps.end()) { return 0.0; }

	set<ull> locs = set<ull>();
	outEvent->GetLocationstamps(locs);

	if(locs.empty() == true)
	{
		if(inEvent->GetType() == Actual) { return 1.0 - fakeInjectionProbability; }
		else if(inEvent->GetType() == Exposed) { return hidingProbability; }
	}


	if(inEvent->GetType() == Exposed)
	{
		ull trueLocation = inEvent->GetLocationstamp();
		set<ull> locations = set<ull>();

		outEvent->GetLocationstamps(locations);

		set<ull> obfuscatedLocations = set<ull>();
		ObfuscateLocation(trueLocation, obfuscatedLocations);

		if(locations.size() != obfuscatedLocations.size()) { return 0.0; }

		foreach_const(set<ull>, locations, iter)
		{
			ull loc = *iter;

			if(obfuscatedLocations.find(loc) == obfuscatedLocations.end()) { return 0.0; }
		}

		return 1.0 - hidingProbability;
	}

	// event is Actual
	set<ull> locationstamps = set<ull>();
	outEvent->GetLocationstamps(locationstamps);

	// check that locationstamps is valid according to obfucationLevel
	ull numObf = (ull)pow(2, obfuscationLevel);
	ull min = 0; ull max = 0;
	foreach_const(set<ull>, locationstamps, iter)
	{
		if(min == 0) { min = max = *iter; }
		if(*iter < min) { min = *iter; }
		if(*iter > max) { max = *iter; }
	}

	// continuity check
	if(min + (locationstamps.size() - 1) != max) { return 0.0; }

	if(((min - minLoc)  % numObf) != 0 && min != minLoc) { return 0.0; }

	if((((max - minLoc) + 1) % numObf) != 0 && max != maxLoc) { return 0.0; }

	if(locationstamps.size() > numObf) { return 0.0; }

	switch(fakeInjectionAlgorithm)
	{
		case UniformSelection:
			{
				return fakeInjectionProbability * ((double)locationstamps.size() / (double)numLoc);
			}
			break;
		case GeneralStatisticsSelection:
			{
				double* avg = NULL;
				ull tp = Parameters::GetInstance()->LookupTimePeriod(trueTimestamp);
				if(tp == INVALID_TIME_PERIOD)
				{
					SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
					return false;
				}

				ComputeGeneralStatistics(tp, &avg);

				double sum = 0.0;
				foreach_const(set<ull>, locationstamps, iter)
				{
					ull loc = *iter;
					sum += avg[loc - minLoc];
				}
				Free(avg);

				return fakeInjectionProbability * sum;
			}
			break;
		default:
			CODING_ERROR; break;
	}

	return 0.0;

  // Bouml preserved body end 00042591
}

string DefaultLPPMOperation::GetDetailString() 
{
  // Bouml preserved body begin 00095E11

	string fakeAlgo = "";
	switch(fakeInjectionAlgorithm)
	{
		case UniformSelection:	fakeAlgo = STRINGIFY(UniformSelection); break;
		case GeneralStatisticsSelection: fakeAlgo = STRINGIFY(GeneralStatisticsSelection); break;
	}

	stringstream details("");
	details << "LPPMOperation: " << operationName << "(" << obfuscationLevel << ", " << (double)fakeInjectionProbability << ", " << fakeAlgo;
	details << ", " << (double)hidingProbability << ")";

	return details.str();

  // Bouml preserved body end 00095E11
}

PseudonymChangeLPPMOperation::PseudonymChangeLPPMOperation(double beta) : LPPMOperation("Pseudonym Change LPPMOperation")
{
  // Bouml preserved body begin 000C6D91

	this->changeProb = beta;

	this->flags = (LPPMFlags)((ull)Anonymization | (ull)PseudonymChange); // set flags

  // Bouml preserved body end 000C6D91
}

PseudonymChangeLPPMOperation::~PseudonymChangeLPPMOperation() 
{
  // Bouml preserved body begin 000C6E11
  // Bouml preserved body end 000C6E11
}

//!   
//! \brief Filters (i.e. distort) the input event 
//!  
//! \param[in] context 	Context*, the context.  
//! \param[in] inEvent 	ActualEvent*, the event to filter.  
//! \param[in] outEvent	ObservedEvent*, the filtered output event.  
//!  
//! \return true or false, depending on whether the call is successful  
//!
bool PseudonymChangeLPPMOperation::Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent) 
{
  // Bouml preserved body begin 000C6E91

	CODING_ERROR; // no need to implement this one, it should never be called

	return 0.0;

  // Bouml preserved body end 000C6E91
}

//! 
//! \brief The probability density function (pdf) of the filter operation
//!
//! Computes and returns the probability that \a outEvent is the filtered output event conditional on the input event \a inEvent and the \a context.
//!
//! \param[in] context 	Context*, the context.
//! \param[in] inEvent 	ActualEvent*, the event to filter.
//! \param[in] outEvent	ObservedEvent*, the filtered output event.
//!
//! \return the value of the pdf as a double
//!
double PseudonymChangeLPPMOperation::PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const 
{
  // Bouml preserved body begin 000C6F11

	CODING_ERROR;// no need to implement this one, it should never be called

	return 0.0;

  // Bouml preserved body end 000C6F11
}

string PseudonymChangeLPPMOperation::GetDetailString() 
{
  // Bouml preserved body begin 000C6F91

	stringstream details("");
	details << "LPPMOperation: " << operationName << "(" << changeProb << ")";

	return details.str();

  // Bouml preserved body end 000C6F91
}

bool PseudonymChangeLPPMOperation::Filter(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, Event** outEvent, const ObservedEvent* prevOutEvent) 
{
  // Bouml preserved body begin 000C7011

	const ull maxNym = 4 * Parameters::GetInstance()->GetUsersCount();
	const ull minNym = 1;

	bool changePseudonym = false;
	ull prevNym = minNym;

	if(prevInEvent == NULL)	{ changePseudonym = true; }
	else
	{
		changePseudonym = RNG::GetInstance()->GetUniformRandomDouble() < changeProb;
		prevNym = prevOutEvent->GetPseudonym();
	}

	ull nym = prevNym;
	if(changePseudonym == true)
	{
		while(nym == prevNym && nym >= minNym && nym <= maxNym)
		{
			bool odd = false;
			if(prevNym % 2 == 1) { odd = (RNG::GetInstance()->GetUniformRandomDouble() < 2.0/3.0); }
			else { odd = (RNG::GetInstance()->GetUniformRandomDouble() < 1.0/3.0); }

			nym = RNG::GetInstance()->GetUniformRandomULLBetween(minNym, maxNym/2);
			if(odd == true) { nym = 2 * nym + 1; }
			else  { nym = 2 * nym; }
		}
	}

	ObservedEvent* observedEvent = new ObservedEvent(nym);

	observedEvent->AddTimestamp(inEvent->GetTimestamp());
	observedEvent->AddLocationstamp(inEvent->GetLocationstamp());

	*outEvent = observedEvent;

	return true;

  // Bouml preserved body end 000C7011
}

double PseudonymChangeLPPMOperation::PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, const ObservedEvent* outEvent, const ObservedEvent* prevOutEvent) const 
{
  // Bouml preserved body begin 000C7091



	return 0.0;

  // Bouml preserved body end 000C7091
}


} // namespace lpm
