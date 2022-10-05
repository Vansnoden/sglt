//!
//! \file
//!
#include "../include/MetricOperation.h"
#include "../include/TraceSet.h"

namespace lpm {

MetricOperation::MetricOperation(MetricType t) : Operation<AttackOutput, File>("MetricOperation")
{
  // Bouml preserved body begin 0004AE91

	actualTraceSet = NULL;
	type = t;

  // Bouml preserved body end 0004AE91
}

MetricOperation::~MetricOperation() 
{
  // Bouml preserved body begin 0004AF11

	if(actualTraceSet != NULL) { actualTraceSet->Release(); }

  // Bouml preserved body end 0004AF11
}

MetricType MetricOperation::GetType() const 
{
  // Bouml preserved body begin 0004AF91

	return type;

  // Bouml preserved body end 0004AF91
}

bool MetricOperation::SetActualTrace(const TraceSet* traceSet) 
{
  // Bouml preserved body begin 00055491

	if(traceSet == NULL) { return false; }

	actualTraceSet = const_cast<TraceSet*>(traceSet);
	actualTraceSet->AddRef();

	return true;

  // Bouml preserved body end 00055491
}

string MetricOperation::GetDetailString() 
{
  // Bouml preserved body begin 00055591

	return "MetricOperation: " + operationName + "()";

  // Bouml preserved body end 00055591
}


} // namespace lpm
