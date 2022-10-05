
#include "../include/ContextAnalysisOperation.h"
#include "../include/MetricOperation.h"

namespace lpm {

ContextAnalysisOperation::ContextAnalysisOperation(string name, MetricDistance* distance) : Operation<Context, File>(name)
{
  // Bouml preserved body begin 000B9B11

	distanceFunction = distance;

  // Bouml preserved body end 000B9B11
}

//! 
//! \brief Gets a detailed string of the operation
//!
//! \return a string detailing the operation
//!
string ContextAnalysisOperation::GetDetailString() 
{
  // Bouml preserved body begin 000B9911

	return "ContextAnalysisOperation: " + operationName + "()";

  // Bouml preserved body end 000B9911
}


} // namespace lpm
