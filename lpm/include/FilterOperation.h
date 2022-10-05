#ifndef LPM_FILTEROPERATION_H
#define LPM_FILTEROPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Trace.h"
#include "TraceSet.h"

namespace lpm { class Context; } 
namespace lpm { class Event; } 

namespace lpm {

//!
//! \brief Represents the filter function, i.e. the pdf of a filter opreration
//!
class FilterFunction 
{
  public:
    virtual double PDF(const Context* context, const Event* inEvent, const Event* outEvent) const = 0;

    virtual double PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const = 0;

};
//!
//! \brief Represents a filter operation
//!
//! A special kind of operation which takes a TraceSet as input and produces a TraceSet as output.
//! Filter operations treat every event in the input TraceSet independently. 
//! For this purpose, it defines a pure virtual Filter() method.
//!
//

class FilterOperation : public Operation<TraceSet, TraceSet>, public FilterFunction 
{
  protected:
    Context* context;


  public:
    FilterOperation(string name);

    virtual ~FilterOperation();

    //! 
    //! \brief Executes the filter operation
    //!
    //! \param[in] input 	TraceSet* whose events are to be filtered.
    //! \param[in,out] output 	TraceSet* the filtered output object.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const TraceSet* input, TraceSet* output);

    //! 
    //! \brief Sets the context (background knowledge of the adversary)
    //!
    //! \param[in] newContext 	Context*, the new context
    //!
    //! \return nothing
    //!
    void SetContext(Context* newContext);

    //! 
    //! \brief Filters the input event
    //!
    //! \param[in] context 	Context*, the context.
    //! \param[in] inEvent 	Event*, the event to filter.
    //! \param[in] outEvent	Event*, the filtered output event.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Filter(const Context* context, const Event* inEvent, Event** outEvent) = 0;

    //! 
    //! \brief The probability density function (pdf) of the filter operation
    //!
    //! Computes and returns the probability that \a outEvent is the filtered output event conditional on the input event \a inEvent and the \a context.
    //!
    //! \param[in] context 	Context*, the context.
    //! \param[in] inEvent 	Event*, the event to filter.
    //! \param[in] outEvent	Event*, the filtered output event.
    //!
    //! \return the value of the pdf as a double
    //!
    virtual double PDF(const Context* context, const Event* inEvent, const Event* outEvent) const = 0;

    virtual string GetDetailString() = 0;

    virtual double PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const = 0;

};

} // namespace lpm
#endif
