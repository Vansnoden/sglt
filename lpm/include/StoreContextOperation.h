#ifndef LPM_STORECONTEXTOPERATION_H
#define LPM_STORECONTEXTOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "Context.h"
#include "File.h"

#include "Defs.h"
#include "Private.h"

namespace lpm {

//!
//! \brief Stores a context object (background knowledge) to file
//!
//! Write the context of a Context* object to a knowledge file. 
//! 
//! \note This operation is the dual of the LoadContextOperation.
//!
class StoreContextOperation : public Operation<Context, File> 
{
  public:
    StoreContextOperation(string name = "DefaultStoreContextOperation");

    virtual ~StoreContextOperation();

    //! \brief Executes the store context operation
    //! 
    //! \param[in] input 	Context*, input knowledge object 
    //! \param[in,out] output 	File*, output knowledge file
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const Context* input, File* output);

    virtual string GetDetailString();

};

} // namespace lpm
#endif
