//!
//! \file
//!
#include "../include/Verify.h"

namespace lpm {

void Verify::RuntimeVerify(bool condition, const char* conditionString, const char* file, int line)

{
  // Bouml preserved body begin 0008DC11

	if(condition == false)
	{
		stringstream message("");
		message << "[Crash at " << dec << " (" << file << ":" << line << ")" << "]: " << conditionString << " is false!";

		Log::GetInstance()->RegisterCrash(message.str());
		exit(-1);
	}

  // Bouml preserved body end 0008DC11
}


} // namespace lpm
