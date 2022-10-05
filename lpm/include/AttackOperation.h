#ifndef LPM_ATTACKOPERATION_H
#define LPM_ATTACKOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;
#include "TraceSet.h"
#include "AttackOutput.h"

#include "Defs.h"
#include "Algorithms.h"

namespace lpm { class MetricOperation; } 
namespace lpm { class FilterFunction; } 
namespace lpm { class Context; } 

namespace lpm {

//! 
//! \brief Base class for all attack operations.
//! 
//! Attack operations represent adversaries, whose goal is to reconstruct the actual traces 
//! from a set of observed traces by leveraging on the background knowledge (context).
//!
class AttackOperation : public Operation<TraceSet, AttackOutput> 
{
  public:
    AttackOperation(string name);

    virtual ~AttackOperation();

    //! 
    //! \brief Creates a MetricOperation* of the given type for the current attack
    //!
    //! \param[in] type 	MetricType, type of the MetricOperation to construct.
    //! \param[out] metric 	MetricOperation**, the output object.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool CreateMetric(MetricType type, MetricOperation** metric) const = 0;


  protected:
    FilterFunction* applicationPDF;

    FilterFunction* lppmPDF;


  public:
    //! 
    //! \brief Sets the PDFs given to the attack
    //!
    //! \param[in] app 	FilterFunction*, an object encompassing the PDF of the application operation.
    //! \param[in] lppm 	FilterFunction*, an object encompassing the PDF of the LPPM operation.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool SetPDFs(const FilterFunction* app, const FilterFunction* lppm);

    virtual string GetDetailString();

    //! 
    //! \brief Executes the attack operation
    //!
    //! \param[in] input 	TraceSet*, containing observed events.
    //! \param[in,out] output 	AttackOutput*, the resulting reconstructed output.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const TraceSet* input, AttackOutput* output) = 0;


  protected:
    Context* context;


  public:
    //! 
    //! \brief Sets the context
    //!
    //! \param[in] newContext 	Context*, the new context
    //!
    //! \return nothing
    //!
    void SetContext(const Context* newContext);

    void SetLPPMFlags(LPPMFlags flags);


  protected:
    LPPMFlags lppmFlags;

};

} // namespace lpm
#endif
