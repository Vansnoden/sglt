//!
//! \file
//!
#include "../include/LPPMOperation.h"
#include "../include/ExposedEvent.h"
#include "../include/ObservedEvent.h"
#include "../include/File.h"
#include "../include/Context.h"
#include "../include/Event.h"
#include "../include/ActualEvent.h"
#include "../include/TraceSet.h"

namespace lpm {

void LPPMOperation::ComputeAnonymizationMap(bool anonymize)
{
  // Bouml preserved body begin 00074211

	VERIFY(anonymization.empty() == true);

	// ull min = 0; ull max = 0;
	// GUR: ### VERIFY(Parameters::GetInstance()->GetUsersRange(&min, &max) == true);

	set<ull> usersSet = set<ull>();
	VERIFY(Parameters::GetInstance()->GetUsersSet(usersSet) == true);
	VERIFY(usersSet.size() > 0);

	ull numUsers = usersSet.size();
	ull byteSize = numUsers * sizeof(ull);

	ull* users = (ull*)Allocate(byteSize);
	ull* pseudonyms = (ull*)Allocate(byteSize);

	VERIFY(users != NULL);
	VERIFY(pseudonyms != NULL);

	memset(users, 0, byteSize);
	memset(pseudonyms, 0, byteSize);

	ull i = 0;
	foreach_const(set<ull>, usersSet, iter) { users[i] = *iter; i++; }

	if(anonymize == false) { for(ull i = 0; i < numUsers; i++) { pseudonyms[i] = users[i]; } }
	else { RNG::GetInstance()->RandomPermutation(users, numUsers, pseudonyms); }

	for(ull i = 0; i < numUsers; i++)
	{
		ull user = users[i];
		ull pseudonym = pseudonyms[i];

		VERIFY(user != 0 && pseudonym != 0);

		anonymization.insert(pair<ull, ull>(user, pseudonym));
	}

	Free(users);
	Free(pseudonyms);

  // Bouml preserved body end 00074211
}

LPPMOperation::LPPMOperation(string name, LPPMFlags f) : FilterOperation(name), flags(f)
{
  // Bouml preserved body begin 00022211

	anonymization = map<ull, ull>();

	sigma = NULL;

	serviceQualityAnalysisOutput = NULL;
	serviceQualityAnalysisMetric = NULL;

  // Bouml preserved body end 00022211
}

LPPMOperation::~LPPMOperation() 
{
  // Bouml preserved body begin 00022291

	anonymization.clear();

	// if(sigma != NULL) { Free(sigma); } // don't free here, it may be used by the metric
	sigma = NULL;

  // Bouml preserved body end 00022291
}

bool LPPMOperation::Filter(const Context* context, const Event* inEvent, Event** outEvent) 
{
  // Bouml preserved body begin 0003B611

	if(anonymization.empty())
	{
		ComputeAnonymizationMap(CONTAINS_FLAG(flags, Anonymization) == true);

#define LOG_ANONYMIZATION 1
#ifdef LOG_ANONYMIZATION

		Log::GetInstance()->Append("LPPM Anonymization Mapping (user -> pseudonym):");
		pair_foreach_const(map<ull, ull>, anonymization, iter)
		{
			stringstream ss("");
			ss << iter->first << " -> " << iter->second;
			Log::GetInstance()->Append(ss.str());
		}

#endif

	}

	return const_cast<LPPMOperation*>(this)->Filter(context, (const ActualEvent*)inEvent, (ObservedEvent**)outEvent);

  // Bouml preserved body end 0003B611
}

double LPPMOperation::PDF(const Context* context, const Event* inEvent, const Event* outEvent) const 
{
  // Bouml preserved body begin 00027911

	// VERIFY(anonymization.empty() == false);

	return PDF(context, (const ActualEvent*)inEvent, (const ObservedEvent*)outEvent);

  // Bouml preserved body end 00027911
}

string LPPMOperation::GetDetailString() 
{
  // Bouml preserved body begin 0002EE91

	return "LPPMOperation: " + operationName;

  // Bouml preserved body end 0002EE91
}

LPPMFlags LPPMOperation::GetFlags() const 
{
  // Bouml preserved body begin 000CF191

	return flags;

  // Bouml preserved body end 000CF191
}

ull LPPMOperation::GetPseudonym(ull user) const 
{
  // Bouml preserved body begin 0003D011

	VERIFY(anonymization.empty() == false);

	map<ull, ull>::const_iterator iter = anonymization.find(user);
	VERIFY(iter != anonymization.end());

	return iter->second;

  // Bouml preserved body end 0003D011
}

void LPPMOperation::GetAnonymizationMap(map<ull, ull>& map) const 
{
  // Bouml preserved body begin 00075B91

	map = anonymization;

  // Bouml preserved body end 00075B91
}

//! 
//! \brief Executes the filter operation
//!
//! \param[in] input 	TraceSet* whose events are to be filtered.
//! \param[in,out] output 	TraceSet* the filtered output object.
//!
//! \return true or false, depending on whether the call is successful
//!
bool LPPMOperation::Execute(const TraceSet* input, TraceSet* output) 
{
  // Bouml preserved body begin 000CA391

	bool persistentPseudonyms = CONTAINS_FLAG(flags, PseudonymChange) == false;

	TraceSet* inputTraceSet = const_cast<TraceSet*>(input);
	TraceSet* outputTraceSet = const_cast<TraceSet*>(output);

	map<ull, Trace*> mapping = map<ull, Trace*>();
	inputTraceSet->GetMapping(mapping);

	bool doServiceQualityAnalysis = false;
	if(serviceQualityAnalysisMetric != NULL && serviceQualityAnalysisOutput != NULL && serviceQualityAnalysisOutput->IsGood() == true)
	{
		doServiceQualityAnalysis = true;
	}

	ull Nusers = mapping.size();

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	context->GetProfiles(profiles);
	VERIFY(Nusers == profiles.size());

	// Allocate sigma
	if(sigma != NULL) { Free(sigma); }

	ull sigmaByteSize = Nusers * numTimes * sizeof(ull);
	sigma = (ull*)Allocate(sigmaByteSize);
	VERIFY(sigma != NULL); memset(sigma, 0, sigmaByteSize);

	ull userIdx = 0;

	pair_foreach_const(map<ull, Trace*>, mapping, mapIter)
	{
		Trace* trace = mapIter->second;
		vector<Event*> events = vector<Event*>();
		trace->GetEvents(events);

		ull eventsSize = events.size();
		ull tmIdx = 0;

		// Warning: if you change the indexing scheme, parts of the code may be break (e.g., attack, metrics), so make sure you update those as well
		ull currentObservedIndex = userIdx; // set currentObservedIndex

		stringstream ss("");

		Event* prevInEvent = NULL; Event* prevOutEvent = NULL;
		foreach_const(vector<Event*>, events, iter)
		{
			Event* inEvent = *iter;
			Event* outEvent = NULL;

			ActualEvent* actualInEvent = dynamic_cast<ActualEvent*>(inEvent);
			ull timestampIdx = actualInEvent->GetTimestamp() - minTime;
			VERIFY(tmIdx == timestampIdx); // check

			// set sigma
			ull sigmaIdx = GET_INDEX(userIdx, tmIdx, numTimes);
			sigma[sigmaIdx] = currentObservedIndex;

			if(persistentPseudonyms == true)
			{
				if(Filter(context, inEvent, &outEvent) == true && outEvent != NULL)
				{
					if(outputTraceSet->AddEvent(outEvent) == false)
					{
						outEvent->Release();
						return false;
					}
				}
			}
			else
			{
				if(Filter(context, actualInEvent, (ActualEvent*)prevInEvent, &outEvent, (ObservedEvent*)prevOutEvent) == true && outEvent != NULL)
				{
					if(outputTraceSet->AddEvent(outEvent) == false)
					{
						outEvent->Release();
						return false;
					}
				}
			}

			VERIFY(outEvent != NULL);

			ObservedEvent* observedEvent = dynamic_cast<ObservedEvent*>(outEvent);
			VERIFY(observedEvent != NULL);
			observedEvent->SetEventIndex(currentObservedIndex); // force the index to be set here
			// note: the event index could be anything, our scheme is just some arbitrary indexing

			if(doServiceQualityAnalysis == true) // analyse
			{
				if(tmIdx == 0) { ull user = ((ActualEvent*)inEvent)->GetUser(); ss << user << ": "; }

				double distance = -1.0;
				if(inEvent->GetType() == Exposed)
				{ distance = serviceQualityAnalysisMetric->ComputeDistance((const ExposedEvent*)inEvent, (const ObservedEvent*)outEvent); };
				ss << distance;

				if(tmIdx != eventsSize - 1) { ss << ", "; }
				else { serviceQualityAnalysisOutput->WriteLine(ss.str()); ss.str(""); }
			}

			outEvent->Release();

			prevInEvent = inEvent;
			prevOutEvent = outEvent;

			VERIFY(prevInEvent != NULL && prevOutEvent != NULL);

			tmIdx++;
		}

		VERIFY(eventsSize == tmIdx);

		userIdx++;
	}

	return true;

  // Bouml preserved body end 000CA391
}

bool LPPMOperation::Filter(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, Event** outEvent, const ObservedEvent* prevOutEvent) 
{
  // Bouml preserved body begin 000C6C91

	VERIFY((prevInEvent == NULL && prevOutEvent == NULL) || (prevInEvent != NULL && prevOutEvent != NULL)); // check we are consistent with out previous events

	CODING_ERROR; // not implemented here: LPPMs without persistent pseudonyms (i.e. ones with the PseudonymChange flag on) must implement this method!
	return false;

  // Bouml preserved body end 000C6C91
}

ull* LPPMOperation::GetSigma() 
{
  // Bouml preserved body begin 000CBE11

	return sigma;

  // Bouml preserved body end 000CBE11
}

double LPPMOperation::PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const 
{
  // Bouml preserved body begin 000D7491

	return PDF(context, (const ActualEvent*)inEvent, (const ActualEvent*)prevInEvent, (const ObservedEvent*)outEvent, (const ObservedEvent*)prevOutEvent);

  // Bouml preserved body end 000D7491
}

double LPPMOperation::PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, const ObservedEvent* outEvent, const ObservedEvent* prevOutEvent) const 
{
  // Bouml preserved body begin 000C6C11

	VERIFY((prevInEvent == NULL && prevOutEvent == NULL) || (prevInEvent != NULL && prevOutEvent != NULL)); // check we are consistent with out previous events

	if(CONTAINS_FLAG(flags, PseudonymChange) == false) // if LPPM does not support pseudonym change, implement this PDF using the previous one
	{
		double val = PDF(context, inEvent, outEvent); // call the original PDF
		if(prevOutEvent != NULL)
		{
			ull prevNym = prevOutEvent->GetPseudonym();
			ull nym = outEvent->GetPseudonym();

			if(nym != prevNym) { return 0.0; } // if LPPM doesn't change pseudonyms the probability of two observed events with different pseudonyms is 0
		}

		return val;
	}

	CODING_ERROR; // not implemented here: LPPMs without persistent pseudonyms (i.e. ones with the PseudonymChange flag on) must implement this method!
	return 0.0;

  // Bouml preserved body end 000C6C11
}

void LPPMOperation::SetServiceQualityAnalysis(const File* output, const ServiceQualityMetricDistance* distance) 
{
  // Bouml preserved body begin 000D9391

	serviceQualityAnalysisOutput = const_cast<File*>(output);
	serviceQualityAnalysisMetric = const_cast<ServiceQualityMetricDistance*>(distance);

  // Bouml preserved body end 000D9391
}


} // namespace lpm
