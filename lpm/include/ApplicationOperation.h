#ifndef LPM_APPLICATIONOPERATION_H
#define LPM_APPLICATIONOPERATION_H

//!
//! \file
//!
#include "FilterOperation.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class Context; } 
namespace lpm { class Event; } 
namespace lpm { class ActualEvent; } 

namespace lpm {

//! 
//! \brief Base class for all application operations.
//! 
//! Application operations are filter operations which are able to expose events.
//!
class ApplicationOperation : public FilterOperation 
{
  public:
    ApplicationOperation(string name);

    virtual ~ApplicationOperation();

    virtual bool Filter(const Context* context, const Event* inEvent, Event** outEvent);

    //!  
    //! \brief Filters (i.e. determine whether to expose) the input event
    //! 
    //! \param[in] context 	Context*, the context. 
    //! \param[in] inEvent 	ActualEvent*, the event to filter. 
    //! \param[in] outEvent	ActualEvent*, the filtered output event. 
    //! 
    //! \return true or false, depending on whether the call is successful 
    //!
    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ActualEvent** outEvent) = 0;

    double PDF(const Context* context, const Event* inEvent, const Event* outEvent) const;

    //! 
    //! \brief The probability density function (pdf) of the filter operation
    //!
    //! Computes and returns the probability that \a outEvent is the filtered output event conditional on the input event \a inEvent and the \a context.
    //!
    //! \param[in] context 	Context*, the context.
    //! \param[in] inEvent 	ActualEvent*, the event to filter.
    //! \param[in] outEvent	ActualEvent*, the filtered output event.
    //!
    //! \return the value of the pdf as a double
    //!
    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* outEvent) const = 0;

    virtual string GetDetailString();

    double PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const;

};

} // namespace lpm
#endif
