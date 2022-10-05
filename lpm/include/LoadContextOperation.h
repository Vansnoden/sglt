#ifndef LPM_LOADCONTEXTOPERATION_H
#define LPM_LOADCONTEXTOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "File.h"
#include "Context.h"

#include "Defs.h"
#include "Private.h"

namespace lpm {

//!
//! \brief Loads the context (background knowledge) from file
//!
//! Constructs a Context* object from a knowledge file. 
//!
//! \note This operation is the dual of the StoreContextOperation.
//!
class LoadContextOperation : public Operation<File, Context> 
{
  public:
    LoadContextOperation(string name = "DefaultLoadContextOperation");

    virtual ~LoadContextOperation();

    //! \brief Executes the load context operation
    //! 
    //! \param[in] input 	File*, input knowledge file 
    //! \param[in,out] output 	Context*, output knowledge object
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const File* input, Context* output);

    virtual string GetDetailString();

};

} // namespace lpm
#endif
