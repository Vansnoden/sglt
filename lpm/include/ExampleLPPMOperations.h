#ifndef LPM_EXAMPLELPPMOPERATIONS_H
#define LPM_EXAMPLELPPMOPERATIONS_H

//!
//! \file
//!
//! \brief Provides some implementation of examples LPPM operations by implementing the LPPMOperation class
//!
#include "LPPMOperation.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class ExposedEvent; } 
namespace lpm { class ObservedEvent; } 
namespace lpm { class Context; } 
namespace lpm { class ActualEvent; } 
namespace lpm { class Event; } 

namespace lpm {

class DefaultServiceQualityMetricDistance : public ServiceQualityMetricDistance 
{
  public:
    virtual double ComputeDistance(const ExposedEvent* exposedEvent, const ObservedEvent* observedEvent) const;

};
//! 
//! \brief Default implementation (i.e. a simple example) of a LPPM operation.
//! 
//! This example LPPM operation performs anonymization (using a random time-independent permutation) and distorts events independently using a combination of obfuscation 
//! (controlled by the parameter \a obfLevel), fake injection (controlled by the parameters \a fakeInjectionAlgorithm and \a fakeInjectionProb), and, hiding
//! (controlled by the parameter \a hidingProb).
//!
class DefaultLPPMOperation : public LPPMOperation 
{
  public:
    DefaultLPPMOperation(ushort obfuscationLevel = 0, double fakeInjectionProbability = 0.0, const FakeInjectionAlgorithm fakeInjectionAlgorithm = UniformSelection,
    					double hidingProbability = 0.0, bool anonymize = true);

    virtual ~DefaultLPPMOperation();


  private:
    void ObfuscateLocation(ull location, set<ull>& obfucatedSet) const;

    void ComputeGeneralStatistics(ull tp, double** avg) const;


  public:
    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent);

    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const;


  private:
    ushort obfuscationLevel;

    double fakeInjectionProbability;

    FakeInjectionAlgorithm fakeInjectionAlgorithm;

    double hidingProbability;


  public:
    virtual string GetDetailString();

};
class PseudonymChangeLPPMOperation : public LPPMOperation 
{
  public:
    PseudonymChangeLPPMOperation(double beta);


  private:
    double changeProb;


  public:
    virtual ~PseudonymChangeLPPMOperation();

    //!   
    //! \brief Filters (i.e. distort) the input event 
    //!  
    //! \param[in] context 	Context*, the context.  
    //! \param[in] inEvent 	ActualEvent*, the event to filter.  
    //! \param[in] outEvent	ObservedEvent*, the filtered output event.  
    //!  
    //! \return true or false, depending on whether the call is successful  
    //!
    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent);

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
    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const;

    virtual string GetDetailString();

    virtual bool Filter(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, Event** outEvent, const ObservedEvent* prevOutEvent);

    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ActualEvent* prevInEvent, const ObservedEvent* outEvent, const ObservedEvent* prevOutEvent) const;

};

} // namespace lpm
#endif
