//!
//! \file
//!
#include "../include/StoreContextOperation.h"

namespace lpm {

StoreContextOperation::StoreContextOperation(string name) : Operation<Context, File>(name)
{
  // Bouml preserved body begin 00066E91
  // Bouml preserved body end 00066E91
}

StoreContextOperation::~StoreContextOperation() 
{
  // Bouml preserved body begin 00066F11
  // Bouml preserved body end 00066F11
}

//! \brief Executes the store context operation
//! 
//! \param[in] input 	Context*, input knowledge object 
//! \param[in,out] output 	File*, output knowledge file
//!
//! \return true or false, depending on whether the call is successful
//!
bool StoreContextOperation::Execute(const Context* input, File* output) 
{
  // Bouml preserved body begin 00066F91

	if(input == NULL || output == NULL || output->IsGood() == false) { return false; }

	const Context* context = input;

	stringstream info("");
	info << "Storing the context to file...";
	Log::GetInstance()->Append(info.str());

	File accuracyOutput(output->GetFilePath() + ".accuracy", false);

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(Parameters::GetInstance()->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	TPNode* partitioning = tpInfo.partitioning;

	ull numStates = numPeriods * numLoc;

	stringstream line(""); // line.precision(8);

	// store the minLoc, maxLoc
	line << minLoc << ", " << maxLoc;
	output->WriteLine(line.str());

	output->WriteLine(""); // leave one line empty

	// now store the time partitioning
	string partitionStr = "";
	VERIFY(partitioning->GetStringRepresentation(partitionStr) == true);
	// Note: technically the partitioning may be encoded using multiples lines but we can still write it to the output like this:
	output->WriteLine(partitionStr);

	output->WriteLine(""); // leave one line empty

	// get user profiles
	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	// first, we store the user profiles
	pair_foreach_const(map<ull, UserProfile*>, profiles, usersIter)
	{
		ull user = usersIter->first;
		UserProfile* profile = usersIter->second;

		double* transitionMatrix = NULL;
		VERIFY(profile->GetTransitionMatrix(&transitionMatrix) == true);

		double* steadyStateVector = NULL;
		VERIFY(profile->GetSteadyStateVector(&steadyStateVector) == true);

		VERIFY(transitionMatrix != NULL && steadyStateVector != NULL);

		line.str(""); line << user;
		output->WriteLine(line.str());

		output->WriteLine(""); // leave one line empty

		// store transition matrix (numStates x numStates) for this user
		vector<string> lines = vector<string>();
		VERIFY(LineFormatter<double>::GetInstance()->FormatMatrix(transitionMatrix, numStates, numStates, lines) == true);
		foreach_const(vector<string>, lines, lineIter) { output->WriteLine(*lineIter); }

		output->WriteLine(""); // leave one line empty

		// store steady-state vector (numStates x 1) for this user
		string stringLine = "";
		VERIFY(LineFormatter<double>::GetInstance()->FormatVector(steadyStateVector, numStates, stringLine) == true);

		output->WriteLine(stringLine);

		output->WriteLine("");
		output->WriteLine("");


		// write down accuracy info
		ull numSamples = 0; double* accuracyMatrix = NULL;
		bool accuracyAvailable = profile->GetAccuracyInfo(&numSamples, &accuracyMatrix);

		if(accuracyAvailable == true)
		{
			line.str(""); line << user << ", " << numSamples;
			accuracyOutput.WriteLine(line.str());

			accuracyOutput.WriteLine(""); // leave one line empty

			// store accuracy matrix (numStates x numStates) for this user
			lines.clear();
			VERIFY(LineFormatter<double>::GetInstance()->FormatMatrix(accuracyMatrix, numStates, numStates, lines) == true);
			foreach_const(vector<string>, lines, lineIter) { accuracyOutput.WriteLine(*lineIter); }

			accuracyOutput.WriteLine("");
			accuracyOutput.WriteLine("");
		}

	}

	return true;

  // Bouml preserved body end 00066F91
}

string StoreContextOperation::GetDetailString() 
{
  // Bouml preserved body begin 00067011

	return "StoreContextOperation: " + operationName + "()";

  // Bouml preserved body end 00067011
}


} // namespace lpm
