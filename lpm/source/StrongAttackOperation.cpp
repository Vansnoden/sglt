//!
//! \file
//!
#include "../include/StrongAttackOperation.h"
#include "../include/TraceSet.h"
#include "../include/ObservedEvent.h"
#include "../include/UserProfile.h"
#include "../include/MetricOperation.h"
#include "../include/AttackOutput.h"

namespace lpm {

StrongAttackOperation::StrongAttackOperation(bool genericRec, ull genericRecSamples, bool viterbiNotAlphaBeta)
			: AttackOperation("StrongAttackOperation")
{
  // Bouml preserved body begin 0004CD91

	actualTraceHint = NULL;
	exposedTraceHint = NULL;
	sigmaHint = NULL;

	genericReconstruction = genericRec;
	genericReconstructionSamples = genericRecSamples;

	viterbiInsteadOfAlphaBeta = viterbiNotAlphaBeta;

  // Bouml preserved body end 0004CD91
}

StrongAttackOperation::~StrongAttackOperation() 
{
  // Bouml preserved body begin 0004CE11
  // Bouml preserved body end 0004CE11
}

bool StrongAttackOperation::CreateMetric(MetricType type, MetricOperation** metric) const 
{
  // Bouml preserved body begin 0004CE91


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
		default:
			CODING_ERROR;
			break;
	}

	return false;

  // Bouml preserved body end 0004CE91
}

bool StrongAttackOperation::Execute(const TraceSet* input, AttackOutput* output) 
{
  // Bouml preserved body begin 0004CF91

/**/
	const double bigNumber = 1e20;
	const double bigNumberInverse = 1.0 / bigNumber;
/**/

	if(input == NULL || output == NULL || context == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	stringstream info("");
	info << "Starting the strong attack!";
	Log::GetInstance()->Append(info.str());

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get user profiles
	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	ull Nusers = profiles.size();

	map<ull, ull> userToPseudonymMapping = map<ull, ull>();
	ull* mostLikelyTrace = NULL;

	ull** mostLikelyTraceArray = NULL;
	double** mostLikelyTraceLLArray = NULL;

	bool persistentPseudonyms = (CONTAINS_FLAG(lppmFlags, PseudonymChange) == false);
	if(persistentPseudonyms == true)
	{
		// de-anonymization
		ull byteSize = (Nusers * Nusers) * sizeof(double);
		double* likelihoodMatrix = (double*)Allocate(byteSize);
		VERIFY(likelihoodMatrix != NULL);

		// for alpha-beta
		// normalization variable
		double* lrnrm = NULL;

		double* alpha = NULL;
		double* beta = NULL;

		if(viterbiInsteadOfAlphaBeta == true)
		{
			if(CONTAINS_FLAG(lppmFlags, Anonymization))
			{
				info.str("");
				info << "Computing likelihoods of users - pseudonyms mapping using Viterbi.";
				Log::GetInstance()->Append(info.str());

				map<ull, Trace*> traceMapping;
				input->GetMapping(traceMapping);

				VERIFY(Nusers == traceMapping.size());

				ull byteSizeVector = Nusers * sizeof(ll);

				ull* pseudonyms = (ull*)Allocate(byteSizeVector);
				VERIFY(pseudonyms != NULL);	memset(pseudonyms, 0, byteSizeVector);

				ull* users = (ull*)Allocate(byteSizeVector);
				VERIFY(users != NULL);	memset(users, 0, byteSizeVector);

				ull i = 0;
				pair_foreach_const(map<ull, UserProfile*>, profiles, userIter)	{ users[i++] = userIter->first; }

				ull j = 0;
				pair_foreach_const(map<ull, Trace*>, traceMapping, pseudonymIter) { pseudonyms[j++] = pseudonymIter->first; }

				ull mostLikelyTraceArrayByteSize = sizeof(ull*) * Nusers;
				mostLikelyTraceArray = (ull**)Allocate(mostLikelyTraceArrayByteSize);
				VERIFY(mostLikelyTraceArray != NULL); memset(mostLikelyTraceArray, 0, mostLikelyTraceArrayByteSize);

				ull mostLikelyTraceLLArrayByteSize = sizeof(double*) * Nusers;
				mostLikelyTraceLLArray = (double**)Allocate(mostLikelyTraceLLArrayByteSize);
				VERIFY(mostLikelyTraceLLArray != NULL); memset(mostLikelyTraceLLArray, 0, mostLikelyTraceLLArrayByteSize);

				for(ull nymIndex = 0; nymIndex < Nusers; nymIndex++)
				{
					ull mostLikelyTraceByteSize = Nusers * numTimes * sizeof(ull);
					mostLikelyTrace = (ull*)Allocate(mostLikelyTraceByteSize);
					VERIFY(mostLikelyTrace != NULL); memset(mostLikelyTrace, 0, mostLikelyTraceByteSize);

					ull logLikelihoodsByteSize = Nusers * sizeof(double);
					double* logLikelihoods =(double*)Allocate(logLikelihoodsByteSize);
					VERIFY(logLikelihoods != NULL);	memset(logLikelihoods, 0, logLikelihoodsByteSize);

					map<ull, ull> hypotheticalUserToPseudonymMap;
					for(ull i = 0; i < Nusers; i++)
					{
						ull user = users[i];
						ull nym = pseudonyms[(i + nymIndex) % Nusers];

						hypotheticalUserToPseudonymMap.insert(make_pair(user, nym));
					}

					bool ok = ComputeMostLikelyTrace(input, hypotheticalUserToPseudonymMap, mostLikelyTrace, logLikelihoods);
					VERIFY(ok == true);

					for(ull userIndex = 0; userIndex < Nusers; userIndex++)
					{
						ull effNymIndex = (userIndex + nymIndex) % Nusers;
						likelihoodMatrix[GET_INDEX(userIndex, effNymIndex, Nusers)] = logLikelihoods[userIndex];
					}

					mostLikelyTraceArray[nymIndex] = mostLikelyTrace;
					mostLikelyTraceLLArray[nymIndex] = logLikelihoods;
					//Free(mostLikelyTrace);
					//Free(logLikelihoods);
				}
			}
			else
			{
				info.str("");
				info << "LPPM does not anonymize, skipping maximum weight assignment computation.";
				Log::GetInstance()->Append(info.str());

				for(ull userIndex = 0; userIndex < Nusers; userIndex++)
				{
					likelihoodMatrix[GET_INDEX(userIndex, userIndex, Nusers)] = 1.0;
				}
			}
		}
		else
		{
			info.str("");
			info << "Computing alpha and beta!";
			Log::GetInstance()->Append(info.str());

			//compute alpha and beta matrices for all users, pseudonyms, times, and locations
			VERIFY(ComputeAlphaBeta(input, &alpha, &beta, &lrnrm) == true);
			VERIFY(alpha != NULL && beta != NULL && lrnrm != NULL);

			for (ull userIndex = 0; userIndex < Nusers; userIndex++)
			{
				for (ull nymIndex = 0; nymIndex < Nusers; nymIndex++)
				{
					double sum = 0.0;
					for (ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull index = GET_INDEX_4D(userIndex, nymIndex, (maxTime - minTime), (loc - minLoc), Nusers, numTimes, numLoc);
						sum += alpha[index];
					}

					/*
					stringstream info("");
					info << "likelihood: " << (double)sum << "   " << DBL_MIN;
					Log::GetInstance()->Append(info.str());
					*/

					if(sum <= 0.0)
					{
						sum = DBL_MIN;

		/**/
						lrnrm[GET_INDEX(userIndex, nymIndex, Nusers)] = 0;
		/**/
					}
					else
					{
						// re-normalization
		/**/
						while (sum < bigNumberInverse)
						{
							sum *= bigNumber;
							lrnrm[GET_INDEX(userIndex, nymIndex, Nusers)]++;
						}
		/**/
					}

		/**/
					likelihoodMatrix[GET_INDEX(userIndex, nymIndex, Nusers)] = log(sum) + lrnrm[GET_INDEX(userIndex, nymIndex, Nusers)] * log(bigNumberInverse);
		/**/
		//			likelihoodMatrix[GET_INDEX(userIndex, nymIndex, Nusers)] = log(sum);
				}
			}

		/**/
			Free(lrnrm);
	/**/
		}

		ull byteSizeMatrix = (Nusers * Nusers) * sizeof(ll);
		ull byteSizeVector = Nusers * sizeof(ll);

		ll* mapping = (ll*)Allocate(byteSizeVector);
		VERIFY(mapping != NULL); memset(mapping, 0, byteSizeVector);

		if(CONTAINS_FLAG(lppmFlags, Anonymization) == true)
		{
			// log the likelihood matrix
			Log::GetInstance()->Append("Likelihood Matrix (strong adv):");
			for(ull i = 0; i < Nusers; i++)
			{
				stringstream info("");

				for(ull j = 0; j < Nusers; j++)
				{
					info << " " << likelihoodMatrix[GET_INDEX(i, j, Nusers)];
				}

				Log::GetInstance()->Append(info.str());
			}

			// allocate weight and mapping
			ll* weight = (ll*)Allocate(byteSizeMatrix);
			VERIFY(weight != NULL);	memset(weight, 0, byteSizeMatrix);


			// convert likelihood matrix to weight matrix
			double maxnll = 0.0;
			for(ull i = 0; i < Nusers; i++)
			{
				for(ull j = 0; j < Nusers; j++)
				{
					ull index = GET_INDEX(i, j, Nusers);

					double nll = -likelihoodMatrix[index];
					if(nll > maxnll) { maxnll = nll; }
				}
			}

			ll llhoodBigNumber = (maxnll == 0.0) ? -1 : (ll)(-(double)LONG_LONG_MAX/maxnll + 10000);
			for(ull i = 0; i < Nusers; i++)
			{
				for(ull j = 0; j < Nusers; j++)
				{
					ull index = GET_INDEX(i, j, Nusers);
					double llhood = likelihoodMatrix[index];
					if(llhood > 0) { weight[index] = 0; }
					else { weight[index] = (ll)(llhoodBigNumber * llhood); }
				}
			}

			Free(likelihoodMatrix); likelihoodMatrix = NULL;

			// log the weight matrix
			Log::GetInstance()->Append("Weight Matrix (strong adv):");
			for(ull i = 0; i < Nusers; i++)
			{
				stringstream info(""); info.precision(16);

				for(ull j = 0; j < Nusers; j++)
				{
					info << " " << weight[GET_INDEX(i, j, Nusers)];
				}

				Log::GetInstance()->Append(info.str());
			}

			//de-anonymization
			// prev code: VERIFY(Algorithms::MaximumWeightAssignment(Nusers, weight, mapping) == 0); // this method was called maximum weight assignment but it was computing minimum cost assignement!

			ll* costMatrix = weight;
			Algorithms::MinimumCostAssignment(costMatrix, Nusers, mapping); // Note: the weight matrix is indeed a cost matrix, the mapping which maximizes the likelihood is the minimum cost assignment!

			Free(weight); weight = costMatrix = NULL;
		}
		else
		{
			// LPPM does not anonymize
			for(ull k = 0; k < Nusers; k++)
			{
				mapping[k] = k; // mapping for userIndex k, is nymIndex k.
			}
		}

		// get mapping (pseudonym -> observed trace): logging and attack output
		{
			map<ull, Trace*> traceMapping = map<ull, Trace*>();
			input->GetMapping(traceMapping);

			VERIFY(Nusers == traceMapping.size());

			ull* pseudonyms = (ull*)Allocate(byteSizeVector);
			VERIFY(pseudonyms != NULL);
			memset(pseudonyms, 0, byteSizeVector);

			ull* users = (ull*)Allocate(byteSizeVector);
			VERIFY(users != NULL);
			memset(users, 0, byteSizeVector);

			ull i = 0;
			pair_foreach_const(map<ull, UserProfile*>, profiles, userIter) { users[i++] = userIter->first; }

			ull j = 0;
			pair_foreach_const(map<ull, Trace*>, traceMapping, pseudonymIter) { pseudonyms[j++] = pseudonymIter->first; }

			for(ull k = 0; k < Nusers; k++)
			{
				ull pseudonym = pseudonyms[mapping[k]];
				ull user = users[k];
				userToPseudonymMapping.insert(pair<ull, ull>(user, pseudonym));
			}

			VERIFY(userToPseudonymMapping.size() == Nusers);

			pair_foreach_const(map<ull, ull>, userToPseudonymMapping, iter)
			{
				stringstream info("");
				info << "Assignment: user -> pseudonym " << iter->first << " " << iter->second;
				Log::GetInstance()->Append(info.str());
			}

			output->SetAnonymizationMap(userToPseudonymMapping); // set the mapping

			Free(pseudonyms);
			Free(users);
		}

		ull mostLikelyTraceByteSize = Nusers * numTimes * sizeof(ull);
		mostLikelyTrace = (ull*)Allocate(mostLikelyTraceByteSize);
		VERIFY(mostLikelyTrace != NULL); memset(mostLikelyTrace, 0, mostLikelyTraceByteSize);

		ull logLikelihoodsByteSize = Nusers * sizeof(double);
		double* logLikelihoods =(double*)Allocate(logLikelihoodsByteSize);
		VERIFY(logLikelihoods != NULL);	memset(logLikelihoods, 0, logLikelihoodsByteSize);

		if(mostLikelyTraceArray != NULL && mostLikelyTraceLLArray != NULL)
		{
			// we've already done the computation of the most likely trace, so no need to rerun Viterbi, just extract it!
			for(ull nymIndex = 0; nymIndex < Nusers; nymIndex++)
			{
				ull userIndex = 0;
				for(userIndex = 0; userIndex < Nusers; userIndex++) { if(nymIndex == mapping[userIndex]) { break; } }

				ull effIndex = (nymIndex - userIndex + Nusers) % Nusers;
				ull* mostLikelyTraceTmp = mostLikelyTraceArray[effIndex];
				VERIFY(mostLikelyTraceTmp != NULL);

				for(ull tm = minTime; tm <= maxTime; tm++)
				{
					ull index = GET_INDEX(userIndex, (tm - minTime), numTimes);
					ull tmpIndex = GET_INDEX(userIndex, (tm - minTime), numTimes);
					mostLikelyTrace[index] = mostLikelyTraceTmp[tmpIndex];
				}

				double* tmpMostLikelyTraceLLTmp = mostLikelyTraceLLArray[effIndex];
				VERIFY(tmpMostLikelyTraceLLTmp != NULL);

				logLikelihoods[userIndex] = tmpMostLikelyTraceLLTmp[userIndex];

				{
					stringstream ssl(""); ssl << "(userIndex, nymIndex) = (" << userIndex << ", " << nymIndex << ") - ll: " << logLikelihoods[userIndex];
					Log::GetInstance()->Append(ssl.str());
				}
			}

			for(ull k = 0; k < Nusers; k++)
			{
				Free(mostLikelyTraceArray[k]);
				Free(mostLikelyTraceLLArray[k]);
			}

			Free(mostLikelyTraceArray); mostLikelyTraceArray = NULL;
			Free(mostLikelyTraceLLArray); mostLikelyTraceLLArray = NULL;
		}
		else
		{
			// tracking
			VERIFY(ComputeMostLikelyTrace(input, userToPseudonymMapping, mostLikelyTrace, logLikelihoods) == true);
		}

		output->SetMostLikelyTrace(mostLikelyTrace);
		output->SetMostLikelyTraceLL(logLikelihoods);


		if(viterbiInsteadOfAlphaBeta == false)
		{
			// de-obfuscation
			ull outputByteSize = Nusers * numTimes * numLoc * sizeof(double);
			double* locationDistribution = (double*)Allocate(outputByteSize);
			VERIFY(locationDistribution != NULL);
			memset(locationDistribution, 0, outputByteSize);


			// for each user
			for(ull userIndex = 0; userIndex < Nusers; userIndex++)
			{
				for (ull tm = minTime; tm <= maxTime; tm++)
				{
					double sum = 0.0;

					for (ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull index = GET_INDEX_4D(userIndex, mapping[userIndex], (tm - minTime), (loc - minLoc), Nusers, numTimes, numLoc);

						double product = 0.0;
						product = alpha[index] * beta[index] * bigNumber;


						stringstream info("");
						info << "alpha beta " << userIndex << " " << tm << " " << loc << " | " << alpha[index] << " * " << beta[index] << " = " << product;
						Log::GetInstance()->Append(info.str());


						ull index2 = GET_INDEX_3D(userIndex, (tm - minTime), (loc - minLoc), numTimes, numLoc);

						locationDistribution[index2] = product;

						sum += product;

						stringstream info2("");
						info2 << "       sum " << userIndex << " " << tm << " " << loc << " | " << sum;
						Log::GetInstance()->Append(info2.str());

					}

					for (ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull index2 = GET_INDEX_3D(userIndex, (tm - minTime), (loc - minLoc), numTimes, numLoc);

						double tmp = locationDistribution[index2];
						VERIFY(sum != 0.0 && (tmp/sum) != nan("n-char-sequence"));
						locationDistribution[index2] /= (double)sum;
					}
				}
			}


			Free(alpha);
			Free(beta);

			output->SetProbabilityDistribution(locationDistribution);
		}
		Free(mapping);
	}

	if(persistentPseudonyms == false || genericReconstruction == true)
	{
		VERIFY(GenericReconstruction(input, userToPseudonymMapping, mostLikelyTrace, output) == true);
	}
	// userToPseudonymMapping.clear(); // no need to clear it....

	return true;

  // Bouml preserved body end 0004CF91
}

void StrongAttackOperation::SetGenericReconstructionHint(const TraceSet* actual, const TraceSet* exposed, ull* sigma) 
{
  // Bouml preserved body begin 000CBD11

	actualTraceHint = const_cast<TraceSet*>(actual);
	exposedTraceHint = const_cast<TraceSet*>(exposed);
	sigmaHint = sigma;

  // Bouml preserved body end 000CBD11
}

bool StrongAttackOperation::ComputeAlphaBeta(const TraceSet* traces, double** alpha, double** beta, double** lrnrm) const 
{
  // Bouml preserved body begin 0001F582

/**/
	const double bigNumber = 1e20;
	const double bigNumberInverse = 1.0 / bigNumber;
/**/

	if(traces == NULL || alpha == NULL || beta == NULL)
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

	//allocate memory for Alpha and Beta
	ull byteSizeAB = (Nusers * Nusers * numTimes * numLoc) * sizeof(double);

	double* myalpha = *alpha = (double*)Allocate(byteSizeAB);
	VERIFY(myalpha != NULL);
	memset(myalpha, 0, byteSizeAB);

	double* mybeta = *beta = (double*)Allocate(byteSizeAB);
	VERIFY(mybeta != NULL);
	memset(mybeta, 0, byteSizeAB);

	// normalization variables
/**/
	ull byteSizeNorm = Nusers * Nusers * numTimes * sizeof(double);

	double* arnrm = (double*)Allocate(byteSizeNorm);
	VERIFY(arnrm != NULL);
	memset(arnrm, 0, byteSizeNorm);

	double* mylrnrm = *lrnrm = (double*)Allocate(Nusers * Nusers * sizeof(double));
	VERIFY(mylrnrm != NULL);
	memset(mylrnrm, 0, Nusers * Nusers * sizeof(double));
/**/

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

		// for all observed traces
		ull pseudonymIndex = 0;
		pair_foreach_const(map<ull, Trace*>, mappingNymObserved, pseudonymsIter)
		{
			//ull pseudonym = pseudonymsIter->first;
			Trace* observedTrace = pseudonymsIter->second;

			vector<Event*> events = vector<Event*>();
			observedTrace->GetEvents(events);

			VERIFY(numTimes == events.size());

			// compute alpha
/*
*/
			// for all time instants
			ull tm = minTime;
			foreach_const(vector<Event*>, events, eventsIter)
			{
				double asum = 0.0;

				ObservedEvent* observedEvent = dynamic_cast<ObservedEvent*>(*eventsIter);
				set<ull> timestamps = set<ull>();
				observedEvent->GetTimestamps(timestamps);

				VERIFY(timestamps.size() == 1);

				ull timestamp = *(timestamps.begin());

				VERIFY(tm == timestamp && (timestamp >= minTime && timestamp <= maxTime));

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
				VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true);


				for(ull loc = minLoc; loc <= maxLoc; loc++)
				{
					ActualEvent* actualEvent = new ActualEvent(user, timestamp, loc);
					ExposedEvent* exposedEvent = new ExposedEvent(*actualEvent);

					VERIFY(actualEvent != NULL && exposedEvent != NULL);

					double lppmProb0 = lppmPDF->PDF(context, actualEvent, observedEvent);
					double applicationProb0 = applicationPDF->PDF(context, actualEvent, actualEvent);

					double lppmProb1 = lppmPDF->PDF(context, exposedEvent, observedEvent);
					double applicationProb1 = applicationPDF->PDF(context, actualEvent, exposedEvent);

					actualEvent->Release();
					exposedEvent->Release();

					double presenceProb = subChainSteadyStateVector[loc - minLoc];

					ull index = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime), (loc - minLoc), Nusers, numTimes, numLoc);

					// compute alpha_1
					if (timestamp == minTime)
					{
						double prob = 0.0;
						prob = (double)presenceProb * ((lppmProb0 * applicationProb0 ) + (lppmProb1 * applicationProb1));

						myalpha[index] = prob;
					}
					else // compute alpha_t
					{
						double sum = 0.0;
						for(ull prevloc = minLoc; prevloc <= maxLoc; prevloc++)
						{
							ull index1 = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime - 1), (prevloc - minLoc), Nusers, numTimes, numLoc);
							double previousAlpha = myalpha[index1];

							// get the proper sub-chain transition vector to the time period of the previous event (we're computing alpha, remember?)
							double* subChainTransitionVector = NULL;
							VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, prevtp, prevloc, tp, &subChainTransitionVector) == true);

							ull index2 = (loc - minLoc);
							double transitionProb = subChainTransitionVector[index2];

							Free(subChainTransitionVector);  // free the sub-chain transition vector

							sum += previousAlpha * transitionProb;
						}

						double prob = 0.0;
						prob = (double)sum * ((lppmProb0 * applicationProb0 ) + (lppmProb1 * applicationProb1));

						myalpha[index] = prob;

						// take care of small alpha
						//VERIFY(prob == 0.0 || prob > bigNumberInverse);
					}

					asum += myalpha[index];
				}

				Free(subChainSteadyStateVector); // free the sub-chain steady-state vector

				// Re-normalize the alpha's s necessary to avoid underflow, keeping track of how many re-normalizations for each alpha
/**/
				if (timestamp == minTime)
				{
					arnrm[GET_INDEX_3D(userIndex, pseudonymIndex, 0, Nusers, numTimes)] = 0;
				}
				else
				{
					arnrm[GET_INDEX_3D(userIndex, pseudonymIndex, (timestamp - minTime), Nusers, numTimes)] = arnrm[GET_INDEX_3D(userIndex, pseudonymIndex, (timestamp - minTime - 1), Nusers, numTimes)];
				}

				while (asum < bigNumberInverse)
				{
					++arnrm[GET_INDEX_3D(userIndex, pseudonymIndex, timestamp - minTime, Nusers, numTimes)];

					asum = 0.0;

					for (ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull index = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime), (loc - minLoc), Nusers, numTimes, numLoc);
						myalpha[index] *= bigNumber;
						asum += myalpha[index];
					}

					// asum is not supposed to be 0.
					VERIFY(asum != 0.0);
					VERIFY(asum != nan("n-char-sequence"));
					VERIFY(asum != -nan("n-char-sequence"));
				}
/**/
				tm++;
				// Done with the re-normalization of alpha
			}

			//keeping track of how many re-normalizations for alpha of user u and pseudonym u'
/**/
			mylrnrm[GET_INDEX(userIndex, pseudonymIndex, Nusers)] = arnrm[GET_INDEX_3D(userIndex, pseudonymIndex, maxTime - minTime, Nusers, numTimes)];
/**/


			ObservedEvent* prevObservedEvent = NULL;


			// compute beta

			// for all time instants
			tm = maxTime;
			foreach_const_reverse(vector<Event*>, events, eventsIter)
			{
				double bsum = 0.0;

				ObservedEvent* observedEvent = dynamic_cast<ObservedEvent*>(*eventsIter);
				set<ull> timestamps = set<ull>();
				observedEvent->GetTimestamps(timestamps);

				VERIFY(timestamps.empty() == false);

				ull timestamp = *(timestamps.begin());

				VERIFY(tm == timestamp && (timestamp >= minTime && timestamp <= maxTime));

				ull tp = Parameters::GetInstance()->LookupTimePeriod(timestamp);
				ull nexttp = tp; // ensure nexttp is always consistent with its usage
				if(timestamp < maxTime) { nexttp = Parameters::GetInstance()->LookupTimePeriod(timestamp + 1); }
				if(nexttp == INVALID_TIME_PERIOD || tp == INVALID_TIME_PERIOD)
				{
					SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
					return false;
				}

				for(ull loc = minLoc; loc <= maxLoc; loc++)
				{
					ull index = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime), (loc - minLoc), Nusers, numTimes, numLoc);

					// compute beta_T
					if (timestamp == maxTime)
					{
						mybeta[index] = 1.0;
					}
					else // compute beta_t
					{
						double sum = 0.0;

						for(ull nextloc = minLoc; nextloc <= maxLoc; nextloc++)
						{
							ActualEvent* actualEvent = new ActualEvent(user, timestamp + 1, nextloc);
							ExposedEvent* exposedEvent = new ExposedEvent(*actualEvent);

							VERIFY(actualEvent != NULL && exposedEvent != NULL);

							double lppmProb0 = lppmPDF->PDF(context, actualEvent, prevObservedEvent);
							double applicationProb0 = applicationPDF->PDF(context, actualEvent, actualEvent);

							double lppmProb1 = lppmPDF->PDF(context, exposedEvent, prevObservedEvent);
							double applicationProb1 = applicationPDF->PDF(context, actualEvent, exposedEvent);

							actualEvent->Release();
							exposedEvent->Release();

							ull index2 = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime + 1), (nextloc - minLoc), Nusers, numTimes, numLoc);
							double nextBeta = mybeta[index2];

							// get the proper sub-chain transition vector to the time period of the next event (we're computing beta, remember?)
							double* subChainTransitionVector = NULL;
							VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, tp, loc, nexttp, &subChainTransitionVector) == true);

							ull index3 = (nextloc - minLoc);
							double transitionProb = subChainTransitionVector[index3];

							Free(subChainTransitionVector);  // free the sub-chain transition vector

							sum += (double)nextBeta * transitionProb * ((lppmProb0 * applicationProb0 ) + (lppmProb1 * applicationProb1));
						}

						//ull index = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime), (loc - minLoc), Nusers, numTimes, numLoc);
						mybeta[index] = sum;

						// take care of small beta
						//VERIFY(sum == 0.0 || sum > bigNumberInverse);
					}

					bsum += mybeta[index];
				}

				prevObservedEvent = observedEvent;

				// Re-normalize the betas; necessary to avoid underflow
/**/
				while (bsum < bigNumberInverse)
				{
					bsum = 0.0;

					for (ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull index = GET_INDEX_4D(userIndex, pseudonymIndex, (timestamp - minTime), (loc - minLoc), Nusers, numTimes, numLoc);
						mybeta[index] *= bigNumber;
						bsum += mybeta[index];
					}

					// bsum is not supposed to be 0.
					VERIFY(bsum != 0.0);
					VERIFY(bsum != nan("n-char-sequence"));
					VERIFY(bsum != -nan("n-char-sequence"));
				}
/**/
				// Done with the re-normalization of beta
				tm--;
			}

			stringstream info("");
			info << "Alpha-Beta: user : pseudonym " << userIndex << " " << pseudonymIndex;
			Log::GetInstance()->Append(info.str());

			pseudonymIndex++;
		}

		userIndex++;
	}

/**/
	Free(arnrm);
/**/

	return true;

  // Bouml preserved body end 0001F582
}

bool StrongAttackOperation::ComputeMostLikelyTrace(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, ull* mostLikelyTrace, double* logLikelihoods)
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

bool StrongAttackOperation::ProposeSampleA(StrongAttackOperation::GenericReconstructionProposalPackage* package, bool training) const 
{
  // Bouml preserved body begin 000D2711

	VERIFY(package != NULL);

	Parameters* params = Parameters::GetInstance();

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(params->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	//ull numLoc = maxLoc - minLoc + 1;

	RNG* rng = RNG::GetInstance();

	double u = 0.0;
	if(training == false) { u = rng->GetUniformRandomDouble(); }

	// extract stuff in our proposal package
    ull step = package->step;
    ull* currentSampleA = package->currentSampleA;
    bool* currentSampleX = package->currentSampleX;
    ull* currentSampleSigma = package->currentSampleSigma;
    ObservedEvent** observedMatrix = package->observedMatrix;
    ull user = package->user;
    ull userIdx = package->userIdx;
    UserProfile* profile = package->profile;
    ull tm = package->tm;
    ull tmIdx = package->tmIdx;

    double* transitionsMatrix = NULL; double* steadyStateVector = NULL;
    VERIFY(profile->GetTransitionMatrix(&transitionsMatrix) == true && transitionsMatrix != NULL);
    VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true && steadyStateVector != NULL);

    // logic starts here
    // 1. get the indices and the events for the current sample
    ull thisMatrixIdx = GET_INDEX(userIdx, tmIdx, numTimes); ull prevMatrixIdx = 0; ull nextMatrixIdx = 0;
	if(tmIdx >= 1) { prevMatrixIdx = GET_INDEX(userIdx, tmIdx - 1, numTimes); }
	if(tmIdx < numTimes - 1) { nextMatrixIdx = GET_INDEX(userIdx, tmIdx + 1, numTimes); }

	// actual events, exposed events
	ull thisLoc = currentSampleA[thisMatrixIdx]; ull prevLoc = 0; ull nextLoc = 0; 	VERIFY(thisLoc >= minLoc && thisLoc <= maxLoc);
	bool thisExposure = currentSampleX[thisMatrixIdx]; bool prevExposure = false; bool nextExposure = false;

	ActualEvent* thisActualEvent = (thisExposure == true) ? new ExposedEvent(user, tm, thisLoc) : new ActualEvent(user, tm, thisLoc);
	ActualEvent* prevActualEvent = NULL; ActualEvent* nextActualEvent = NULL;

	if(tmIdx >= 1)
	{
		prevLoc = currentSampleA[prevMatrixIdx]; prevExposure = currentSampleX[prevMatrixIdx];
		prevActualEvent = (prevExposure == true) ? new ExposedEvent(user, tm - 1, prevLoc) : new ActualEvent(user, tm - 1, prevLoc);
	}
	if(tmIdx < numTimes - 1)
	{
		nextLoc = currentSampleA[nextMatrixIdx]; nextExposure = currentSampleX[nextMatrixIdx];
		nextActualEvent = (nextExposure == true) ? new ExposedEvent(user, tm + 1, nextLoc) : new ActualEvent(user, tm + 1, nextLoc);
	}

	// observed events
	ull thisObservedEventIndex = currentSampleSigma[thisMatrixIdx]; ull prevObservedEventIndex  = 0; ull nextObservedEventIndex = 0;
	if(tmIdx >= 1) { prevObservedEventIndex = currentSampleSigma[prevMatrixIdx]; }
	if(tmIdx < numTimes - 1) { nextObservedEventIndex = currentSampleSigma[nextMatrixIdx]; }

	ull thisObservedMatrixIdx = GET_INDEX(thisObservedEventIndex, tmIdx, numTimes);
	ObservedEvent* thisObservedEvent = observedMatrix[thisObservedMatrixIdx]; ObservedEvent* prevObservedEvent = NULL; ObservedEvent* nextObservedEvent = NULL;

	if(tmIdx >= 1)
	{
		ull prevObservedMatrixIdx = GET_INDEX(prevObservedEventIndex, tmIdx - 1, numTimes);
		prevObservedEvent = observedMatrix[prevObservedMatrixIdx];
	}
	if(tmIdx < numTimes - 1)
	{
		ull nextObservedMatrixIdx = GET_INDEX(nextObservedEventIndex, tmIdx + 1, numTimes);
		nextObservedEvent = observedMatrix[nextObservedMatrixIdx];
	}

	// 2. propose new sample and get/create events for the proposal
	ull proposedThisLoc = thisLoc;
	while(proposedThisLoc == thisLoc) { proposedThisLoc = rng->GetUniformRandomULLBetween(minLoc, maxLoc); }

	/** compute ratio **/
	// create the new actual event
	ActualEvent* proposedThisActualEvent = thisExposure == true ? new ExposedEvent(user, tm, proposedThisLoc) : new ActualEvent(user, tm, proposedThisLoc);

	// looks weird but its ok: we are asking for probability of exposing '(ActualEvent*)thisActualEvent' (if thisActualEvent is exposed)
	// or the probability of not exposing '(ActualEvent*)thisActualEvent' (if thisActualEvent is not exposed)
	double denomPartApp = applicationPDF->PDF(context, thisActualEvent, thisActualEvent);
	double numeratorPartApp = applicationPDF->PDF(context, proposedThisActualEvent, proposedThisActualEvent);

	double denomPartLPPM1 = lppmPDF->PDF(context, thisActualEvent, prevActualEvent, thisObservedEvent, prevObservedEvent);
	double numeratorPartLPPM1 = lppmPDF->PDF(context, proposedThisActualEvent, prevActualEvent, thisObservedEvent, prevObservedEvent);

	double denomPartLPPM2 = 1.0;
	double numeratorPartLPPM2 = 1.0;

	if(tmIdx < numTimes - 1)
	{
		denomPartLPPM2 = lppmPDF->PDF(context, nextActualEvent, thisActualEvent, nextObservedEvent, thisObservedEvent);
		numeratorPartLPPM2 = lppmPDF->PDF(context, nextActualEvent, proposedThisActualEvent, nextObservedEvent, thisObservedEvent);
	}

	// get time periods
	ull thisTP = params->LookupTimePeriod(tm, false, NULL, NULL); VERIFY(thisTP != INVALID_TIME_PERIOD);

	ull prevTP = INVALID_TIME_PERIOD; ull nextTP = INVALID_TIME_PERIOD;
	if(tmIdx >= 1) { prevTP = params->LookupTimePeriod(tm-1, false, NULL, NULL); VERIFY(prevTP != INVALID_TIME_PERIOD); }
	if(tmIdx < numTimes - 1) { nextTP = params->LookupTimePeriod(tm+1, false, NULL, NULL); VERIFY(nextTP != INVALID_TIME_PERIOD); }

	double probGoingToThis = 0.0; double probGoingToProposedThis = 0.0;
	if(tmIdx == 0) // use steady-state
	{
		double* subChainSS = NULL;
		VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, thisTP, &subChainSS, false) == true);
		probGoingToThis = subChainSS[(thisLoc - minLoc)];
		probGoingToProposedThis = subChainSS[(proposedThisLoc - minLoc)];
		Free(subChainSS);
	}
	else
	{
		double* subChainVector = NULL;
		VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionsMatrix, prevTP, prevLoc, thisTP, &subChainVector, false) == true);
		probGoingToThis = subChainVector[(thisLoc - minLoc)];
		probGoingToProposedThis = subChainVector[(proposedThisLoc - minLoc)];
		Free(subChainVector);
	}

	double probLeavingFromThis = 1.0; double probLeavingFromProposedThis = 1.0;
	if(tmIdx < numTimes - 1)
	{
		double* subChainVector = NULL;
		VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionsMatrix, thisTP, thisLoc, nextTP, &subChainVector, false) == true);
		probLeavingFromThis = subChainVector[(nextLoc - minLoc)];
		Free(subChainVector);

		subChainVector = NULL;
		VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionsMatrix, thisTP, proposedThisLoc, nextTP, &subChainVector, false) == true);
		probLeavingFromProposedThis = subChainVector[(nextLoc - minLoc)];
		Free(subChainVector);
	}

	double denomPartContext = probGoingToThis * probLeavingFromThis;
	double numeratorPartContext = probGoingToProposedThis * probLeavingFromProposedThis;

	//double denom = denomPartApp *  denomPartLPPM1 * denomPartLPPM2 * denomPartContext;
	//double numerator = numeratorPartApp * numeratorPartLPPM1 * numeratorPartLPPM2 * numeratorPartContext;

	// to make sure we don't overflow, compute the ratio like this
	double ratio = 1.0;
	ratio *= numeratorPartApp / denomPartApp; VERIFY(denomPartApp != 0.0);
	ratio *= numeratorPartLPPM1 / denomPartLPPM1; VERIFY(denomPartLPPM1 != 0.0);
	ratio *= numeratorPartLPPM2 / denomPartLPPM2; VERIFY(denomPartLPPM2 != 0.0);
	ratio *= numeratorPartContext / denomPartContext; VERIFY(denomPartContext != 0.0);

	// decide acceptance
	//VERIFY(denom != 0.0); // otherwise how did we get here??
	//double ratio = numerator / denom;

	bool accepted = false;
	if(u <= ratio)
	{
		accepted = true;
		currentSampleA[thisMatrixIdx] = proposedThisLoc; // update current sample
	}

	// cleanup
	thisActualEvent->Release();
	if(prevActualEvent != NULL) { prevActualEvent->Release(); }
	if(nextActualEvent != NULL) { nextActualEvent->Release(); }

	proposedThisActualEvent->Release();

/*	if(training == false)
	{
		// log stuff
		stringstream ss("");
		ss << "ProposeSampleA(): step " << step << ", user = " << user << ", tm = " << tm << " ratio: " << ratio << " - u = " << u;
		ss << " -> accepted: " << accepted;
		Log::GetInstance()->Append(ss.str());
	}*/

    return accepted;

  // Bouml preserved body end 000D2711
}

bool StrongAttackOperation::ProposeSampleX(StrongAttackOperation::GenericReconstructionProposalPackage* package, bool training) const 
{
  // Bouml preserved body begin 000D4111

	VERIFY(package != NULL);

	Parameters* params = Parameters::GetInstance();

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(params->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	//ull numLoc = maxLoc - minLoc + 1;

	double u = 0.0;
	if(training == false)
	{
		RNG* rng = RNG::GetInstance();
		u = rng->GetUniformRandomDouble();
	}

	// extract stuff in our proposal package
    ull step = package->step;
    ull* currentSampleA = package->currentSampleA;
    bool* currentSampleX = package->currentSampleX;
    ull* currentSampleSigma = package->currentSampleSigma;
    ObservedEvent** observedMatrix = package->observedMatrix;
    ull user = package->user;
    ull userIdx = package->userIdx;
    //UserProfile* profile = package->profile;
    ull tm = package->tm;
    ull tmIdx = package->tmIdx;

    // logic starts here
    // 1. get the indices and the events for the current sample
    ull thisMatrixIdx = GET_INDEX(userIdx, tmIdx, numTimes); ull prevMatrixIdx = 0; ull nextMatrixIdx = 0;
	if(tmIdx >= 1) { prevMatrixIdx = GET_INDEX(userIdx, tmIdx - 1, numTimes); }
	if(tmIdx < numTimes - 1) { nextMatrixIdx = GET_INDEX(userIdx, tmIdx + 1, numTimes); }

	// actual events, exposed events
	ull thisLoc = currentSampleA[thisMatrixIdx]; ull prevLoc = 0; ull nextLoc = 0; 	VERIFY(thisLoc >= minLoc && thisLoc <= maxLoc);
	bool thisExposure = currentSampleX[thisMatrixIdx]; bool prevExposure = false; bool nextExposure = false;

	ActualEvent* thisActualEvent = (thisExposure == true) ? new ExposedEvent(user, tm, thisLoc) : new ActualEvent(user, tm, thisLoc);
	ActualEvent* prevActualEvent = NULL; ActualEvent* nextActualEvent = NULL;

	if(tmIdx >= 1)
	{
		prevLoc = currentSampleA[prevMatrixIdx]; prevExposure = currentSampleX[prevMatrixIdx];
		prevActualEvent = (prevExposure == true) ? new ExposedEvent(user, tm - 1, prevLoc) : new ActualEvent(user, tm - 1, prevLoc);
	}
	if(tmIdx < numTimes - 1)
	{
		nextLoc = currentSampleA[nextMatrixIdx]; nextExposure = currentSampleX[nextMatrixIdx];
		nextActualEvent = (nextExposure == true) ? new ExposedEvent(user, tm + 1, nextLoc) : new ActualEvent(user, tm + 1, nextLoc);
	}

	// observed events
	ull thisObservedEventIndex = currentSampleSigma[thisMatrixIdx]; ull prevObservedEventIndex  = 0; ull nextObservedEventIndex = 0;
	if(tmIdx >= 1) { prevObservedEventIndex = currentSampleSigma[prevMatrixIdx]; }
	if(tmIdx < numTimes - 1) { nextObservedEventIndex = currentSampleSigma[nextMatrixIdx]; }

	ull thisObservedMatrixIdx = GET_INDEX(thisObservedEventIndex, tmIdx, numTimes);
	ObservedEvent* thisObservedEvent = observedMatrix[thisObservedMatrixIdx]; ObservedEvent* prevObservedEvent = NULL; ObservedEvent* nextObservedEvent = NULL;

	if(tmIdx >= 1)
	{
		ull prevObservedMatrixIdx = GET_INDEX(prevObservedEventIndex, tmIdx - 1, numTimes);
		prevObservedEvent = observedMatrix[prevObservedMatrixIdx];
	}
	if(tmIdx < numTimes - 1)
	{
		ull nextObservedMatrixIdx = GET_INDEX(nextObservedEventIndex, tmIdx + 1, numTimes);
		nextObservedEvent = observedMatrix[nextObservedMatrixIdx];
	}

    // 2. propose new sample and get/create events for the proposal
    bool proposedThisExposure = !thisExposure; // flip it

	/** compute ratio **/
	// create the new actual event
	ActualEvent* proposedThisActualEvent = proposedThisExposure == true ? new ExposedEvent(userIdx, tm, thisLoc) : new ActualEvent(userIdx, tm, thisLoc);

	double denomPartApp = applicationPDF->PDF(context, thisActualEvent, thisActualEvent);
	double numeratorPartApp = applicationPDF->PDF(context, proposedThisActualEvent, proposedThisActualEvent);

	double denomPartLPPM1 = lppmPDF->PDF(context, thisActualEvent, prevActualEvent, thisObservedEvent, prevObservedEvent);
	double numeratorPartLPPM1 = lppmPDF->PDF(context, proposedThisActualEvent, prevActualEvent, thisObservedEvent, prevObservedEvent);

	double denomPartLPPM2 = 1.0;
	double numeratorPartLPPM2 = 1.0;

	if(tmIdx < numTimes - 1)
	{
		denomPartLPPM2 = lppmPDF->PDF(context, nextActualEvent, thisActualEvent, nextObservedEvent, thisObservedEvent);
		numeratorPartLPPM2 = lppmPDF->PDF(context, nextActualEvent, proposedThisActualEvent, nextObservedEvent, thisObservedEvent);
	}

	// note: no change to actual loc. means no need to compute probability ratio for partial actual trace from context
	// double denom = denomPartApp *  denomPartLPPM1 * denomPartLPPM2;
	// double numerator = numeratorPartApp * numeratorPartLPPM1 * numeratorPartLPPM2;

	// to make sure we don't overflow, compute the ratio like this
	double ratio = 1.0;
	ratio *= numeratorPartApp / denomPartApp; VERIFY(denomPartApp != 0.0);
	ratio *= numeratorPartLPPM1 / denomPartLPPM1; VERIFY(denomPartLPPM1 != 0.0);
	ratio *= numeratorPartLPPM2 / denomPartLPPM2; VERIFY(denomPartLPPM2 != 0.0);

	// decide acceptance
	//VERIFY(denom != 0.0); // otherwise how did we get here??
	//double ratio = numerator / denom;

	bool accepted = false;
	if(u <= ratio)
	{
		accepted = true;

		// update current sample
		currentSampleX[thisMatrixIdx] = proposedThisExposure;
	}

    // cleanup
	thisActualEvent->Release();
	if(prevActualEvent != NULL) { prevActualEvent->Release(); }
	if(nextActualEvent != NULL) { nextActualEvent->Release(); }

	proposedThisActualEvent->Release();

/*	if(training == false)
	{
		// log stuff
		stringstream ss("");
		ss << "ProposeSampleX(): step " << step << ", user = " << user << ", tm = " << tm << " ratio: " << ratio << " - u = " << u;
		ss << " -> accepted: " << accepted;
		Log::GetInstance()->Append(ss.str());
	}*/

	return accepted;

  // Bouml preserved body end 000D4111
}

bool StrongAttackOperation::ProposeSampleSigma(StrongAttackOperation::GenericReconstructionProposalPackage* package, bool training) const 
{
  // Bouml preserved body begin 000D4191

	VERIFY(package != NULL);

	Parameters* params = Parameters::GetInstance();

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(params->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	//ull numLoc = maxLoc - minLoc + 1;

	RNG* rng = RNG::GetInstance();

	double u = 0.0;
	if(training == false) { u = rng->GetUniformRandomDouble(); }

	// extract stuff in our proposal package
    ull step = package->step;
    ull* currentSampleA = package->currentSampleA;
    bool* currentSampleX = package->currentSampleX;
    ull* currentSampleSigma = package->currentSampleSigma;
    ObservedEvent** observedMatrix = package->observedMatrix;
    vector<ull> users = package->users;
    ull user = package->user;
    ull userIdx = package->userIdx;
    //UserProfile* profile = package->profile;
    ull tm = package->tm;
    ull tmIdx = package->tmIdx;

    ull Nusers = users.size();

    bool persistentPseudonyms = CONTAINS_FLAG(lppmFlags, PseudonymChange) == false;

    // logic starts here
	// determine two users whose observed events we want to swap
	ull otherUserIdx = userIdx;
	while(otherUserIdx == userIdx) { otherUserIdx = rng->GetUniformRandomULLBetween(0, Nusers-1); }
	ull otherUser = users[otherUserIdx];

	const ull timeWindow = (persistentPseudonyms == true) ? numTimes : 1; // for now

	vector<double> numeratorLog = vector<double>();
	vector<double> denomLog = vector<double>();

	ull i = 0;
	for(ull tm2Idx = 0; tm2Idx < numTimes; tm2Idx++, i++)
	{
		if(timeWindow == 1) { tm2Idx = tmIdx; }

		tmIdx = tm2Idx; tm = minTime + tmIdx;

		// get observed and actual events for userIdx
		ull thisMatrixIdx = GET_INDEX(userIdx, tmIdx, numTimes); ull prevMatrixIdx = 0; ull nextMatrixIdx = 0;
		if(tmIdx >= 1) { prevMatrixIdx = GET_INDEX(userIdx, tmIdx - 1, numTimes); }
		if(tmIdx < numTimes - 1) { nextMatrixIdx = GET_INDEX(userIdx, tmIdx + 1, numTimes); }

		ull thisLoc = currentSampleA[thisMatrixIdx]; ull prevLoc = 0; ull nextLoc = 0; 	VERIFY(thisLoc >= minLoc && thisLoc <= maxLoc);
		bool thisExposure = currentSampleX[thisMatrixIdx]; bool prevExposure = false; bool nextExposure = false;

		ActualEvent* thisActualEvent = (thisExposure == true) ? new ExposedEvent(user, tm, thisLoc) : new ActualEvent(user, tm, thisLoc);
		ActualEvent* prevActualEvent = NULL; ActualEvent* nextActualEvent = NULL;

		if(tmIdx >= 1)
		{
			prevLoc = currentSampleA[prevMatrixIdx]; prevExposure = currentSampleX[prevMatrixIdx];
			prevActualEvent = (prevExposure == true) ? new ExposedEvent(user, tm - 1, prevLoc) : new ActualEvent(user, tm - 1, prevLoc);
		}
		if(tmIdx < numTimes - 1)
		{
			nextLoc = currentSampleA[nextMatrixIdx]; nextExposure = currentSampleX[nextMatrixIdx];
			nextActualEvent = (nextExposure == true) ? new ExposedEvent(user, tm + 1, nextLoc) : new ActualEvent(user, tm + 1, nextLoc);
		}

		// observed events
		ull thisObservedEventIndex = currentSampleSigma[thisMatrixIdx]; ull prevObservedEventIndex  = 0; ull nextObservedEventIndex = 0;
		if(tmIdx >= 1) { prevObservedEventIndex = currentSampleSigma[prevMatrixIdx]; }
		if(tmIdx < numTimes - 1) { nextObservedEventIndex = currentSampleSigma[nextMatrixIdx]; }

		ull thisObservedMatrixIdx = GET_INDEX(thisObservedEventIndex, tmIdx, numTimes);
		ObservedEvent* thisObservedEvent = observedMatrix[thisObservedMatrixIdx]; ObservedEvent* prevObservedEvent = NULL; ObservedEvent* nextObservedEvent = NULL;

		if(tmIdx >= 1)
		{
			ull prevObservedMatrixIdx = GET_INDEX(prevObservedEventIndex, tmIdx - 1, numTimes);
			prevObservedEvent = observedMatrix[prevObservedMatrixIdx];
		}
		if(tmIdx < numTimes - 1)
		{
			ull nextObservedMatrixIdx = GET_INDEX(nextObservedEventIndex, tmIdx + 1, numTimes);
			nextObservedEvent = observedMatrix[nextObservedMatrixIdx];
		}

		// get observed and actual events for otherUserIdx
		// here the proposal is swap between elements of sigma at thisMatrixIdx and otherThisMatrixIdx
		ull otherThisMatrixIdx = GET_INDEX(otherUserIdx, tmIdx, numTimes);

		ull otherPrevMatrixIdx = 0; ull otherNextMatrixIdx = 0;
		if(tmIdx >= 1) { otherPrevMatrixIdx = GET_INDEX(otherUserIdx, tmIdx - 1, numTimes); }
		if(tmIdx < numTimes - 1) { otherNextMatrixIdx = GET_INDEX(otherUserIdx, tmIdx + 1, numTimes); }

		ull otherThisObservedEventIndex = currentSampleSigma[otherThisMatrixIdx];
		ull otherPrevObservedEventIndex  = 0; ull otherNextObservedEventIndex = 0;
		if(tmIdx >= 1) { otherPrevObservedEventIndex = currentSampleSigma[otherPrevMatrixIdx]; }
		if(tmIdx < numTimes - 1) { otherNextObservedEventIndex = currentSampleSigma[otherNextMatrixIdx]; }

		ull otherThisLoc = currentSampleA[otherThisMatrixIdx]; ull otherPrevLoc = 0; ull otherNextLoc = 0;
		bool otherThisExposure = currentSampleX[otherThisMatrixIdx]; bool otherPrevExposure = false; bool otherNextExposure = false;

		VERIFY(otherThisLoc >= minLoc && otherThisLoc <= maxLoc);

		ActualEvent* otherThisActualEvent = (otherThisExposure == true) ? new ExposedEvent(otherUser, tm, otherThisLoc) : new ActualEvent(otherUser, tm, otherThisLoc);
		ActualEvent* otherPrevActualEvent = NULL; ActualEvent* otherNextActualEvent = NULL;

		if(tmIdx >= 1)
		{
			otherPrevLoc = currentSampleA[otherPrevMatrixIdx];
			otherPrevExposure = currentSampleX[otherPrevMatrixIdx];
			otherPrevActualEvent = (otherPrevExposure == true) ? new ExposedEvent(otherUser, tm - 1, otherPrevLoc) : new ActualEvent(otherUser, tm - 1, otherPrevLoc);
		}
		if(tmIdx < numTimes - 1)
		{
			otherNextLoc = currentSampleA[otherNextMatrixIdx];
			otherNextExposure = currentSampleX[otherNextMatrixIdx];
			otherNextActualEvent = (otherNextExposure == true) ? new ExposedEvent(otherUser, tm + 1, otherNextLoc) : new ActualEvent(otherUser, tm + 1, otherNextLoc);
		}

		// get the observed events
		ull otherThisObservedMatrixIdx = GET_INDEX(otherThisObservedEventIndex, tmIdx, numTimes);
		ObservedEvent* otherThisObservedEvent = observedMatrix[otherThisObservedMatrixIdx];
		ObservedEvent* otherPrevObservedEvent = NULL; ObservedEvent* otherNextObservedEvent = NULL;

		if(tmIdx >= 1)
		{
			ull otherPrevObservedMatrixIdx = GET_INDEX(otherPrevObservedEventIndex, tmIdx - 1, numTimes);
			otherPrevObservedEvent = observedMatrix[otherPrevObservedMatrixIdx];
		}
		if(tmIdx < numTimes - 1)
		{
			ull otherNextObservedMatrixIdx = GET_INDEX(otherNextObservedEventIndex, tmIdx + 1, numTimes);
			otherNextObservedEvent = observedMatrix[otherNextObservedMatrixIdx];
		}

		// compute probabilities:
		// 1. for the denominator part (current sample), "this" goes with "this", "other" goes with "other"
		// 2. for the numerator part (proposal), "this" is switched with "other", and "other" with "this"
		double denomPartLPPM1This = lppmPDF->PDF(context, thisActualEvent, prevActualEvent, thisObservedEvent, prevObservedEvent);
		double denomPartLPPM1Other = lppmPDF->PDF(context, otherThisActualEvent, otherPrevActualEvent, otherThisObservedEvent, otherPrevObservedEvent);

		double numeratorPartLPPM1SwitchThisOther = lppmPDF->PDF(context, thisActualEvent, prevActualEvent, otherThisObservedEvent, otherPrevObservedEvent);
		double numeratorPartLPPM1SwitchOtherThis = lppmPDF->PDF(context, otherThisActualEvent, otherPrevActualEvent, thisObservedEvent, prevObservedEvent);

		double denomPartLPPM2This = 1.0;
		double denomPartLPPM2Other = 1.0;

		double numeratorPartLPMM2SwitchThisOther = 1.0;
		double numeratorPartLPMM2SwitchOtherThis = 1.0;

		if(tmIdx < numTimes - 1)
		{
			denomPartLPPM2This = lppmPDF->PDF(context, nextActualEvent, thisActualEvent, nextObservedEvent, thisObservedEvent);
			denomPartLPPM2Other = lppmPDF->PDF(context, otherNextActualEvent, otherThisActualEvent, otherNextObservedEvent, otherThisObservedEvent);

			numeratorPartLPMM2SwitchThisOther = lppmPDF->PDF(context, nextActualEvent, thisActualEvent, otherNextObservedEvent, otherThisObservedEvent);
			numeratorPartLPMM2SwitchOtherThis = lppmPDF->PDF(context, otherNextActualEvent, otherThisActualEvent, nextObservedEvent, thisObservedEvent);
		}

		double denom = denomPartLPPM1This * denomPartLPPM1Other * denomPartLPPM2This * denomPartLPPM2Other;
		double numerator = numeratorPartLPPM1SwitchThisOther * numeratorPartLPPM1SwitchOtherThis * numeratorPartLPMM2SwitchThisOther * numeratorPartLPMM2SwitchOtherThis;

		// decide acceptance
		VERIFY(denom != 0.0); // otherwise how did we get here??
		//ratio *= numerator / denom;

		// store the log of denom and numerator in a vector in an attempt to avoid underflows
		denomLog.push_back(log(denom));
		numeratorLog.push_back(log(numerator));

		// cleanup
		thisActualEvent->Release();
		if(prevActualEvent != NULL) { prevActualEvent->Release(); }
		if(nextActualEvent != NULL) { nextActualEvent->Release(); }

		otherThisActualEvent->Release();
		if(otherPrevActualEvent != NULL) { otherPrevActualEvent->Release(); }
		if(otherNextActualEvent != NULL) { otherNextActualEvent->Release(); }

		if(timeWindow == 1) { break; }

		if(numerator == 0.0) { return false; } // it won't get accepted
	}

	ull count = denomLog.size(); VERIFY(numeratorLog.size() == count);

	double numeratorLogSum = 0.0;
	double denomLogSum = 0.0;
	for(ull i = 0; i < count; i++)
	{
		numeratorLogSum += numeratorLog[i];
		denomLogSum += denomLog[i];
	}

	double logRatio = numeratorLogSum - denomLogSum;

	bool accepted = false;
	if(log(u) <= logRatio)
	{
		accepted = true;

		// update current sample
		for(ull tm2Idx = 0; tm2Idx < numTimes; tm2Idx++)
		{
			if(timeWindow == 1) { tm2Idx = tmIdx; }
			tmIdx = tm2Idx; tm = minTime + tmIdx;

			ull thisMatrixIdx = GET_INDEX(userIdx, tmIdx, numTimes);
			ull thisObservedEventIndex = currentSampleSigma[thisMatrixIdx];

			ull otherThisMatrixIdx = GET_INDEX(otherUserIdx, tmIdx, numTimes);
			ull newThisObservedEventIndex = currentSampleSigma[otherThisMatrixIdx];

			currentSampleSigma[thisMatrixIdx] = newThisObservedEventIndex;
			currentSampleSigma[otherThisMatrixIdx] = thisObservedEventIndex;

			if(timeWindow == 1) { break; }
		}
	}

/*	if(training == false)
	{
		// log stuff
		stringstream ss("");
		ss << "ProposeSampleSigma(): step " << step << ", user = " << user << ", tm = " << tm << " ratio: " << ratio << " - u = " << u;
		ss << " -> accepted: " << accepted;
		Log::GetInstance()->Append(ss.str());
	}*/

	return accepted;

  // Bouml preserved body end 000D4191
}

bool StrongAttackOperation::GenericReconstruction(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, const ull* mostLikelyTrace, AttackOutput* output) 
{
  // Bouml preserved body begin 000C6D11

	RNG* rng = RNG::GetInstance();
	Parameters* params = Parameters::GetInstance();

	// get time parameters
	ull minTime = 0; ull maxTime = 0;
	VERIFY(params->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	VERIFY(numLoc > 1); // otherwise we'll never sample a different loc

	set<ull> usersSet = set<ull>();
	VERIFY(params->GetUsersSet(usersSet) == true);

	// get user profiles
	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	ull Nusers = profiles.size();
	VERIFY(Nusers == usersSet.size());

	vector<ull> users = vector<ull>();
	users.insert(users.begin(), usersSet.begin(), usersSet.end());
	VERIFY(users.size() == Nusers);

	map<ull, ull> usersToIdxMap = map<ull, ull>();
	ull userIdx = 0;
	foreach_const(set<ull>, usersSet, iter)
	{
		ull user = *iter;
		usersToIdxMap.insert(pair<ull, ull>(user, userIdx));
		userIdx++;
	}
	VERIFY(usersToIdxMap.size() == Nusers);

	stringstream ss("");
	ss << "Starting Generic Reconstruction: Nusers = " << Nusers << ", numTimes = " << numTimes << " numLoc = " << numLoc;
	Log::GetInstance()->Append(ss.str());

	ull distributionByteSize =  Nusers * numTimes * numLoc * sizeof(double);
	double* dist = (double*)Allocate(distributionByteSize); // locations prob. distribution matrix
	VERIFY(dist != NULL); memset(dist, 0, distributionByteSize);

	// observed events matrix (numIndex x numTimes)
	ull numIndex = Nusers;
	ull observedMatrixByteSize = numIndex * numTimes * sizeof(ObservedEvent*);
	ObservedEvent** observedMatrix = (ObservedEvent**)Allocate(observedMatrixByteSize);
	VERIFY(observedMatrix != NULL); memset(observedMatrix, 0, observedMatrixByteSize);

	map<ull, ull> pseudonymsToIdxMap = map<ull, ull>();
	set<ull> pseudonymsSet = set<ull>();

	vector<Event*> observedEvents = vector<Event*>();
	traces->GetAllEvents(observedEvents);

	// iterate over all observed events (by iterating first on every trace, and for each trace on each of its events)
	foreach_const(vector<Event*>, observedEvents, eventsIter)
	{
		ObservedEvent* observed = dynamic_cast<ObservedEvent*>(*eventsIter);
		VERIFY(observed != NULL);

		set<ull> timestamps = set<ull>();
		observed->GetTimestamps(timestamps);
		VERIFY(timestamps.size() == 1); // this attack doesn't work with time obfuscation

		ull tm = *(timestamps.begin());
		VERIFY(tm >= minTime && tm <= maxTime);

		ull index = observed->GetEventIndex();
		VERIFY(index >= 0 && index < numIndex);

		ull nym = observed->GetPseudonym();
		pseudonymsSet.insert(nym);
		if(pseudonymsToIdxMap.find(nym) == pseudonymsToIdxMap.end())
		{
			pseudonymsToIdxMap.insert(pair<ull, ull>(nym, pseudonymsToIdxMap.size()));
		}

		ull matrixIdx = GET_INDEX(index, tm - minTime, numTimes);
		VERIFY(observedMatrix[matrixIdx] == NULL); // check that we do indeed have at most one event per index, time

		observedMatrix[matrixIdx] = observed;
	}

	// check that we have exactly one event per index, time
	for(ull index = 0; index < numIndex; index++)
	{
		for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
		{
			ull matrixIdx = GET_INDEX(index, tmIdx, numTimes);
			VERIFY(observedMatrix[matrixIdx] != NULL);
		}
	}


	ull Nnyms = pseudonymsSet.size();
	ull pseudonymsByteSize =  Nusers * numTimes * Nnyms * sizeof(double);
	double* nyms = (double*)Allocate(pseudonymsByteSize); // pseudonyms prob. distribution matrix
	VERIFY(nyms != NULL); memset(nyms, 0, pseudonymsByteSize);

	ull sigmaByteSize = Nusers * numTimes * numIndex * sizeof(ull);
	double* sigma = (double*)Allocate(sigmaByteSize); // sigma (idx mapping, i.e., matrix of users, times, observed event idx count)
	VERIFY(sigma != NULL); memset(sigma, 0, sigmaByteSize);


	// Allocate the current sample. It is composed of three matrices A, X, Sigma
	ull currentSampleAByteSize = Nusers * numTimes * sizeof(ull);
	ull* currentSampleA = (ull*)Allocate(currentSampleAByteSize); // current sample matrix A (actual events, i.e., matrix of users, times -> location)
	VERIFY(currentSampleA != NULL); memset(currentSampleA, 0, currentSampleAByteSize);

	ull currentSampleXByteSize = Nusers * numTimes * sizeof(bool);
	bool* currentSampleX = (bool*)Allocate(currentSampleXByteSize); // current sample matrix X (exposure, i.e., matrix of users, times -> exposed)
	VERIFY(currentSampleX != NULL); memset(currentSampleX, 0, currentSampleXByteSize);

	ull currentSampleSigmaByteSize = Nusers * numTimes * sizeof(ull);
	ull* currentSampleSigma = (ull*)Allocate(currentSampleSigmaByteSize); // current sample matrix Sigma (idx mapping, i.e., matrix of users, times -> observed event idx)
	VERIFY(currentSampleSigma != NULL); memset(currentSampleSigma, 0, currentSampleSigmaByteSize);

	// initialize the proposal package
	GenericReconstructionProposalPackage package;
	memset(&package, 0, sizeof(GenericReconstructionProposalPackage));
	package.step = 0;
	package.currentSampleA = currentSampleA;
	package.currentSampleX = currentSampleX;
	package.currentSampleSigma = currentSampleSigma;
	package.observedMatrix = observedMatrix;
	package.users = users;
	package.user = 0;
	package.userIdx = 0;
	package.profile = NULL;
	package.tm = 0;
	package.tmIdx = 0;


	// get the initial sample
	bool persistentPseudonyms = CONTAINS_FLAG(lppmFlags, PseudonymChange) == false;
	if(persistentPseudonyms == false) // use hints
	{
		VERIFY(actualTraceHint != NULL);
		VERIFY(exposedTraceHint != NULL);
		VERIFY(sigmaHint != NULL);

		vector<Event*> actualEvents = vector<Event*>();
		actualTraceHint->GetAllEvents(actualEvents);
		VERIFY(actualEvents.size() == Nusers * numTimes);

		foreach_const(vector<Event*>, actualEvents, iter) // A
		{
			ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(*iter);
			ull user = actualEvent->GetUser();
			ull tm = actualEvent->GetTimestamp();
			ull loc = actualEvent->GetLocationstamp();

			map<ull, ull>::const_iterator userIdxIter = usersToIdxMap.find(user);
			VERIFY(userIdxIter != usersToIdxMap.end());

			ull userIdx = userIdxIter->second;
			ull tmIdx = tm - minTime;

			ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
			VERIFY(currentSampleA[idx] == 0);
			currentSampleA[idx] = loc;
		}

		vector<Event*> exposedEvents = vector<Event*>();
		exposedTraceHint->GetAllEvents(exposedEvents);
		VERIFY(exposedEvents.size() == Nusers * numTimes);

		foreach_const(vector<Event*>, actualEvents, iter) // X
		{
			ActualEvent* actualEvent = dynamic_cast<ActualEvent*>(*iter);
			ull user = actualEvent->GetUser();
			ull tm = actualEvent->GetTimestamp();
			bool exposure = actualEvent->GetType() == Exposed ? true : false;

			map<ull, ull>::const_iterator userIdxIter = usersToIdxMap.find(user);
			VERIFY(userIdxIter != usersToIdxMap.end());

			ull userIdx = userIdxIter->second;
			ull tmIdx = tm - minTime;

			ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
			currentSampleX[idx] = exposure;
		}

		memcpy(currentSampleSigma, sigmaHint, currentSampleSigmaByteSize); // sigma

		// now we have the hint as initial sample
		// do a random walk to obtain another feasible initial sample
		const ull maxWalkSteps = 100;
		ull walkSteps = 0; ull step = 0;
		while(walkSteps < maxWalkSteps)
		{
			ull userIdx = 0;

			//! update the package !//
			package.step = step;

			// go over all users, times
			foreach_const(vector<ull>, users, usersIter) // for(ull userIdx = 0; userIdx < Nusers; userIdx++)
			{
				ull user = *usersIter;
				map<ull, UserProfile*>::const_iterator iterProfile = profiles.find(user);
				VERIFY(iterProfile != profiles.end());
				UserProfile* profile = iterProfile->second;

				//! update the package !//
				package.user = user;
				package.userIdx = userIdx;
				package.profile = profile;

				double* transitionsMatrix = NULL; double* steadyStateVector = NULL;
				VERIFY(profile->GetTransitionMatrix(&transitionsMatrix) == true && transitionsMatrix != NULL);
				VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true && steadyStateVector != NULL);

				for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
				{
					ull tm = minTime + tmIdx;

// #define LOGGING_CURRENT_SAMPLE 1
#ifdef LOGGING_CURRENT_SAMPLE
					Log::GetInstance()->Append("");

					vector<string> outputLines = vector<string>();
					VERIFY(LineFormatter<ull>::GetInstance()->FormatMatrix(currentSampleA, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Training step: " << step << " - currentSampleA: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					VERIFY(LineFormatter<bool>::GetInstance()->FormatMatrix(currentSampleX, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Training step: " << step << " - currentSampleX: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					VERIFY(LineFormatter<ull>::GetInstance()->FormatMatrix(currentSampleA, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Training step: " << step << " - currentSampleSigma: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					Log::GetInstance()->Append("");
#endif

					//! update the package !//
					package.tm = tm;
					package.tmIdx = tmIdx;

					bool accepted = false;
					switch(step % 3)
					{
					case 0: // modify A
						{ accepted = ProposeSampleA(&package, true); break; }
					case 1: // modify X
						{ accepted = ProposeSampleX(&package, true); break; }
					case 2: // modify Sigma
						{ accepted = ProposeSampleSigma(&package, true); break; }
					}

					if(accepted == true) { walkSteps++; }

#ifdef LOGGING_CURRENT_SAMPLE
					// log stuff
					ss.str("");
					ss << "Training step: " << step << " user = " << user << ", tm = " << tm << " -> accepted: " << accepted;
					Log::GetInstance()->Append(ss.str());
#endif
				}

				userIdx++;
			}
			step++;
		}

		ss.str("");
		ss << "Training phase done after " << step << " steps: accepted " << walkSteps << " samples.";
		Log::GetInstance()->Append(ss.str());
	}
	else // use the result of the most likely trace attack
	{
		VERIFY(mostLikelyTrace != NULL);
		VERIFY(userToPseudonymMap.empty() == false);

		map<ull, Trace*> mapping = map<ull, Trace*>();
		traces->GetMapping(mapping);

		for(ull userIdx = 0; userIdx < Nusers; userIdx++)
		{
			ull user = users[userIdx];

			map<ull, ull>::const_iterator iter = userToPseudonymMap.find(user);
			VERIFY(iter != userToPseudonymMap.end());
			ull nym = iter->second;

			map<ull, Trace*>::const_iterator mapIter = mapping.find(nym);
			VERIFY(mapIter != mapping.end());
			Trace* trace = mapIter->second;

			vector<Event*> observedEvents = vector<Event*>();
			trace->GetEvents(observedEvents);

			ull index = 0;

			for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
			{
				ull tm = minTime + tmIdx;

				ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
				ull loc = mostLikelyTrace[idx];
				currentSampleA[idx] = loc;

				ActualEvent* actualEvent = new ActualEvent(user, tm, loc);
				ExposedEvent* exposedEvent = new ExposedEvent(*actualEvent);

				ObservedEvent* properObservedEvent = dynamic_cast<ObservedEvent*>(observedEvents[tmIdx]);
				VERIFY(tmIdx == 0 || properObservedEvent->GetEventIndex() == index);

				set<ull> tmSet = set<ull>();
				properObservedEvent->GetTimestamps(tmSet);
				VERIFY(tmSet.size() == 1 && (*(tmSet.begin())) == tm);

				index = properObservedEvent->GetEventIndex();
				properObservedEvent = NULL; // from now on use the event in the observedMatrix

				ObservedEvent* observedEvent = observedMatrix[GET_INDEX(index, tmIdx, numTimes)];
				VERIFY(observedEvent->GetEventIndex() == index);

				ull sigmaIdx = GET_INDEX(userIdx, tmIdx, numTimes);
				currentSampleSigma[sigmaIdx] = index;

				VERIFY(actualEvent != NULL && exposedEvent != NULL);

				double lppmProb0 = lppmPDF->PDF(context, actualEvent, observedEvent);
				double applicationProb0 = applicationPDF->PDF(context, actualEvent, actualEvent);

				double lppmProb1 = lppmPDF->PDF(context, exposedEvent, observedEvent);
				double applicationProb1 = applicationPDF->PDF(context, actualEvent, exposedEvent);

				double probExposure = applicationProb1 * lppmProb1;
				double probNonExposure = applicationProb0 * lppmProb0;

				VERIFY(probExposure > 0 || probNonExposure > 0);

				bool exposure = (probExposure >= probNonExposure) ? true : false;
				currentSampleX[idx] = exposure;
			}
		}

	}

	const ull initialSubSamplesBeforeCheck = 30;
	const ull subSamplesBeforeCheck = 1; // for now // 100;
	ull checkPointSamples = initialSubSamplesBeforeCheck;

	const ull maxTPTFailuresInARow = 2;
	ull tptFailuresInARow = 0;

	bool prevCompValue = 0;
	ull turningPoints = 0;

	ull subSamples = 0;
	double subSampleProb = 0.01;

	// prev. sub-sample
	ull* prevSubSampleA = (ull*)Allocate(currentSampleAByteSize); VERIFY(prevSubSampleA != NULL); memset(prevSubSampleA, 0, currentSampleAByteSize);
	bool* prevSubSampleX = (bool*)Allocate(currentSampleXByteSize); VERIFY(prevSubSampleX != NULL); memset(prevSubSampleX, 0, currentSampleXByteSize);
	ull* prevSubSampleSigma = (ull*)Allocate(currentSampleSigmaByteSize); VERIFY(prevSubSampleSigma != NULL); memset(prevSubSampleSigma, 0, currentSampleSigmaByteSize);

	const ull iidSamples = genericReconstructionSamples;
	const ull maxSeconds = 0; // for now

	ull step = 0; ull startTime = time(NULL);
	while(true)
	{
		ull currentTime = time(NULL);
		if((step >= 2 && currentTime > (startTime + maxSeconds) && maxSeconds > 0) || (subSamples >= iidSamples)) { break; }

		ull userIdx = 0;

		// update the package
		package.step = step;

		// go over all users, times
		foreach_const(vector<ull>, users, usersIter) // for(ull userIdx = 0; userIdx < Nusers; userIdx++)
		{
			ull user = *usersIter;
			map<ull, UserProfile*>::const_iterator iterProfile = profiles.find(user);
			VERIFY(iterProfile != profiles.end());
			UserProfile* profile = iterProfile->second;

			//! update the package !//
			package.user = user;
			package.userIdx = userIdx;
			package.profile = profile;

			for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
			{
				ull tm = minTime + tmIdx;

#ifdef LOGGING_CURRENT_SAMPLE
					Log::GetInstance()->Append("");

					vector<string> outputLines = vector<string>();
					VERIFY(LineFormatter<ull>::GetInstance()->FormatMatrix(currentSampleA, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Step: " << step << " - currentSampleA: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					VERIFY(LineFormatter<bool>::GetInstance()->FormatMatrix(currentSampleX, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Step: " << step << " - currentSampleX: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					VERIFY(LineFormatter<ull>::GetInstance()->FormatMatrix(currentSampleSigma, Nusers, numTimes, outputLines) == true);
					ss.str(""); ss << "Step: " << step << " - currentSampleSigma: "; Log::GetInstance()->Append(ss.str());
					foreach_const(vector<string>, outputLines, iter) { Log::GetInstance()->Append(*iter); }

					Log::GetInstance()->Append("");
#endif

				//! update the package !//
				package.tm = tm;
				package.tmIdx = tmIdx;

				for(ull i = 0; i <= 2; i++)
				{
					bool accepted = false;
					switch(i)
					{
						case 0: // modify A
						{
							if(numLoc > 1)
							{ accepted = ProposeSampleA(&package, false); }
						}
						break;

						case 1: // modify X
						{ accepted = ProposeSampleX(&package, false); }
						break;

						case 2: // modify Sigma
						{
							if(tm != maxTime) { continue; } // with persistent pseudonyms, we switch two users when proposing new sigma sample, so we only need to this for the last timestamp

							if(Nusers > 1)
							{ accepted = ProposeSampleSigma(&package, false); }
						}
						break;
						default: CODING_ERROR; break;
					}

					bool subSampled = (rng->GetUniformRandomDouble() <= subSampleProb) ? true: false;

#ifdef LOGGING_CURRENT_SAMPLE
					// log stuff
					ss.str("");
					ss << "Step: " << step << " accepted: " << accepted << ", subSampled: " << subSampled << " (subSampleProb = " << subSampleProb << ")";
					Log::GetInstance()->Append(ss.str());
#endif

					if(subSampled == true)
					{
						// Register the "new" sample
						subSamples++;

						// compare values (true if newer is greater than or equal to previous, false otherwise)
						bool compValue = true;

						if(subSamples > 1)
						{
							bool done = false;
							for(ull userIdx = 0; userIdx < Nusers; userIdx++)
							{
								for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
								{
									ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
									ull loc = currentSampleA[idx];
									ull prevLoc = prevSubSampleA[idx];
									if(loc > prevLoc) { compValue = true; done = true; break; }
									else if(loc < prevLoc) { compValue = false; done = true; break; }
								}
							}

							if(done == false)
							{
								for(ull userIdx = 0; userIdx < Nusers; userIdx++)
								{
									for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
									{
										ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
										bool exposure = currentSampleX[idx];
										bool prevExposure = prevSubSampleX[idx];
										if(exposure == true && prevExposure == false) { compValue = true; done = true; break; }
										else if(exposure == false && prevExposure == true) { compValue = false; done = true; break; }
									}
								}
							}

							if(done == false)
							{
								for(ull userIdx = 0; userIdx < Nusers; userIdx++)
								{
									for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
									{
										ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
										ull index = currentSampleSigma[idx];
										ull prevIndex = prevSubSampleSigma[idx];
										if(index > prevIndex) { compValue = true; done = true; break; }
										else if(index < prevIndex) { compValue = false; done = true; break; }
									}
								}
							}
						}

						// is there a turning point? We compare only two values but the probability (if the samples are iid) that prevCompValue != compValue is 2/3
						// note: subSamples > 1 makes sure we don't count the first sub-sample as a turning point
						ull newTP = 0;
						if(subSamples > 1 && prevCompValue != compValue) { turningPoints++;  newTP++; }

						prevCompValue = compValue;

						// copy over the prevSubSample
						for(ull userIdx = 0; userIdx < Nusers; userIdx++)
						{
							for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
							{
								ull idx = GET_INDEX(userIdx, tmIdx, numTimes);
								prevSubSampleA[idx] = currentSampleA[idx];
								prevSubSampleX[idx] = currentSampleX[idx];
								prevSubSampleSigma[idx] = currentSampleSigma[idx];
							}
						}

						bool reset = false;
						if(subSamples == checkPointSamples)
						{
							ss.str("");
							ss << "Step: " << step << ", subSampled: " << subSampled << " (subSampleProb = " << subSampleProb << ")";
							Log::GetInstance()->Append(ss.str());

							// do turning point test
							double mean = 2.0/3.0 * subSamples;
							double var = (16.0 * subSamples - 29.0)/ 90.0;
							VERIFY(var > 0);

							double halfInterval = 1.96 * sqrt(var);


							double ratio = (double)turningPoints/subSamples;
							bool iid = (abs(ratio - mean/subSamples) <= (halfInterval/subSamples));

							ss.str("");
							ss << "Turning point test: subSamples = " << subSamples << ", turningPoints = " << turningPoints << " (ratio: " << ratio;
							ss << ") | interval: " << mean << " +- " << halfInterval << " (+= " << halfInterval/subSamples << ") -> iid = " << (iid == 1 ? "yes" : "no");
							Log::GetInstance()->Append(ss.str());

							checkPointSamples += subSamplesBeforeCheck;
							if(iid == false) // our samples are not iid (at 95% level)
							{
								tptFailuresInARow++;

								ss.str("");
								ss << "Turning point test: failures in a row: " << tptFailuresInARow;

								if(tptFailuresInARow > maxTPTFailuresInARow) // discard everything, reset the test
								{
									 // we don't count this sample
									turningPoints -= newTP;
									reset = true;

									// clear output
									tptFailuresInARow = 0;
									subSamples = 0;
									checkPointSamples = initialSubSamplesBeforeCheck;
									turningPoints = 0;
									prevCompValue = false;


									VERIFY(dist != NULL); memset(dist, 0, distributionByteSize);
									VERIFY(nyms != NULL); memset(nyms, 0, pseudonymsByteSize);
									VERIFY(sigma != NULL); memset(sigma, 0, sigmaByteSize);

									ss << ", clearing all previously acquired samples";
								}
								else {	subSampleProb /= 2.0; } // decrease subSampleProb

								ss << " (new subSampleProb = " << subSampleProb << ")";
								Log::GetInstance()->Append(ss.str());
							}
							else// setup next check point
							{
								tptFailuresInARow = 0;
							}
						}


						if(reset == false) // include this sample
						{
							// count (i.e., update dist or nyms)
							for(ull userIdx = 0; userIdx < Nusers; userIdx++)
							{
								for(ull tmIdx = 0; tmIdx < numTimes; tmIdx++)
								{
									ull sampleIdx = GET_INDEX(userIdx, tmIdx, numTimes);
									ull loc = currentSampleA[sampleIdx];
									ull index = currentSampleSigma[sampleIdx];
									ObservedEvent* observedEvent = observedMatrix[GET_INDEX(index, tmIdx, numTimes)];
									ull nym = observedEvent->GetPseudonym();

									ull locIdx = loc - minLoc;
									ull distIdx = GET_INDEX_3D(userIdx, tmIdx, locIdx, numTimes, numLoc);

									map<ull, ull>::const_iterator iter = pseudonymsToIdxMap.find(nym);
									VERIFY(iter != pseudonymsToIdxMap.end());
									ull nymIdxInSet = iter->second;

									ull nymIdx = GET_INDEX_3D(userIdx, tmIdx, nymIdxInSet, numTimes, Nnyms);

									ull sigmaIdx = GET_INDEX_3D(userIdx, tmIdx, index, numTimes, numIndex);

									dist[distIdx]++;
									nyms[nymIdx]++;
									sigma[sigmaIdx]++;
								}
							}
						}

					}
				}
			}

			userIdx++;
		}
		step++;

	} // end of while

	// cleanup
	// Free(observedMatrix); don't free it, we give it the output
	output->SetObservedMatrix(observedMatrix);

	Free(currentSampleA);
	Free(currentSampleX);
	Free(currentSampleSigma);

	Free(prevSubSampleA);
	Free(prevSubSampleX);
	Free(prevSubSampleSigma);


	output->SetGenericReconstructionObjects(dist, sigma, nyms, pseudonymsSet); // set the output

	return true;

  // Bouml preserved body end 000C6D11
}


} // namespace lpm
