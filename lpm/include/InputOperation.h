#ifndef LPM_INPUTOPERATION_H
#define LPM_INPUTOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "File.h"
#include "TraceSet.h"

#include "Defs.h"
#include "Private.h"

namespace lpm { class Event; } 

namespace lpm {

//!
//! \brief Encompasses the characteristics (parameters) of the input trace
//!

struct InputInfo 
{
    ull minUserID;

    ull maxUserID;

    ull minTimestamp;

    ull maxTimestamp;

    ull minLocationstamp;

    ull maxLocationstamp;

};
//!
//! \brief Reads a TraceSet from file
//!
//! Main class for input operations. 
//! The default implementation reads a TraceSet from a File.
//!

class InputOperation : public Operation<File, TraceSet> 
{
  public:
    InputOperation(string name = "DefaultInputOperation");

    ~InputOperation();

    //! \brief Executes the input operation
    //!
    //! \param[in] input 	File*, input file.
    //! \param[in,out] output 	TraceSet* output object.
    //!
    //! \note The implementation can read all three kinds of traces (i.e. actual, exposed, and, observed). 
    //!
    //! \return true or false, depending on whether the call is successful
    virtual bool Execute(const File* input, TraceSet* output);

    virtual string GetDetailString();


  private:
    bool ParseEvent(string line, Event** event, TraceType type) const;

    void UpdateInputInfo(const Event* event);

    bool ShouldBeIncluded(const Event* event) const;

    InputInfo inputInfo;

};

} // namespace lpm
#endif
