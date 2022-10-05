#include "SGMetric.h"

SGMetricOperation::SGMetricOperation() : MetricOperation(SGMetric)
{

}

SGMetricOperation::~SGMetricOperation()
{
}

bool SGMetricOperation::Execute(const AttackOutput* input, File* output)
{
	Log::GetInstance()->Append("Entering SGMetricOperation::Execute");

	VERIFY(input != NULL && output != NULL);

	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	ull numTimes = maxTime - minTime + 1;

	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);

	ull* mostLikelyTrace = NULL;
	VERIFY(input->GetMostLikelyTrace(&mostLikelyTrace) == true);
	VERIFY(mostLikelyTrace != NULL);

	double* mostLikelyTraceLL = NULL;
	VERIFY(input->GetMostLikelyTraceLL(&mostLikelyTraceLL) == true);
	VERIFY(mostLikelyTraceLL != NULL);

	map<ull, Trace*> mapping = map<ull, Trace*>();
	actualTraceSet->GetMapping(mapping);

	// output most likely trace
	ull userIndex = 0;
	pair_foreach_const(map<ull, Trace*>, mapping, userIter)
	{
		ull user = userIter->first;

		stringstream ss("");
		//ss << "Most likely trace for user " << user << ": ";
		ss << user << ", " << mostLikelyTraceLL[userIndex] <<  ": ";

		for(ull tm = minTime; tm <= maxTime; tm++)
		{
			ull index = GET_INDEX(userIndex, (tm - minTime), numTimes);
			ull loc = mostLikelyTrace[index];

			ss << loc;

			if(tm != maxTime) { ss << ", "; }
		}

		output->WriteLine(ss.str());

		userIndex++;
	}

	return true;
}

string SGMetricOperation::GetTypeString() const
{
	return "sg";
}
