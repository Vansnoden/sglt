
#include "../include/ContextAnalysisOperations.h"
#include "../include/MetricOperation.h"
#include "../include/Context.h"
#include "../include/File.h"

namespace lpm {

PredictabilityAnalysisOperation::PredictabilityAnalysisOperation(string name, MetricDistance* distance, bool normalize) : ContextAnalysisOperation(name, distance)
{
  // Bouml preserved body begin 000B9B91

	this->normalize = normalize;

  // Bouml preserved body end 000B9B91
}

//! 
//! \brief Execute the operation
//!
//! Pure virtual method which executes the operation. 
//!
//! \tparam[[in] input 	InputType* to the input object of the operation.
//! \tparam[[in,out] output 	OutputType* to the output object of the operation.
//!
//! \return true if the operation is successful, false otherwise
//!
bool PredictabilityAnalysisOperation::Execute(const Context* input, File* output) 
{
  // Bouml preserved body begin 000B9A11

	if(distanceFunction == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	VERIFY(input != NULL && output != NULL && output->IsGood());

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(params->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	ull maxPeriod = tpInfo.maxPeriod;

	ull numStates = numPeriods * numLoc;

	// compute max distance
	double maxDistance = 0.0;
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
		{
			double distance = distanceFunction->ComputeDistance(loc, loc2);
			if(distance > maxDistance)
			{
				maxDistance = distance;
			}
		}
	}

	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(input->GetProfiles(profiles) == true);

	pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles)
	{
		ull user = iterProfiles->first;
		UserProfile* profile = iterProfiles->second;

		double* steadyStateVector = NULL;
		double* transitionMatrix = NULL;

		VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true);
		VERIFY(profile->GetTransitionMatrix(&transitionMatrix) == true);

		// Compute prediction error for zeroth-order (epred0) and first-order (epred1)
		double epred0 = 0.0; double epred1 = 0.0;

		// compute the adjusted stationary distribution (called \twidle{\pi})
		double* adjustedSteadyStateVector = (double*)Allocate(numPeriods * numLoc * sizeof(double));
		double* tpConditionalSteadyStateVector = (double*)Allocate(numPeriods * numLoc * sizeof(double));
		double sum = 0.0;
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			double pitp = 0.0;
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				pitp += steadyStateVector[idx];
			}

			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				double piltp = steadyStateVector[idx];
				tpConditionalSteadyStateVector[idx] = piltp / pitp;
				adjustedSteadyStateVector[idx] = piltp; //(tpInfo.propTPVector[(tp - minPeriod)] / pitp) * piltp;
				sum += adjustedSteadyStateVector[idx];
			}
		}
		VERIFY(abs(sum - 1) < EPSILON);

		// zeroth-order
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			double proptp = tpInfo.propTPVector[(tp - minPeriod)];

			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idxr = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
				{
					double distance = distanceFunction->ComputeDistance(loc, loc2);

					if(distance > 0.0)
					{
						ull idxp = GET_INDEX((tp - minPeriod), (loc2 - minLoc), numLoc);
						epred0 += proptp * tpConditionalSteadyStateVector[idxr] * tpConditionalSteadyStateVector[idxp] * distance;
					}
				}
			}
		}
		if(normalize == true) { epred0 = epred0 / maxDistance; }

		// first-order
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				double stationaryProb = adjustedSteadyStateVector[idx];

				ull state1Idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

				for(ull tp2 = minPeriod; tp2 <= maxPeriod; tp2++)
				{
					if(tpInfo.propTransMatrix[GET_INDEX(tp - minPeriod, tp2 - minPeriod, numPeriods)] == 0) { continue; } // if the time period transition is not possible (has prob. 0), skip it.

					double* transitionVector = NULL;
					VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, tp, loc, tp2, &transitionVector, false) == true);

					for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
					{
						ull state2Idx = GET_INDEX((tp2 - minPeriod), (loc2 - minLoc), numLoc);
						ull fullChainIdx = GET_INDEX(state1Idx, state2Idx, numStates);

						for(ull loc3 = minLoc; loc3 <= maxLoc; loc3++)
						{
							double distance = distanceFunction->ComputeDistance(loc2, loc3);

							if(distance > 0.0)
							{
								epred1 += stationaryProb * transitionMatrix[fullChainIdx] * transitionVector[(loc3 - minLoc)] * distance;
							}
						}
					}

					Free(transitionVector);
				}
			}
		}

		if(normalize == true) { epred1 = epred1 / maxDistance; }

		Free(tpConditionalSteadyStateVector);
		Free(adjustedSteadyStateVector);

		stringstream ss("");
		ss << user << ": ";
		ss << epred0 << DEFAULT_FIELDS_DELIMITER << " " << epred1;
		output->WriteLine(ss.str());
	}

	return true;

  // Bouml preserved body end 000B9A11
}

RandomnessAnalysisOperation::RandomnessAnalysisOperation(string name, bool normalize) : ContextAnalysisOperation(name)
{
  // Bouml preserved body begin 000B9A91

	this->normalize = normalize;

  // Bouml preserved body end 000B9A91
}

//! 
//! \brief Execute the operation
//!
//! Pure virtual method which executes the operation. 
//!
//! \tparam[[in] input 	InputType* to the input object of the operation.
//! \tparam[[in,out] output 	OutputType* to the output object of the operation.
//!
//! \return true if the operation is successful, false otherwise
//!
bool RandomnessAnalysisOperation::Execute(const Context* input, File* output) 
{
  // Bouml preserved body begin 000B9991

	VERIFY(input != NULL && output != NULL && output->IsGood());

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(params->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	ull maxPeriod = tpInfo.maxPeriod;

	ull numStates = numPeriods * numLoc;


	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(input->GetProfiles(profiles) == true);

	pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles)
	{
		ull user = iterProfiles->first;
		UserProfile* profile = iterProfiles->second;

		double* steadyStateVector = NULL;
		double* transitionMatrix = NULL;

		VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true);
		VERIFY(profile->GetTransitionMatrix(&transitionMatrix) == true);

		// Compute entropy rate for zeroth-order (er0) and first-order (er1)
		double er0 = 0.0; double er1 = 0.0;

		// compute the adjusted stationary distribution (called \twidle{\pi})
		double* adjustedSteadyStateVector = (double*)Allocate(numPeriods * numLoc * sizeof(double));
		double* tpConditionalSteadyStateVector = (double*)Allocate(numPeriods * numLoc * sizeof(double));
		double sum = 0.0;
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			double pitp = 0.0;
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				pitp += steadyStateVector[idx];
			}

			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				double piltp = steadyStateVector[idx];
				tpConditionalSteadyStateVector[idx] = piltp / pitp;
				adjustedSteadyStateVector[idx] = piltp; //(tpInfo.propTPVector[(tp - minPeriod)] / pitp) * piltp;
				sum += adjustedSteadyStateVector[idx];
			}
		}
		VERIFY(abs(sum - 1) < EPSILON);

		// zeroth-order
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				er0 -= adjustedSteadyStateVector[idx] * log2(tpConditionalSteadyStateVector[idx]);
			}
		}
		if(normalize == true) { er0 /= log2(numLoc); }// normalize

		// first-order
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				double stationaryProb = adjustedSteadyStateVector[idx];

				ull state1Idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

				for(ull tp2 = minPeriod; tp2 <= maxPeriod; tp2++)
				{
					if(tpInfo.propTransMatrix[GET_INDEX(tp - minPeriod, tp2 - minPeriod, numPeriods)] == 0) { continue; } // if the time period transition is not possible (has prob. 0), skip it.

					double* transitionVector = NULL;
					VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, tp, loc, tp2, &transitionVector, false) == true);

					for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
					{
						ull state2Idx = GET_INDEX((tp2 - minPeriod), (loc2 - minLoc), numLoc);
						ull fullChainIdx = GET_INDEX(state1Idx, state2Idx, numStates);
						er1 -= stationaryProb * transitionMatrix[fullChainIdx] * log2(transitionVector[(loc2 - minLoc)]);
					}

					Free(transitionVector);
				}
			}
		}
		if(normalize == true) { er1 /= log2(numLoc); } // normalize

		Free(tpConditionalSteadyStateVector);
		Free(adjustedSteadyStateVector);

		stringstream ss("");
		ss << user << ": ";
		ss << er0 << DEFAULT_FIELDS_DELIMITER << " " << er1;
		output->WriteLine(ss.str());
	}

	return true;

  // Bouml preserved body end 000B9991
}

AbsoluteSimilarityAnalysisOperation::AbsoluteSimilarityAnalysisOperation(string name, bool zerothOnly, MetricDistance* distance)
					: ContextAnalysisOperation(name, distance)
{
  // Bouml preserved body begin 000B9C91

	 zerothOrderOnly = zerothOnly;

  // Bouml preserved body end 000B9C91
}

//! 
//! \brief Execute the operation
//!
//! Pure virtual method which executes the operation. 
//!
//! \tparam[[in] input 	InputType* to the input object of the operation.
//! \tparam[[in,out] output 	OutputType* to the output object of the operation.
//!
//! \return true if the operation is successful, false otherwise
//!
bool AbsoluteSimilarityAnalysisOperation::Execute(const Context* input, File* output) 
{
  // Bouml preserved body begin 000B9C11

	VERIFY(input != NULL && output != NULL && output->IsGood());

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(params->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	ull maxPeriod = tpInfo.maxPeriod;

	ull numStates = numPeriods * numLoc;


	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(input->GetProfiles(profiles) == true);

	bool isDefaultDistance = (dynamic_cast<DefaultMetricDistance*>(distanceFunction) != NULL);

	pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles)
	{
		ull user1 = iterProfiles->first;
		UserProfile* profile1 = iterProfiles->second;

		double* steadyStateVector1 = NULL;
		double* transitionMatrix1 = NULL;

		VERIFY(profile1->GetSteadyStateVector(&steadyStateVector1) == true);
		VERIFY(profile1->GetTransitionMatrix(&transitionMatrix1) == true);

		// compute the adjusted stationary distribution for user1 (called \twidle{\pi})
		double* adjustedSteadyStateVector1 = (double*)Allocate(numPeriods * numLoc * sizeof(double));
		double sum1 = 0.0;
		for(ull tp = minPeriod; tp <= maxPeriod; tp++)
		{
			double pitp = 0.0;
			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				pitp += steadyStateVector1[idx];
			}

			for(ull loc = minLoc; loc <= maxLoc; loc++)
			{
				ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
				double piltp = steadyStateVector1[idx];
				adjustedSteadyStateVector1[idx] = piltp; // (tpInfo.propTPVector[(tp - minPeriod)] / pitp) * piltp;
				sum1 += adjustedSteadyStateVector1[idx];
			}
		}
		VERIFY(abs(sum1 - 1) < EPSILON);


		pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles2)
		{
			ull user2 = iterProfiles2->first;
			UserProfile* profile2 = iterProfiles2->second;

			double* steadyStateVector2 = NULL;
			double* transitionMatrix2 = NULL;

			VERIFY(profile2->GetSteadyStateVector(&steadyStateVector2) == true);
			VERIFY(profile2->GetTransitionMatrix(&transitionMatrix2) == true);

			double sim0 = 0.0; double sim1 = 0.0;

			// compute the adjusted stationary distribution for user2 (called \twidle{\pi})
			double* adjustedSteadyStateVector2 = (double*)Allocate(numPeriods * numLoc * sizeof(double));
			double sum2 = 0.0;
			for(ull tp = minPeriod; tp <= maxPeriod; tp++)
			{
				double pitp = 0.0;
				for(ull loc = minLoc; loc <= maxLoc; loc++)
				{
					ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
					pitp += steadyStateVector2[idx];
				}

				for(ull loc = minLoc; loc <= maxLoc; loc++)
				{
					ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
					double piltp = steadyStateVector2[idx];
					adjustedSteadyStateVector2[idx] = piltp; // (tpInfo.propTPVector[(tp - minPeriod)] / pitp) * piltp;
					sum2 += adjustedSteadyStateVector2[idx];
				}
			}
			VERIFY(abs(sum2 - 1) < EPSILON);


			if(isDefaultDistance == true) // for now, we implement the metric only for the default distance
			{
				// zeroth-order
				for(ull tp = minPeriod; tp <= maxPeriod; tp++)
				{
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

						sim0 += min(adjustedSteadyStateVector1[idx], adjustedSteadyStateVector2[idx]);
					}
				}

				if(zerothOrderOnly == false)
				{
					// first-order
					for(ull tp = minPeriod; tp <= maxPeriod; tp++)
					{
						for(ull loc = minLoc; loc <= maxLoc; loc++)
						{
							ull idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
							double stationaryProb = adjustedSteadyStateVector1[idx]; // prob. of user1 (leader) being there

							ull state1Idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

							for(ull tp2 = minPeriod; tp2 <= maxPeriod; tp2++)
							{
								if(tpInfo.propTransMatrix[GET_INDEX(tp - minPeriod, tp2 - minPeriod, numPeriods)] == 0) { continue; } // if the time period transition is not possible (has prob. 0), skip it.

								double check1 = 0.0; double check2 = 0.0;
								for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
								{
									ull state2Idx = GET_INDEX((tp2 - minPeriod), (loc2 - minLoc), numLoc);
									ull idx2 = GET_INDEX(state1Idx, state2Idx, numStates);
									sim1 += stationaryProb * min(transitionMatrix1[idx2], transitionMatrix2[idx2]);

									check1 += transitionMatrix1[idx2];
									check2 += transitionMatrix2[idx2];
								}
								// VERIFY(abs(check1 - check2) < 0.1); // check assumption
							}
						}
					}
				}
			}
			else
			{
				CODING_ERROR;
			}

			Free(adjustedSteadyStateVector2);

			stringstream ss("");
			ss << user1 << DEFAULT_FIELDS_DELIMITER << " " << user2 << ": "; // leader, follower
			if(zerothOrderOnly == true)	{ ss << sim0; }
			else { ss << sim0 << DEFAULT_FIELDS_DELIMITER << " " << sim1; }
			output->WriteLine(ss.str());
		}

		Free(adjustedSteadyStateVector1);
	}

	return true;

  // Bouml preserved body end 000B9C11
}

HiddenSemanticsSimilarityAnalysisOperation::HiddenSemanticsSimilarityAnalysisOperation(string name, bool zerothOnly, MetricDistance* distance)
			: ContextAnalysisOperation(name, distance)
{
  // Bouml preserved body begin 000C1D91

	SetLimits(); // default values

	zerothOrderOnly = zerothOnly;

  // Bouml preserved body end 000C1D91
}

//! 
//! \brief Execute the operation
//!
//! Pure virtual method which executes the operation. 
//!
//! \tparam[[in] input 	InputType* to the input object of the operation.
//! \tparam[[in,out] output 	OutputType* to the output object of the operation.
//!
//! \return true if the operation is successful, false otherwise
//!
bool HiddenSemanticsSimilarityAnalysisOperation::Execute(const Context* input, File* output) 
{
  // Bouml preserved body begin 000C1D11

	VERIFY(input != NULL && output != NULL && output->IsGood());

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(params->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	ull maxPeriod = tpInfo.maxPeriod;

	ull numStates = numPeriods * numLoc;

	RNG* rng = RNG::GetInstance();


	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(input->GetProfiles(profiles) == true);

	bool isDefaultDistance = (dynamic_cast<DefaultMetricDistance*>(distanceFunction) != NULL);

	pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles)
	{
		ull user1 = iterProfiles->first;
		UserProfile* profile1 = iterProfiles->second;

		double* steadyStateVector1 = NULL;
		double* transitionMatrix1 = NULL;

		VERIFY(profile1->GetSteadyStateVector(&steadyStateVector1) == true);
		VERIFY(profile1->GetTransitionMatrix(&transitionMatrix1) == true);


		pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles2)
		{
			ull user2 = iterProfiles2->first;
			UserProfile* profile2 = iterProfiles2->second;

			double* steadyStateVector2 = NULL;
			double* transitionMatrix2 = NULL;

			VERIFY(profile2->GetSteadyStateVector(&steadyStateVector2) == true);
			VERIFY(profile2->GetTransitionMatrix(&transitionMatrix2) == true);

			double sim0 = 0.0; double sim1 = 0.0;

			if(isDefaultDistance == true) // for now, we implement the metric only for the default distance
			{

//#define LOG_SEM_SIM 1
#ifdef LOG_SEM_SIM
				{
					Log::GetInstance()->Append("#########################################\n");
					stringstream info("");
					info << "Computation for the pair: " <<  user1 << ", " << user2;
					Log::GetInstance()->Append(info.str());
				}
#endif

				if(user1 == user2) { sim0 = 1.0; sim1 = 1.0; }
				else
				{
					ull sigmasArrayByteSize = numPeriods * sizeof(ll*);
					ll** sigmasArray = (ll**)Allocate(sigmasArrayByteSize);
					VERIFY(sigmasArray != NULL); memset(sigmasArray, 0, sigmasArrayByteSize);

					// zeroth-order
					sim0 = 0.0;
					for(ull tp = minPeriod; tp <= maxPeriod; tp++)
					{
						// Compute the best sigma
						ull sigmaByteSize = numLoc * sizeof(ll);
						ll* sigma = (ll*)Allocate(sigmaByteSize);
						VERIFY(sigma != NULL); memset(sigma, 0, sigmaByteSize);

						ull wdByteSize = numLoc * numLoc * sizeof(double);
						double* wd = (double*)Allocate(wdByteSize);
						VERIFY(wd != NULL); memset(wd, 0, wdByteSize);

						// fill in the weight matrix
						double maxVal = 0.0; double maxOverlapProb = 0.0;
						for(ull loc = minLoc; loc <= maxLoc; loc++)
						{
							//stringstream info(""); info.precision(5);

							ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
							for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
							{
								ull idx2 = GET_INDEX((tp - minPeriod), (loc2 - minLoc), numLoc);

								ull wIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);
								double w = min(steadyStateVector1[idx1], steadyStateVector2[idx2]);

								if(w > maxOverlapProb) { maxOverlapProb = w; }

								//info << w << (loc2 < maxLoc ? ", " : "");

								w = log2(w);
								wd[wIdx] = w;

								double mw = -w;
								if(mw > maxVal) { maxVal = mw; } // keep track of max
							}

							//Log::GetInstance()->Append(info.str());
						}

						ull costMatrixByteSize = numLoc * numLoc * sizeof(ll);
						ll* costMatrix = (ll*)Allocate(costMatrixByteSize);
						VERIFY(costMatrix != NULL); memset(costMatrix, 0, costMatrixByteSize);

						//Log::GetInstance()->Append("***********************");

						// make the probability an integer: multiply by 1/minVal
						const ll bigNumber = (maxVal == 0.0) ? -1 : (ll)(-((double)(1000000/2.0)/maxVal) + 10);
						for(ull loc = minLoc; loc <= maxLoc; loc++)
						{
							//stringstream info(""); info.precision(5);

							for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
							{
								ull wIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);

								// Note: we are transforming the maximization problem (maximum weight assignment) in a minimization problem (minimum cost assignment)
								ll c = wd[wIdx] > 0 ? 0 : (ll)(wd[wIdx]*bigNumber);
								costMatrix[wIdx] = c;

								//info << c << (loc2 < maxLoc ? ", " : "");
							}

							//Log::GetInstance()->Append(info.str());
						}
						Free(wd); wd = NULL; // free wd
						//Log::GetInstance()->Append("------------");

						Algorithms::MinimumCostAssignment(costMatrix, numLoc, sigma); // Hungarian (munkres) algorithm to find sigma
						for(ull loc = minLoc; loc <= maxLoc; loc++)
						{
							stringstream info("");
							VERIFY(sigma[loc - minLoc] >= 0 && sigma[loc - minLoc] < (ll)numLoc);
#ifdef LOG_SEM_SIM
							info << "Assignment: loc u1 -> loc u2: " << loc << " -> " << sigma[loc - minLoc] + minLoc;
							Log::GetInstance()->Append(info.str());
#endif
						}


						Free(costMatrix); costMatrix = NULL; // free costMatrix

						// finally compute the similarity value according to sigma
						double tmpSim0 = 0.0;
						for(ull loc = minLoc; loc <= maxLoc; loc++)
						{
							ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
							ull idx2 = GET_INDEX((tp - minPeriod), sigma[(loc - minLoc)], numLoc);

							sim0 += min(steadyStateVector1[idx1], steadyStateVector2[idx2]);
							tmpSim0 += sim0;
						}

						VERIFY(maxOverlapProb <= tmpSim0);
						sigmasArray[tp - minPeriod] = sigma; // save sigma;
					}

					if(zerothOrderOnly == false)
					{
						// first-order
						sim1 = 0.0;
						for(ull tp = minPeriod; tp <= maxPeriod; tp++)
						{
							// get the proper sigma (starting sample for M-H)
							ll* sigma = sigmasArray[tp - minPeriod];

							for(ull tp2 = minPeriod; tp2 <= maxPeriod; tp2++)
							{
								if(tpInfo.propTransMatrix[GET_INDEX(tp - minPeriod, tp2 - minPeriod, numPeriods)] == 0) { continue; } // if the time period transition is not possible (has prob. 0), skip it.

								ull sigmaByteSize = numLoc * sizeof(ll);
								ll* newSigma = (ll*)Allocate(sigmaByteSize);
								VERIFY(newSigma != NULL);
								memcpy(newSigma, sigma, sigmaByteSize); // copy sigma

								for(ull loc = minLoc; loc <= maxLoc; loc++)	{ newSigma[loc - minLoc] += minLoc; } // to be consistent

								double bestScore = 0.0;
								double oldScore = 0.0;

								ull step = 1; ull startTime = time(NULL);
								while(true)
								{
									if((maxIterations != 0 && step >= maxIterations) || (maxSeconds != 0 && (time(NULL) - startTime) >= maxSeconds)) { break; }

									ull secondPos = 0; ull firstPos = 0;
									if(step > 1) // at step 1 we don't propose sample, we just compute the similarity score
									{
										if(oldScore > bestScore) { bestScore = oldScore; }

										// propose new sample
										firstPos = rng->GetUniformRandomULLBetween(0, numLoc - 1);
										do
										{
											secondPos = rng->GetUniformRandomULLBetween(0, numLoc - 1);
										}
										while(firstPos == secondPos);
										VERIFY(firstPos != secondPos);

										ll tmp = newSigma[firstPos];
										newSigma[firstPos] = newSigma[secondPos];
										newSigma[secondPos] = tmp;
									}

									// compute similarity score
									double newScore = 0.0;
									for(ull loc = minLoc; loc <= maxLoc; loc++)
									{
										ull state1Idx = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

										ull idxSS = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
										double stationaryProb = steadyStateVector1[idxSS]; // prob. of user1 (leader) being there

										double transitionToTp2Prob = 0.0;
										for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
										{
											ull state2Idx = GET_INDEX((tp2 - minPeriod), (loc2 - minLoc), numLoc);
											ull idxMat = GET_INDEX(state1Idx, state2Idx, numStates);
											transitionToTp2Prob += transitionMatrix1[idxMat];
										}

										double* subTransVector1 = NULL;
										VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix1, tp, loc, tp2, &subTransVector1) == true);

										ull semanticLoc = newSigma[loc - minLoc];

										double* subTransVector2 = NULL;
										VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix2, tp, semanticLoc, tp2, &subTransVector2) == true);

										for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
										{
											ull semanticLoc2 = newSigma[loc2 - minLoc];
											newScore += stationaryProb * transitionToTp2Prob *
														min(subTransVector1[loc2 - minLoc], subTransVector2[semanticLoc2 - minLoc]);
										}

										Free(subTransVector1);
										Free(subTransVector2);
									}

									// decide on sample
									bool accept = step == 1 ? true : (rng->GetUniformRandomDouble() < newScore / oldScore);

									if(accept == false) // reject -> keep previous sample
									{
										ll tmp = newSigma[firstPos];
										newSigma[firstPos] = newSigma[secondPos];
										newSigma[secondPos] = tmp;
									}
									else { oldScore = newScore; }

									step++;
								}
								Free(newSigma); newSigma = NULL;

								sim1 += bestScore;
							}
						}
					}
					for(ull tpIdx = 0; tpIdx < numPeriods; tpIdx++)	{ Free(sigmasArray[tpIdx]); } Free(sigmasArray); // cleanup
				}
			}
			else
			{
				CODING_ERROR;
			}

			stringstream ss("");
			ss << user1 << DEFAULT_FIELDS_DELIMITER << " " << user2 << ": "; // leader, follower
			if(zerothOrderOnly == true)	{ ss << sim0; }
			else { ss << sim0 << DEFAULT_FIELDS_DELIMITER << " " << sim1; }
			output->WriteLine(ss.str());
		}
	}

	return true;

  // Bouml preserved body end 000C1D11
}

void HiddenSemanticsSimilarityAnalysisOperation::SetLimits(ull iterations, ull seconds) 
{
  // Bouml preserved body begin 000C3711

	maxIterations = iterations;
	maxSeconds = seconds;

  // Bouml preserved body end 000C3711
}


} // namespace lpm
