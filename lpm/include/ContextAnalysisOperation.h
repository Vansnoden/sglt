#ifndef LPM_CONTEXTANALYSISOPERATION_H
#define LPM_CONTEXTANALYSISOPERATION_H


#include "Operations.h"
#include <string>
using namespace std;
#include "Context.h"
#include "File.h"

#include "Defs.h"
#include "Private.h"


namespace lpm { class MetricDistance; } 

namespace lpm {

class ContextAnalysisOperation : public Operation<Context, File> 
{
  public:
    ContextAnalysisOperation(string name, MetricDistance* distance = NULL);

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
    virtual bool Execute(const Context* input, File* output) = 0;

    //! 
    //! \brief Gets a detailed string of the operation
    //!
    //! \return a string detailing the operation
    //!
    virtual string GetDetailString();


  protected:
    MetricDistance* distanceFunction;

};

} // namespace lpm
#endif
