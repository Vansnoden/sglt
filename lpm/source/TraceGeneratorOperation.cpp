//!
//! \file
//!
#include "../include/TraceGeneratorOperation.h"

namespace lpm {

TraceGeneratorInput::TraceGeneratorInput() 
{
  // Bouml preserved body begin 00096411
  // Bouml preserved body end 00096411
}

TraceGeneratorInput::~TraceGeneratorInput() 
{
  // Bouml preserved body begin 00096491
  // Bouml preserved body end 00096491
}

TraceGeneratorOperation::TraceGeneratorOperation(string name) : Operation<TraceGeneratorInput, File>(name)
{
  // Bouml preserved body begin 00095F11
  // Bouml preserved body end 00095F11
}

TraceGeneratorOperation::~TraceGeneratorOperation() 
{
  // Bouml preserved body begin 00095F91
  // Bouml preserved body end 00095F91
}

string TraceGeneratorOperation::GetDetailString() 
{
  // Bouml preserved body begin 00096091

	return "TraceGeneratorOperation: " + operationName + "()";

  // Bouml preserved body end 00096091
}


} // namespace lpm
