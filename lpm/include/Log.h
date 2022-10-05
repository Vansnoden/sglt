#ifndef LPM_LOG_H
#define LPM_LOG_H

//!
//! \file
//!
#include "Singleton.h"
#include <string>
using namespace std;

#include "Defs.h"

namespace lpm {

//!
//! \brief Provides the logging facilities
//!
//! Singleton class which provides convenient methods to log information, warnings and errors to file.
//!

class Log : public Singleton<Log> 
{
  public:
    Log();

    virtual ~Log();

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
    void Append(string message, ushort level = Log::infoLevel);


  private:
    ofstream logFile;

    ofstream crashLogFile;

    ofstream errorLogFile;

    bool enabled;


  public:
    static const ushort warningLevel;

    static const ushort errorLevel;

    static const ushort infoLevel;

    //!
    //! \brief Sets the output file name
    //!
    //! \param[in] filename 	string to be use as base file name (if not present, the '.log' extension will be used).
    //
    //! \return nothing
    void SetOutputFileName(string filename);

    //!
    //! \brief Enables/Disables the logging facilities
    //
    //! \param[in] state 	bool determining whether to enable or disable logging.
    //!
    //! \note Crash logging cannot be disabled
    //
    //! \return nothing
    void SetEnabled(bool state);

    //!
    //! \brief Registers an error
    //
    //! \param[in] message 	string detailing the error.
    //!
    //! \return nothing
    void RegisterError(string message);

    //!
    //! \brief Registers a crash
    //!
    //! \param[in] message 	string detailing the crash.
    //
    //! \note the program will terminate before it returns from the call
    //!
    //! \return nothing
    void RegisterCrash(string message);


  private:
    bool GetTimeString(string& timeString) const;

};

} // namespace lpm
#endif
