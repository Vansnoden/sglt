#ifndef LPM_EXAMPLEAPPLICATIONOPERATIONS_H
#define LPM_EXAMPLEAPPLICATIONOPERATIONS_H

//!
//! \file
//!
//! \brief Provides some examples of application operations by implementing the ApplicationOperation class
//!
#include "ApplicationOperation.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class Context; } 
namespace lpm { class ActualEvent; } 

namespace lpm {

//! 
//! \brief Default implementation (i.e. a simple example) of an application operation.
//! 
//! This example application operation exposes events independently with probability \a mu. 
//! If the method used (i.e. the \a type) is \a Basic, events are exposed without considering their user ID, locationstamp, or timestamp.
//! If on the other hand, the method used is \a GeneralStatistics, the probability of exposure of events depends on how popular their locationstamps are.
//!
class DefaultApplicationOperation : public ApplicationOperation 
{
  public:
    DefaultApplicationOperation(double mu, ApplicationType type = Basic);

    virtual ~DefaultApplicationOperation();

    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ActualEvent** outEvent);

    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* outEvent) const;


  private:
    double mu;

    ApplicationType appType;


  public:
    virtual string GetDetailString();

};

} // namespace lpm
#endif
