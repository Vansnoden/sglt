#ifndef LPM_TRACEGENERATOROPERATION_H
#define LPM_TRACEGENERATOROPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "File.h"

#include "Defs.h"
#include "Private.h"

namespace lpm {

//!
//! \brief Represents the input of a trace generator (abstract base class for all trace generator)
//!
//! \see TraceGeneratorOperation
//!
//

class TraceGeneratorInput 
{
  public:
    TraceGeneratorInput();

    virtual ~TraceGeneratorInput();

};
//! 
//! \brief Base class for all trace generator operations.
//! 
//! Trace generators are used to synthesize traces according to some logic and input.
//!
class TraceGeneratorOperation : public Operation<TraceGeneratorInput, File> 
{
  public:
    explicit TraceGeneratorOperation(string name);

    virtual ~TraceGeneratorOperation();

    //! 
    //! \brief Executes the trace generator operation
    //!
    //! \param[in] input 	TraceGeneratorInput*, the generator's input.
    //! \param[in,out] output 	File*, the output file for the generated trace.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const TraceGeneratorInput* input, File* output) = 0;

    virtual string GetDetailString();

};

} // namespace lpm
#endif
