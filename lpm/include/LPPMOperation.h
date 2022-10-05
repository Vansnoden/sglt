#ifndef LPM_LPPMOPERATION_H
#define LPM_LPPMOPERATION_H

//!
//! \file
//!
#include <map>
using namespace std;
#include "FilterOperation.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class ExposedEvent; } 
namespace lpm { class ObservedEvent; } 
namespace lpm { class File; } 
namespace lpm { class Context; } 
namespace lpm { class Event; } 
namespace lpm { class ActualEvent; } 
namespace lpm { class TraceSet; } 

namespace lpm {

class ServiceQualityMetricDistance 
{
  public:
    virtual double ComputeDistance(const ExposedEvent* exposedEvent, const ObservedEvent* observedEvent) const = 0;

};
//! 
//! \brief Base class for all LPPM operations.
//! 
//! LPPM operations are filter operations which are able to distort events.
//!
class LPPMOperation : public FilterOperation 
{
  protected:
    LPPMFlags flags;


  private:
    map<ull, ull> anonymization;

    ull* sigma;

    File* serviceQualityAnalysisOutput;

    ServiceQualityMetricDistance* serviceQualityAnalysisMetric;

    void ComputeAnonymizationMap(bool anonymize = true);


  public:
    LPPMOperation(string name, LPPMFlags f = Anonymization);

    virtual ~LPPMOperation();

    virtual bool Filter(const Context* context, const Event* inEvent, Event** outEvent);

    //!   
    //! \brief Filters (i.e. distort) the input event 
    //!  
    //! \param[in] context 	Context*, the context.  
    //! \param[in] inEvent 	ActualEvent*, the event to filter.  
    //! \param[in] outEvent	ObservedEvent*, the filtered output event.  
    //!  
    //! \return true or false, depending on whether the call is successful  
    //!
    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent) = 0;

    double PDF(const Context* context, const Event* inEvent, const Event* outEvent) const;

    //! 
    //! \brief The probability density function (pdf) of the filter operation
    //!
    //! Computes and returns the probability that \a outEvent is the filtered output event conditional on the input event \a inEvent and the \a context.
    //!
    //! \param[in] context 	Context*, the context.
    //! \param[in] inEvent 	ActualEvent*, the event to filter.
    //! \param[in] outEvent	ObservedEvent*, the filtered output event.
    //!
    //! \return the value of the pdf as a double
    //!
    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const = 0;

    virtual string GetDetailString();

    LPPMFlags GetFlags() const;


  protected:
    ull GetPseudonym(ull user) const;


  public:
    void GetAnonymizationMap(map<ull, ull>& map) const;

    //! 
    //! \brief Executes the filter operation
    //!
    //! \param[in] input 	TraceSet* whose events are to be filtered.
    //! \param[in,out] output 	TraceSet* the filtered output object.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const TraceSet* input, TraceSet* output);

    virtual bool Filter(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, Event** outEvent, const ObservedEvent* prevOutEvent);

    ull* GetSigma();

    double PDF(const Context* context, const Event* inEvent, const Event* prevInEvent, const Event* outEvent, const Event* prevOutEvent) const;

    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, const ObservedEvent* outEvent, const ObservedEvent* prevOutEvent) const;

    virtual void SetServiceQualityAnalysis(const File* output, const ServiceQualityMetricDistance* distance);

};

} // namespace lpm
#endif
