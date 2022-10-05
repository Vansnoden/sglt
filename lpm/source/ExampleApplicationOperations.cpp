//!
//! \file
//!
//! \brief Provides some examples of application operations by implementing the ApplicationOperation class
//!
#include "../include/ExampleApplicationOperations.h"
#include "../include/Context.h"
#include "../include/ActualEvent.h"

namespace lpm {

DefaultApplicationOperation::DefaultApplicationOperation(double mu, ApplicationType type) : ApplicationOperation("Default ApplicationOperation")
{
  // Bouml preserved body begin 0003EE11

	this->mu = mu;
	this->appType = type;

  // Bouml preserved body end 0003EE11
}

DefaultApplicationOperation::~DefaultApplicationOperation() 
{
  // Bouml preserved body begin 0003EE91
  // Bouml preserved body end 0003EE91
}

bool DefaultApplicationOperation::Filter(const Context* context, const ActualEvent* inEvent, ActualEvent** outEvent) 
{
  // Bouml preserved body begin 0003EC11

	if(context == NULL || inEvent == NULL || outEvent == NULL)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	ExposedEvent* exposedEvent = new ExposedEvent(*inEvent);

	if(RNG::GetInstance()->GetUniformRandomDouble() <= PDF(context, inEvent, exposedEvent))
	{
		*outEvent = exposedEvent; // expose
	}
	else
	{
		exposedEvent->Release();

		*outEvent = new ActualEvent(*inEvent); // do not expose
	}

	return true;

  // Bouml preserved body end 0003EC11
}

double DefaultApplicationOperation::PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* outEvent) const 
{
  // Bouml preserved body begin 00042491

	VERIFY(outEvent != NULL);

	if(inEvent->GetUser() != outEvent->GetUser() ||
		inEvent->GetTimestamp() != outEvent->GetTimestamp() ||
		inEvent->GetLocationstamp() != outEvent->GetLocationstamp())
	{
		return 0.0;
	}

	if(appType == Basic)
	{
		return (outEvent->GetType() == Exposed) ? mu : 1.0 - mu;
	}
	else if(appType == LocalSearch)
	{
		ull user = inEvent->GetUser();
		ull location = inEvent->GetLocationstamp();
		ull timestamp = inEvent->GetTimestamp();


		ull minLoc = 0; ull maxLoc = 0;
		VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);

		ull tp = Parameters::GetInstance()->LookupTimePeriod(timestamp);
		if(tp == INVALID_TIME_PERIOD)
		{
			SET_ERROR_CODE(ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE);
			return false;
		}


		UserProfile* profile = NULL;
		VERIFY(context->GetUserProfile(user, &profile) == true);
		VERIFY(profile != NULL);

		double* steadyStateVector = NULL;
		VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true);

		// get the proper sub-chain steady-state vector according to the time period of the event
		double* subChainSteadyStateVector = NULL;
		VERIFY(Algorithms::GetSteadyStateVectorOfSubChain(steadyStateVector, tp, &subChainSteadyStateVector) == true);

		double probExposure = mu * (1.0 - subChainSteadyStateVector[location - minLoc]);

		Free(subChainSteadyStateVector); // free the sub-chain steady-state vector

		return (outEvent->GetType() == Exposed) ? probExposure : 1.0 - probExposure;
	}
	else
	{
		CODING_ERROR;
		return 0.0;
	}

  // Bouml preserved body end 00042491
}

string DefaultApplicationOperation::GetDetailString() 
{
  // Bouml preserved body begin 00095E91

	string type = "";
	switch(appType)
	{
		case Basic:	type = STRINGIFY(Basic); break;
		case LocalSearch: type = STRINGIFY(LocalSearch); break;
	}

	stringstream details("");
	details << "ApplicationOperation: " << operationName << "(" << mu << ", " << type << ")";

	return details.str();

  // Bouml preserved body end 00095E91
}


} // namespace lpm
