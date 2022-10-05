//!
//! \file
//!
#include "../include/Errors.h"

namespace lpm {

Errors::Errors() 
{
  // Bouml preserved body begin 00099D11

	lastErrorCode = 0;
	lastErrorFile = "";
	lastErrorLine = 0;

  // Bouml preserved body end 00099D11
}

//! 
//! \brief Returns the error code of the last error (the most recent one)
//!
//! \note If no error occurred since the start of the program, a value of \a 0 is returned. 
//!
//! \return ull, the error code
//!
//!
ull Errors::GetLastErrorCode() const 
{
  // Bouml preserved body begin 0002B791

	return lastErrorCode;

  // Bouml preserved body end 0002B791
}

//! 
//! \brief Sets the error code
//!
//! \note This method is not meant to be used (i.e. called) directly. Rather, users should use the SET_ERROR_CODE macro.
//!
//! \param[in] errorCode 	ull, the error code.
//! \param[in,out] file 	char*, the name of the source file whose code encountered the error.
//! \param[in,out] line int, the line number in the source file at which the error occurred (or was detected).
//!
//! \return nothing
//!
void Errors::SetErrorCode(ull errorCode, const char* file, int line, string details) 
{
  // Bouml preserved body begin 0002EB91

	lastErrorCode = errorCode;
	lastErrorFile = file;
	lastErrorLine = line;
	lastErrorDetails = details;

	Log::GetInstance()->Append(GetLastErrorMessage(), Log::errorLevel);
	Log::GetInstance()->RegisterError(GetLastErrorMessage());

  // Bouml preserved body end 0002EB91
}

//! 
//! \brief Returns a message detailing the last error (the most recent one) that occurred
//!
//! \return string, the message detailing the last error
//!
string Errors::GetLastErrorMessage() const 
{
  // Bouml preserved body begin 0002B811

	stringstream message("");
	message << "[Error Code 0x" << hex << lastErrorCode << dec << " (" << lastErrorFile << ":" << lastErrorLine << ")" << "]: ";

	switch(lastErrorCode)
	{

		case ERROR_CODE_INVALID_ARGUMENTS: message << "Invalid arguments"; break;
		case ERROR_CODE_INVALID_FORMAT: message << "Invalid format"; break;
		case ERROR_CODE_INVALID_OPERATION: message << "Invalid operation"; break;
		case ERROR_CODE_INVALID_POSITION: message << "Invalid position"; break;
		case ERROR_CODE_INVALID_SCHEDULE: message << "Invalid schedule"; break;

		case ERROR_CODE_INVALID_INPUT_FILE: message << "Invalid input file"; break;
		case ERROR_CODE_TRACE_INPUT_FILE_MISMATCH: message << "The trace and input file do not match"; break;
		case ERROR_CODE_INCOMPLETE_INPUT_FILE: message << "Incomplete input file"; break;
		case ERROR_CODE_EMPTY_TRACE: message << "Trace is empty"; break;
		case ERROR_CODE_INCOMPATIBLE_INPUT_FILES: message << "Input files are incompatible"; break;
		case ERROR_CODE_IMPOSSIBLE_TRACE: message << "The trace is impossible (i.e. it could not have been observed given the current constraints)"; break;
		case ERROR_CODE_DUPLICATE_ENTRIES: message << "The trace contain duplicate events (e.g. if this is an actual trace: more than one event for one user, timestamp pair)"; break;

		case ERROR_CODE_INVALID_TIME_PARTITIONING: message << "Invalid time partitioning"; break;
		case ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE: message << "The trace or the intended usage is not consistent with the given time partitioning"; break;

		case ERROR_CODE_SIZE_T_OVERFLOW: message << "Overflow on size_t"; break;
		case ERROR_CODE_MEMORY_ALLOCATION_FAILURE: message << "Failed to allocate memory"; break;

		case ERROR_CODE_INVALID_METHOD_CALL: message << "Invalid method call"; break;

		default: message << "Unknown error message"; break;
	}

	if(SUCCESS(lastErrorCode) == true)
	{
		message.str(""); message << "No errors occurred so far";
	}
	else
	{
		message << ": " << lastErrorDetails;
	}

	message << "!";

	return message.str();

  // Bouml preserved body end 0002B811
}


} // namespace lpm
