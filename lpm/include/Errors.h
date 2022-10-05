#ifndef LPM_ERRORS_H
#define LPM_ERRORS_H

//!
//! \file
//!
#include "Singleton.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Log.h"

#define NO_ERROR 0

#define MAKE_WARNING_CODE(_code) (((ull)0x1) << 62 | (_code))
#define MAKE_ERROR_CODE(_code) (((ull)0x1) << 63 | (_code))

#define SUCCESS(_code) ((((_code) >> 63) == 0) ? true : false)

#define EXTRACT_CODE(_code) ((_code) & 0xFFFFFFFF)

#define ERROR_CODE_INVALID_ARGUMENTS MAKE_ERROR_CODE(0x00000001UL)
#define ERROR_CODE_INVALID_FORMAT MAKE_ERROR_CODE(0x00000002UL)
#define ERROR_CODE_INVALID_OPERATION MAKE_ERROR_CODE(0x00000010UL)
#define ERROR_CODE_INVALID_POSITION MAKE_ERROR_CODE(0x00000020UL)
#define ERROR_CODE_INVALID_SCHEDULE MAKE_ERROR_CODE(0x00000030UL)

#define ERROR_CODE_INVALID_INPUT_FILE MAKE_ERROR_CODE(0x00000040UL)
#define ERROR_CODE_TRACE_INPUT_FILE_MISMATCH MAKE_ERROR_CODE(0x00000041UL)
#define ERROR_CODE_INCOMPLETE_INPUT_FILE MAKE_ERROR_CODE(0x00000042UL)
#define ERROR_CODE_EMPTY_TRACE MAKE_ERROR_CODE(0x00000043UL)
#define ERROR_CODE_INCOMPATIBLE_INPUT_FILES MAKE_ERROR_CODE(0x00000044UL)
#define ERROR_CODE_DUPLICATE_ENTRIES MAKE_ERROR_CODE(0x00000045UL)
#define ERROR_CODE_IMPOSSIBLE_TRACE MAKE_ERROR_CODE(0x00000046UL)

#define ERROR_CODE_INVALID_TIME_PARTITIONING MAKE_ERROR_CODE(0x00000050UL)
#define ERROR_CODE_INCONSISTENT_TIME_PARTITIONING_USAGE MAKE_ERROR_CODE(0x00000051UL)

#define ERROR_CODE_SIZE_T_OVERFLOW MAKE_ERROR_CODE(0x00000102UL)
#define ERROR_CODE_MEMORY_ALLOCATION_FAILURE MAKE_ERROR_CODE(0x00000201UL)

#define ERROR_CODE_INVALID_METHOD_CALL MAKE_ERROR_CODE(0x00000301UL)

#define SET_ERROR_CODE(_code) Errors::GetInstance()->SetErrorCode((_code), __FILE__, __LINE__)
#define SET_ERROR_CODE_DETAILS(_code, _dets) Errors::GetInstance()->SetErrorCode((_code), __FILE__, __LINE__, (_dets))

namespace lpm {

//!
//! \brief Provides the error reporting facilities
//!
//! Singleton class which provides convenient methods to report errors.
//!

class Errors : public Singleton<Errors> 
{
  public:
    Errors();


  private:
    ull lastErrorCode;

    string lastErrorFile;

    int lastErrorLine;

    string lastErrorDetails;


  public:
    //! 
    //! \brief Returns the error code of the last error (the most recent one)
    //!
    //! \note If no error occurred since the start of the program, a value of \a 0 is returned. 
    //!
    //! \return ull, the error code
    //!
    //!
    ull GetLastErrorCode() const;

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
    void SetErrorCode(ull errorCode, const char* file, int line, string details = "");

    //! 
    //! \brief Returns a message detailing the last error (the most recent one) that occurred
    //!
    //! \return string, the message detailing the last error
    //!
    string GetLastErrorMessage() const;

};

} // namespace lpm
#endif
