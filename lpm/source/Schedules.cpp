//!
//! \file
//!
#include "../include/Schedules.h"
#include "../include/Schedule.h"
#include "../include/File.h"
#include "../include/ScheduleBuilder.h"
#include "../include/MetricOperation.h"
#include "../include/ApplicationOperation.h"
#include "../include/LPPMOperation.h"

namespace lpm {

ScheduleTemplate::ScheduleTemplate() 
{
  // Bouml preserved body begin 0006D791

	builder = NULL;

  // Bouml preserved body end 0006D791
}

ScheduleTemplate::~ScheduleTemplate() 
{
  // Bouml preserved body begin 0006D811
  // Bouml preserved body end 0006D811
}

SimpleScheduleTemplate::SimpleScheduleTemplate() 
{
  // Bouml preserved body begin 00086E91

	ResetTemplate();

  // Bouml preserved body end 00086E91
}

Schedule* SimpleScheduleTemplate::BuildSchedule(const File* contextFile, string name) 
{
  // Bouml preserved body begin 0006D911

	if(contextFile == NULL || contextFile->IsGood() == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return NULL;
	}

	 VERIFY(builder == NULL);

	 builder = new ScheduleBuilder(name);

	 ApplicationOperation* application = applicationInstance;
	 if(applicationInstance == NULL) { application = new DefaultApplicationOperation(mu, appType); }

	 LPPMOperation* lppm = lppmInstance;
	 if(lppmInstance == NULL) { lppm = new DefaultLPPMOperation(obfuscationLevel, fakeInjectionProb, fakeInjectionAlgo, hidingProb); }

	 AttackOperation* attack = NULL;

	 switch(attackType)
	 {
		 case Weak: attack = new WeakAttackOperation(); break;
		 case Strong: attack = new StrongAttackOperation(genericReconstruction, genericReconstructionSamples); break;
		 default: return NULL;
	 }

	 VERIFY(builder->SetInputs(contextFile) == true);
	 VERIFY(builder->SetApplicationOperation(application) == true);
	 VERIFY(builder->SetLPPMOperation(lppm) == true);
	 VERIFY(builder->SetAttackOperation(attack) == true);
	 VERIFY(builder->SetMetricType(metricType, metricDistance) == true);

	 // release ownership so that the schedule is the only owner of these operations' instances
	 application->Release();
	 lppm->Release();
	 attack->Release();

	 Schedule* schedule = builder->GetSchedule();
	 VERIFY(schedule != NULL);

	 // Free the builder (this essentially severs the tie between the builder and the schedule)
	 delete builder;
	 builder = NULL;

	 return schedule;

  // Bouml preserved body end 0006D911
}

void SimpleScheduleTemplate::SetApplicationParameters(ApplicationType type, double mu) 
{
  // Bouml preserved body begin 00086D91

	appType = type;
	this->mu = mu;

  // Bouml preserved body end 00086D91
}

bool SimpleScheduleTemplate::SetApplicationOperation(ApplicationOperation* application) 
{
  // Bouml preserved body begin 0008DB11

	if(application == NULL) { return false; }

	applicationInstance = application;
	applicationInstance->AddRef();

	return true;

  // Bouml preserved body end 0008DB11
}

void SimpleScheduleTemplate::SetLPPMParameters(ull obfuscationLevel, FakeInjectionAlgorithm fakeInjectionAlgo, double fakeInjectionProb, double hidingProb) 
{
  // Bouml preserved body begin 00086E11

	this->obfuscationLevel = obfuscationLevel;
	this->fakeInjectionAlgo = fakeInjectionAlgo;
	this->fakeInjectionProb = fakeInjectionProb;
	this->hidingProb = hidingProb;

  // Bouml preserved body end 00086E11
}

bool SimpleScheduleTemplate::SetLPPMOperation(LPPMOperation* lppm) 
{
  // Bouml preserved body begin 0008DB91

	if(lppm == NULL) { return false; }

	lppmInstance = lppm;
	lppmInstance->AddRef();

	return true;

  // Bouml preserved body end 0008DB91
}

void SimpleScheduleTemplate::SetAttackParameters(AttackType type, bool genericRec, ull genericRecSamples) 
{
  // Bouml preserved body begin 00086F11

	attackType = type;
	genericReconstruction = genericRec;
	genericReconstructionSamples = genericRecSamples;

  // Bouml preserved body end 00086F11
}

void SimpleScheduleTemplate::SetMetricParameters(MetricType type, MetricDistance* distance) 
{
  // Bouml preserved body begin 00086F91

	metricType = type;
	metricDistance = distance;

  // Bouml preserved body end 00086F91
}

void SimpleScheduleTemplate::ResetTemplate() 
{
  // Bouml preserved body begin 0006DA11

	builder = NULL;

	// reset to the default values
	SetApplicationParameters();
	SetLPPMParameters();
	SetAttackParameters();
	SetMetricParameters();

	applicationInstance = NULL;
	lppmInstance = NULL;

	return;

  // Bouml preserved body end 0006DA11
}

SimpleLPPMComparisonScheduleTemplate::SimpleLPPMComparisonScheduleTemplate() 
{
  // Bouml preserved body begin 0009B691

	ResetTemplate();

  // Bouml preserved body end 0009B691
}

Schedule* SimpleLPPMComparisonScheduleTemplate::BuildSchedule(const File* contextFile, string name) 
{
  // Bouml preserved body begin 0006DA91

	if(contextFile == NULL || contextFile->IsGood() == false)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return NULL;
	}

	 VERIFY(builder == NULL);

	 builder = new ScheduleBuilder(name);
	 VERIFY(builder != NULL && builder->SetInputs(contextFile) == true);

	 ApplicationOperation* application = new DefaultApplicationOperation(mu, appType);
	 VERIFY(builder->SetApplicationOperation(application) == true);

	 // release ownership so that the schedule is the only owner of the operation instance
	 application->Release();

	 // branch the schedule (one branch per LPPM)
	 ull branches = lppmsVector.size();
	 VERIFY(branches >= 2);

	 VERIFY((ushort)branches == branches && builder->ForkSchedule(branches) == true);

	 for(ushort branchIdx = 0; branchIdx < branches; branchIdx++)
	 {
		 ScheduleBuilder* branchBuilder = builder->GetBranchScheduleBuilder(branchIdx);
		 VERIFY(branchBuilder != NULL);

		 LPPMOperation* lppm = lppmsVector.at(branchIdx);

		 VERIFY(branchBuilder->SetLPPMOperation(lppm) == true);

		 AttackOperation* attack = NULL;

		 switch(attackType)
		 {
			 case Weak: attack = new WeakAttackOperation(); break;
			 case Strong: attack = new StrongAttackOperation(genericReconstruction, genericReconstructionSamples); break;
			 default: return NULL;
		 }

		 VERIFY(branchBuilder->SetAttackOperation(attack) == true);
		 VERIFY(branchBuilder->SetMetricType(metricType, metricDistance) == true);

		 // release ownership so that the schedule is the only owner of the operations' instances
		 lppm->Release();
		 attack->Release();
	 }

	 Schedule* schedule = builder->GetSchedule();
	 VERIFY(schedule != NULL);

	 // Free the builder (this essentially severs the tie between the builder and the schedule)
	 delete builder;
	 builder = NULL;

	 return schedule;

  // Bouml preserved body end 0006DA91
}

void SimpleLPPMComparisonScheduleTemplate::ResetTemplate() 
{
  // Bouml preserved body begin 0006DB11

	lppmsVector.clear();
	builder = NULL;

	// reset to the default values
	SetApplicationParameters();
	SetAttackParameters();
	SetMetricParameters();

	return;

  // Bouml preserved body end 0006DB11
}

//! 
//! \brief Sets the lppms the template will use to build schedules
//!
//! \param[in] lppms 	vector<LPPMOperation*>, the lppms to use.
//!
//! \return true or false, depending on whether the call is successful
//!
bool SimpleLPPMComparisonScheduleTemplate::SetLPPMs(vector<LPPMOperation*> lppms) 
{
  // Bouml preserved body begin 0009B811

	if(lppms.size() < 2)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	foreach_const(vector<LPPMOperation*>, lppms, iter)
	{
		if(*iter == NULL)
		{
			SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
			return false;
		}
	}

	// add ref
	foreach(vector<LPPMOperation*>, lppms, iter) { (*iter)->AddRef(); }

	lppmsVector = lppms;

	return true;

  // Bouml preserved body end 0009B811
}

void SimpleLPPMComparisonScheduleTemplate::SetApplicationParameters(ApplicationType type, double mu) 
{
  // Bouml preserved body begin 0009D191

	appType = type;
	this->mu = mu;

  // Bouml preserved body end 0009D191
}

void SimpleLPPMComparisonScheduleTemplate::SetAttackParameters(AttackType type, bool genericRec, ull genericRecSamples) 
{
  // Bouml preserved body begin 0009D211

	attackType = type;
	genericReconstruction = genericRec;
	genericReconstructionSamples = genericRecSamples;

  // Bouml preserved body end 0009D211
}

void SimpleLPPMComparisonScheduleTemplate::SetMetricParameters(MetricType type, MetricDistance* distance) 
{
  // Bouml preserved body begin 0009D291

	metricType = type;
	metricDistance = distance;

  // Bouml preserved body end 0009D291
}


} // namespace lpm
