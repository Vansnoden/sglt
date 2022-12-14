//!
//! \file
//!
#include "../include/Log.h"

namespace lpm {

Log::Log() 
{
  // Bouml preserved body begin 00058B91

	enabled = false;
	SetOutputFileName("log");

  // Bouml preserved body end 00058B91
}

Log::~Log() 
{
  // Bouml preserved body begin 00058C11

	logFile.close();

  // Bouml preserved body end 00058C11
}

//! 
//! \brief Appends a message to the log
//
//! Adds a message at the end of the current log file. 
//! The seriousness level of the message (info, warning, or error) can be specified.
//
//! \param[in] message 	string to append to the log file.
//! \param[in] level 	[optional] ushort specifying the seriousness of the message.
//!	This can be one of Log::infoLevel (default), Log::warningLevel, or, Log::errorLevel.
//
//! \return nothing
//!
void Log::Append(string message, ushort level) 
{
  // Bouml preserved body begin 0002B291

	if(enabled == false) { return; }

	string timeString = "";
	GetTimeString(timeString);

	string levelMsg = ((level == warningLevel) ? "[Warning]: " : ((level == errorLevel) ? "[Error]: " : "[Info]: "));
	logFile << timeString << " - " << levelMsg << message << endl;

	logFile.flush();

  // Bouml preserved body end 0002B291
}

const ushort Log::warningLevel = 0;

const ushort Log::errorLevel = 1;

const ushort Log::infoLevel = 2;

//!
//! \brief Sets the output file name
//!
//! \param[in] filename 	string to be use as base file name (if not present, the '.log' extension will be used).
//
//! \return nothing
void Log::SetOutputFileName(string filename) 
{
  // Bouml preserved body begin 0005D891

	if(enabled == false) { return; }

	if(logFile.is_open() == true) { logFile.close(); }

	string filepath = filename + ".log";
	logFile.open(filepath.c_str(), ofstream::out);

  // Bouml preserved body end 0005D891
}

//!
//! \brief Enables/Disables the logging facilities
//
//! \param[in] state 	bool determining whether to enable or disable logging.
//!
//! \note Crash logging cannot be disabled
//
//! \return nothing
void Log::SetEnabled(bool state) 
{
  // Bouml preserved body begin 0005F211

	enabled = state;

  // Bouml preserved body end 0005F211
}

//!
//! \brief Registers an error
//
//! \param[in] message 	string detailing the error.
//!
//! \return nothing
void Log::RegisterError(string message) 
{
  // Bouml preserved body begin 00081C91

	if(errorLogFile.is_open() == false)
	{
		errorLogFile.open("error.log", ofstream::out);
	}

	string timeString = "";
	GetTimeString(timeString);

	errorLogFile << timeString << " - " << message << endl;

	errorLogFile.flush();

  // Bouml preserved body end 00081C91
}

//!
//! \brief Registers a crash
//!
//! \param[in] message 	string detailing the crash.
//
//! \note the program will terminate before it returns from the call
//!
//! \return nothing
void Log::RegisterCrash(string message) 
{
  // Bouml preserved body begin 00081D11

	if(crashLogFile.is_open() == false)
	{
		crashLogFile.open("crash.log", ofstream::out);
	}

	string timeString = "";
	GetTimeString(timeString);

	crashLogFile << timeString << " - " << message << endl;

	crashLogFile.flush();

  // Bouml preserved body end 00081D11
}

bool Log::GetTimeString(string& timeString) const 
{
  // Bouml preserved body begin 00088911

	time_t t = time(NULL);
	struct tm* tm = localtime(&t);

#define MAX_TIME_STRING_LENGTH 1024
	char tmp[MAX_TIME_STRING_LENGTH + 1];

	size_t status = strftime(tmp, MAX_TIME_STRING_LENGTH, "%c", tm);

	if(status == 0) { return false; }

	timeString = tmp;

	return true;

  // Bouml preserved body end 00088911
}


} // namespace lpm
