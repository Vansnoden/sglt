#ifndef LPM_WEAKATTACKOPERATION_H
#define LPM_WEAKATTACKOPERATION_H

//!
//! \file
//!
#include "AttackOperation.h"
#include <map>
using namespace std;

#include "Defs.h"
#include "Private.h"
#include "Metrics.h"

namespace lpm { class MetricOperation; } 
namespace lpm { class TraceSet; } 
namespace lpm { class AttackOutput; } 

namespace lpm {

//!
//! \note The WeakAttackOperation implements the weak adversary who uses only
//! the steady-state vector of the mobility of users (and not the transition matrix), unlike the strong adversary.
//!
class WeakAttackOperation : public AttackOperation 
{
  public:
    WeakAttackOperation();

    virtual ~WeakAttackOperation();

    virtual bool CreateMetric(MetricType type, MetricOperation** metric) const;

    virtual bool Execute(const TraceSet* input, AttackOutput* output);


  private:
    bool ComputeLikelihood(const TraceSet* trace, double** matrix) const;

    bool ComputeLocationDistribution(const TraceSet* trace, const map<ull, ull>* mapping, double* locationDistribution) const;

};

} // namespace lpm
#endif
