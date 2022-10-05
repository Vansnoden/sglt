#include "SGAttackOperation.h"
#include "SGMetric.h"

SGAttackOperation::SGAttackOperation(double maxMult): AttackOperation("SGAttackOperation"), maxMultFactor(maxMult)
{
	VERIFY(maxMultFactor >= 1.0);
}

SGAttackOperation::~SGAttackOperation()
{

}

bool SGAttackOperation::CreateMetric(MetricType type, MetricOperation** metric) const
{
	switch(type)
	{
		case Distortion:
			*metric = new DistortionMetricOperation();
			return true;
		case MostLikelyLocationDistortion:
			*metric = new MostLikelyLocationDistortionMetricOperation();
			return true;
		case MostLikelyTraceDistortion:
			*metric = new MostLikelyTraceDistortionMetricOperation();
			return true;
		case Entropy:
			*metric = new EntropyMetricOperation();
			return true;
		case Density:
			*metric = new DensityMetricOperation();
			return true;
		case MeetingDisclosure:
			*metric = new MeetingDisclosureMetricOperation();
			return true;
		case Anonymity:
			*metric = new AnonymityMetricOperation();
			return true;
		case SGMetric:
			*metric = new SGMetricOperation();
			return true;
		default:
			CODING_ERROR;
			break;
	}

	return false;
}

bool SGAttackOperation::Execute(const TraceSet* input, AttackOutput* output)
{
	if(input == NULL || output == NULL || context == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	stringstream info("");
	info << "Starting the SG attack!";
	Log::GetInstance()->Append(info.str());

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	// ull numLoc = maxLoc - minLoc + 1;

	// get user profiles
	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	ull Nusers = profiles.size();

	map<ull, ull> userToPseudonymMapping = map<ull, ull>();
	ull* mostLikelyTrace = NULL;

	pair_foreach_const(map<ull, UserProfile*>, profiles, usersIter)
	{
		ull user = usersIter->first;
		userToPseudonymMapping.insert(make_pair(user, user)); // we assume that there is no anonymization!!!
	}

	ull mostLikelyTraceByteSize = Nusers * numTimes * sizeof(ull);
	mostLikelyTrace = (ull*)Allocate(mostLikelyTraceByteSize);
	VERIFY(mostLikelyTrace != NULL);
	memset(mostLikelyTrace, 0, mostLikelyTraceByteSize);

	ull logLikelihoodsByteSize = Nusers * sizeof(double);
	double* logLikelihoods =(double*)Allocate(logLikelihoodsByteSize);
	VERIFY(logLikelihoods != NULL);
	memset(logLikelihoods, 0, logLikelihoodsByteSize);

	// tracking
	VERIFY(ModifiedViterbi(input, userToPseudonymMapping, mostLikelyTrace, logLikelihoods) == true);
	output->SetMostLikelyTrace(mostLikelyTrace);

	output->SetMostLikelyTraceLL(logLikelihoods);

	return true;

}


bool SGAttackOperation::ModifiedViterbi(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, ull* mostLikelyTrace, double* logLikelihoods)
{
  // Bouml preserved body begin 0007C991

	if(traces == NULL || userToPseudonymMap.empty() == true || mostLikelyTrace == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get the user profiles
	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	ull Nusers = profiles.size();

	ull deltaByteSize = Nusers * numTimes * numLoc * sizeof(double);
	double* delta = (double*)Allocate(deltaByteSize);

	VERIFY(delta != NULL);
	memset(delta, 0, deltaByteSize);


	ull predecessorByteSize = Nusers * numTimes * numLoc * sizeof(ull);
	ull* predecessor = (ull*)Allocate(predecessorByteSize);

	VERIFY(predecessor != NULL);
	memset(predecessor, 0, predecessorByteSize);


	// get the mapping (pseudonym -> observed trace)
	map<ull, Trace*> mappingNymObserved = map<ull, Trace*>();
	traces->GetMapping(mappingNymObserved);

	VERIFY(Nusers == mappingNymObserved.size());

	// for all users
	ull userIndex = 0;
	pair_foreach_const(map<ull, UserProfile*>, profiles, usersIter)
	{
		ull user = usersIter->first;
		UserProfile* profile = usersIter->second;

		double* transitionMatrix = NULL;
		profile->GetTransitionMatrix(&transitionMatrix);

		double* steadyStateVector = NULL;
		profile->GetSteadyStateVector(&steadyStateVector);

		VERIFY(transitionMatrix != NULL && steadyStateVector != NULL);

		map<ull, ull>::const_iterator iter = userToPseudonymMap.find(user);
		VERIFY(iter != userToPseudonymMap.end());

		ull pseudonym = iter->second;

		map<ull, Trace*>::const_iterator mappingIter = mappingNymObserved.find(pseudonym);
		VERIFY(mappingIter != mappingNymObserved.end());

		Trace* observedTrace = mappingIter->second;

		vector<Event*> events = vector<Event*>();
		observedTrace->GetEvents(events);

		VERIFY(numTimes == events.size());

		ull mostLikelyLastLoc = minLoc;
		double mostLikelyLastLocValue = log(SQRT_DBL_MIN); // initially a very large (negative) value

		// for all time instants
		ull tm = minTime;
		foreach_const(vector<Event*>, events, eventsIter)
		{
			ObservedEvent* observedEvent = dynamic_cast<ObservedEvent*>(*eventsIter);
			set<ull> timestamps = set<ull>();
			observedEvent->GetTimestamps(timestamps);

			VERIFY(timestamps.size() == 1);
			ull timestamp = *(timestamps.begin());

			VERIFY(timestamp == tm && (timestamp >= minTime && timestamp <= maxTime));

			ull tp = Parameters::GetInstance()->LookupTimePeriod(timestamp);
			ull prevtp = tp; // ensure prevtp is always consistent with its usage
			if(timestamp > minTime) { prevtp = Parameters::GetInstance()->LookupTimePeriod(timestamp - 1); }
			if(prevtp == INVALID_TIME_PERIOD || tp == INVALID_TIME_PERIOD)
			{
				SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
				return false;
			}

			// get the proper sub-chain steady-state vector according to the time period of the event
			double* subChainSteadyStateVector = NULL;
			if(timestamp == minTime) // only needed for timestamp == minTime
			{ VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true); }

			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull deltaIndex = GET_INDEX_3D(userIndex, (timestamp - minTime), (loc - minLoc), numTimes, numLoc);

				ActualEvent* actualEvent = new ActualEvent(user, timestamp, loc);
				ExposedEvent* exposedEvent = new ExposedEvent(*actualEvent);

				VERIFY(actualEvent != NULL && exposedEvent != NULL);

				double lppmProb0 = lppmPDF->PDF(context, actualEvent, observedEvent);
				double applicationProb0 = applicationPDF->PDF(context, actualEvent, actualEvent);

				double lppmProb1 = lppmPDF->PDF(context, exposedEvent, observedEvent);
				double applicationProb1 = applicationPDF->PDF(context, actualEvent, exposedEvent);

				actualEvent->Release();
				exposedEvent->Release();

				double f = ((lppmProb0 * applicationProb0) + (lppmProb1 * applicationProb1));
				double logf = log(f);

				if(f <= 0.0 || logf == nan("n-char-sequence"))
				{
					logf = log(SQRT_DBL_MIN); // avoid log overflow/underflow/nan
				}

				if(timestamp == minTime) // initialization
				{
					double presenceProb = subChainSteadyStateVector[loc - minLoc];
					double logpp = log(presenceProb);

					if(presenceProb <= 0.0 || logpp == nan("n-char-sequence"))
					{
						logpp = log(SQRT_DBL_MIN); // avoid log overflow/underflow/nan
					}

					// delta[deltaIndex] = f * presenceProb;
					delta[deltaIndex] = logf + logpp; // use logarithms to avoid underflow

					{ // multiplicative factor to slightly change the probabilities
						double mult = ((maxMultFactor - 1.0) * RNG::GetInstance()->GetUniformRandomDouble()) + 1.0;
						delta[deltaIndex] += log(mult);
					}
				}
				else
				{
					ull maximizingLoc = minLoc;
					double maximizingValue = log(SQRT_DBL_MIN); // initially a very large (negative) value
					for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
					{
						ull prevDeltaIndex = GET_INDEX_3D(userIndex, ((timestamp - 1) - minTime), (loc2 - minLoc), numTimes, numLoc);

						double prevDelta = delta[prevDeltaIndex];

						// get the proper sub-chain transition vector to the time period of the previous event
						double* subChainTransitionVector = NULL;
						VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, prevtp, loc2, tp, &subChainTransitionVector) == true);

						ull currLocIdx = (loc - minLoc);
						double transProb = subChainTransitionVector[currLocIdx];

						Free(subChainTransitionVector);  // free the sub-chain transition vector

						// double m = (prevDelta * transProb);
						double m = prevDelta + log(transProb);  // use logarithms to avoid underflow

						{ // multiplicative factor to slightly change the probabilities
							double mult = ((maxMultFactor - 1.0) * RNG::GetInstance()->GetUniformRandomDouble()) + 1.0;
							m += log(mult);
						}

						if(maximizingValue < m)
						{
							maximizingLoc = loc2;
							maximizingValue = m;
						}

						/*// debug
						{
							stringstream info("");
							info << "User: " << user << " - possible loc at time " <<  timestamp << " for loc: " << loc;
							info << " is " << loc2 << " | m: " << m << " | prevDelta: " << prevDelta << " | transProb: " << transProb;
							Log::GetInstance()->Append(info.str());
						}*/
					}

					// delta[deltaIndex] = f * maximizingValue;
					delta[deltaIndex] = maximizingValue + logf; // use logarithms to avoid underflow

					// store predecessor
					ull predecessorIndex = GET_INDEX_3D(userIndex, (timestamp - minTime), (loc - minLoc), numTimes, numLoc);
					predecessor[predecessorIndex] = maximizingLoc;

					/* // debug
					{
						stringstream info("");
						info << "User: " << user << " - maximizing predecessor at time " <<  timestamp << " for loc: " << loc;
						info << " is " << maximizingLoc << " | delta: " << delta[deltaIndex] << " | f: " << f << " | maximizingValue: " << maximizingValue;
						Log::GetInstance()->Append(info.str());
					}*/
				}

				if(timestamp == maxTime) // find the max
				{
					if(mostLikelyLastLocValue < delta[deltaIndex])
					{
						mostLikelyLastLocValue = delta[deltaIndex];
						mostLikelyLastLoc = loc;
					}
				}
			}
			if(timestamp == minTime) // only needed for tm == minTime
			{ Free(subChainSteadyStateVector); /* free the sub-chain steady-state vector */ }

			tm++;
		}

		// reconstruct most likely trace for this user
		VERIFY(minTime > 0);
		ull predecessorLoc = mostLikelyLastLoc;

		ull index = GET_INDEX(userIndex, (maxTime - minTime), numTimes);
		mostLikelyTrace[index] = mostLikelyLastLoc;

		VERIFY(mostLikelyLastLoc >= minLoc && mostLikelyLastLoc <= maxLoc);

		for(ull tm = maxTime - 1; tm >= minTime; tm--)
		{
			ull predecessorIndex = GET_INDEX_3D(userIndex, ((tm + 1) - minTime), (predecessorLoc - minLoc), numTimes, numLoc);
			predecessorLoc = predecessor[predecessorIndex];

			VERIFY(predecessorLoc >= minLoc && predecessorLoc <= maxLoc);

			index = GET_INDEX(userIndex, (tm - minTime), numTimes);
			mostLikelyTrace[index] = predecessorLoc;
		}

		// compute the likelihood, we will need it later
		double logLikelihood = 0.0;
		for(ull tm = minTime; tm <= maxTime-1; tm++)
		{
			ull tp = Parameters::GetInstance()->LookupTimePeriod(tm);
			ull nexttp = tp; // ensure nexttp is always consistent with its usage
			if(tm < maxTime) { nexttp = Parameters::GetInstance()->LookupTimePeriod(tm + 1); }
			if(nexttp == INVALID_TIME_PERIOD || tp == INVALID_TIME_PERIOD)
			{
				SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
				return false;
			}

			index = GET_INDEX(userIndex, (tm - minTime), numTimes);
			ull loc = mostLikelyTrace[index];

			if(tm == minTime) // initialization
			{
				double* subChainSteadyStateVector = NULL;
				VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true);

				double presenceProb = subChainSteadyStateVector[loc - minLoc];
				double logpp = log(presenceProb);

				Free(subChainSteadyStateVector);

				if(presenceProb <= 0.0 || logpp == nan("n-char-sequence"))
				{
					logpp = log(SQRT_DBL_MIN); // avoid log overflow/underflow/nan
				}

				logLikelihood += logpp; // use logarithms to avoid underflow
			}
			else
			{
				ull index2 = GET_INDEX(userIndex, (tm+1 - minTime), numTimes);
				ull loc2 = mostLikelyTrace[index2];

				// get the proper sub-chain transition vector to the time period of the previous event
				double* subChainTransitionVector = NULL;
				VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, tp, loc, nexttp, &subChainTransitionVector) == true);

				ull nextLocIdx = (loc2 - minLoc);
				double transProb = subChainTransitionVector[nextLocIdx];

				Free(subChainTransitionVector);  // free the sub-chain transition vector

				double logtp = log(transProb);

				if(transProb <= 0.0 || logtp == nan("n-char-sequence"))
				{
					logtp = log(SQRT_DBL_MIN); // avoid log overflow/underflow/nan
				}
				logLikelihood += logtp;  // use logarithms to avoid underflow

			}
		}
		logLikelihoods[userIndex] = logLikelihood;

		userIndex++;
	}

	Free(delta);
	Free(predecessor);

	return true;

  // Bouml preserved body end 0007C991
}
