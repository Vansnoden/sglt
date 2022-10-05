//!
//! \file
//!
#include "../include/ExampleTraceGeneratorOperations.h"
#include "../include/File.h"
#include "../include/UserProfile.h"
#include "../include/TraceSet.h"
#include "../include/Context.h"

namespace lpm {

KnowledgeSamplingTraceGeneratorOperation::KnowledgeSamplingTraceGeneratorOperation(bool piOnly)
		: TraceGeneratorOperation("KnowledgeSamplingTraceGeneratorOperation"), usePiOnly(piOnly)
{
  // Bouml preserved body begin 00096111
  // Bouml preserved body end 00096111
}

KnowledgeSamplingTraceGeneratorOperation::~KnowledgeSamplingTraceGeneratorOperation() 
{
  // Bouml preserved body begin 00096191
  // Bouml preserved body end 00096191
}

bool KnowledgeSamplingTraceGeneratorOperation::Execute(const TraceGeneratorInput* input, File* output) 
{
  // Bouml preserved body begin 00096211

	if(input == NULL || output == NULL || output->IsGood() == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	const KnowledgeTraceGeneratorInput* downcastedInput = dynamic_cast<const KnowledgeTraceGeneratorInput*>(input);

	if(downcastedInput == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	Context* context = downcastedInput->GetContext();

	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	TraceSet* traces = new TraceSet(ActualTrace);
	pair_foreach_const(map<ull, UserProfile*>, profiles, iter)
	{
		UserProfile* profile = iter->second;

		if(GenerateUserTrace(profile, traces) == false)
		{
			traces->Release();
			return false;
		}
	}

	OutputOperation* outputOperation = new OutputOperation();

	bool success = outputOperation->Execute(traces, output);

	outputOperation->Release();
	traces->Release();

	return success;

  // Bouml preserved body end 00096211
}

bool KnowledgeSamplingTraceGeneratorOperation::GenerateUserTrace(const UserProfile* profile, TraceSet* traces) 
{
  // Bouml preserved body begin 00096911

	VERIFY(profile != NULL && traces != NULL);

	ull user = profile->GetUser();

	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	double* steadyStateVector = NULL;
	profile->GetSteadyStateVector(&steadyStateVector);
	VERIFY(steadyStateVector != NULL);

	double* transitionMatrix = NULL;
	profile->GetTransitionMatrix(&transitionMatrix);
	VERIFY(transitionMatrix != NULL);


	// sample the trace from the markov chain
	ull prevLoc = minLoc;
	for(ull tm = minTime; tm <= maxTime; tm++)
	{
		ull nextLoc = minLoc;

		if(tm == minTime || usePiOnly == true) // use the steady state vector
		{
			ull tp = Parameters::GetInstance()->LookupTimePeriod(tm);
			if(tp == INVALID_TIME_PERIOD)
			{
				SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
				return false;
			}

			double* subChainSteadyStateVector = NULL;
			VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true);
			nextLoc = minLoc + RNG::GetInstance()->SampleIndexFromVector(subChainSteadyStateVector, numLoc);
			Free(subChainSteadyStateVector);
		}
		else // use the transition matrix
		{
			ull tp1 = Parameters::GetInstance()->LookupTimePeriod(tm - 1);
			ull tp2 = Parameters::GetInstance()->LookupTimePeriod(tm);

			if(tp1 == INVALID_TIME_PERIOD || tp2 == INVALID_TIME_PERIOD)
			{
				SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
				return false;
			}

			double* transitionVector = NULL;
			VERIFY(Algorithms::GetTransitionVectorOfSubChain(transitionMatrix, tp1, prevLoc, tp2, &transitionVector) == true);
			nextLoc = minLoc + RNG::GetInstance()->SampleIndexFromVector(transitionVector, numLoc);
			Free(transitionVector);
		}

		ActualEvent* actualEvent = new ActualEvent(user, tm, nextLoc);
		traces->AddEvent(actualEvent);
		actualEvent->Release(); // release ownership

		prevLoc = nextLoc;
	}

	return true;

  // Bouml preserved body end 00096911
}

KnowledgeTraceGeneratorInput::KnowledgeTraceGeneratorInput(Context* context) 
{
  // Bouml preserved body begin 00096311

	this->context = context;
	this->context->AddRef();

  // Bouml preserved body end 00096311
}

KnowledgeTraceGeneratorInput::~KnowledgeTraceGeneratorInput() 
{
  // Bouml preserved body begin 00096391

	this->context->Release();

  // Bouml preserved body end 00096391
}

Context* KnowledgeTraceGeneratorInput::GetContext() const 
{
  // Bouml preserved body begin 00096291

	return context;

  // Bouml preserved body end 00096291
}


} // namespace lpm
