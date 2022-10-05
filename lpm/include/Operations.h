#ifndef LPM_OPERATIONS_H
#define LPM_OPERATIONS_H

//!
//! \file
//!
#include <string>
using namespace std;
#include "Reference.h"

#include "Defs.h"

namespace lpm {

//!
//! \brief Abstract base class for all operations.
//!
//! Abstract template base class from which all operations classes derive. 
//! The two template parameters represent the input and output object types, respectively.
//!
template<typename InputType, typename OutputType>
class Operation : public Reference<Operation<InputType, OutputType> > 
{
  protected:
    string operationName;


  public:
    explicit Operation(string name);

    virtual ~Operation();

    //! 
    //! \brief Execute the operation
    //!
    //! Pure virtual method which executes the operation. 
    //!
    //! \tparam[[in] input 	InputType* to the input object of the operation.
    //! \tparam[[in,out] output 	OutputType* to the output object of the operation.
    //!
    //! \return true if the operation is successful, false otherwise
    //!
    virtual bool Execute(const InputType* input, OutputType* output) = 0;

    //! 
    //! \brief Gets a detailed string of the operation
    //!
    //! \return a string detailing the operation
    //!
    virtual string GetDetailString() = 0;

};
template<typename InputType, typename OutputType>
Operation<InputType, OutputType>::Operation(string name) 
{
  // Bouml preserved body begin 00021B11

	operationName = name;

  // Bouml preserved body end 00021B11
}

template<typename InputType, typename OutputType>
Operation<InputType, OutputType>::~Operation() 
{
  // Bouml preserved body begin 0005A591
  // Bouml preserved body end 0005A591
}


} // namespace lpm
#endif
