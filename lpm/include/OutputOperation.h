#ifndef LPM_OUTPUTOPERATION_H
#define LPM_OUTPUTOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "TraceSet.h"
#include "File.h"

#include "Defs.h"
#include "Private.h"

namespace lpm {

//!
//! \brief Writes a TraceSet to file
//!
//! Main class for output operations. 
//! The default implementation writes a TraceSet to a File.
//!

class OutputOperation : public Operation<TraceSet, File> 
{
  public:
    OutputOperation(string name = "DefaultOutputOperation");

    ~OutputOperation();

    //! \brief Executes the output operation 
    //! 
    //! \param[in] input 	TraceSet*, input object 
    //! \param[in,out] output 	File*, output file. 
    //!
    //! \note The implementation can format and write to file all three kinds of traces (i.e. actual, exposed, and, observed). 
    //! 
    //! \return true or false, depending on whether the call is successful
    virtual bool Execute(const TraceSet* input, File* output);

    virtual string GetDetailString();

};

} // namespace lpm
#endif
