#ifndef LPM_METRICOPERATION_H
#define LPM_METRICOPERATION_H

//!
//! \file
//!
#include "Operations.h"
#include "AttackOutput.h"
#include "File.h"
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class TraceSet; } 

namespace lpm {

//! 
//! \brief Base class for all metric operations.
//! 
//! Metric operations evaluate the performance of the attack.
//!
class MetricOperation : public Operation<AttackOutput, File> 
{
  private:
    MetricType type;


  public:
    explicit MetricOperation(MetricType t);

    virtual ~MetricOperation();

    MetricType GetType() const;

    //! 
    //! \brief Executes the metric operation
    //!
    //! \param[in] input 	AttackOutput*, the result of the attack operation.
    //! \param[in,out] output 	File*, the output file.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    virtual bool Execute(const AttackOutput* input, File* output) = 0;


  protected:
    TraceSet* actualTraceSet;


  public:
    bool SetActualTrace(const TraceSet* traceSet);

    virtual string GetDetailString();

    virtual string GetTypeString() const = 0;

};
//! 
//! \brief Base class for all metric distance functions (used by distortion-based metrics).
//! 
//! Metric distances implement a single function which computes the distance between two locations.
//!
class MetricDistance 
{
  public:

	virtual ~MetricDistance() {}
    //! 
    //! \brief Computes the distance between two locations
    //!
    //! \param[in] firstLocation 	ull, the first location.
    //! \param[in] secondLocation 	ull, the second location.
    //!
    //! \return double, representing the distance between \a firstLocation and \a secondLocation.
    //!
    virtual double ComputeDistance(ull firstLocation, ull secondLocation) const = 0;

};

} // namespace lpm
#endif
