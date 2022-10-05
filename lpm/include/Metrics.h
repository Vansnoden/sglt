#ifndef LPM_METRICS_H
#define LPM_METRICS_H

//!
//! \file
//!
#include "MetricOperation.h"
#include <string>
using namespace std;
#include <map>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class AttackOutput; } 
namespace lpm { class File; } 

namespace lpm {

//! 
//! \brief Base class for distortion based metric operations.
//! 
//! Evalutes the location-privacy of users at each timestamp using a distance function.
//!
class DistortionMetricOperation : public MetricOperation 
{
  public:
    DistortionMetricOperation(MetricType type = Distortion);

    ~DistortionMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);


  protected:
    MetricDistance* distanceFunction;


  public:
    void SetDistanceFunction(MetricDistance* function);

    virtual string GetTypeString() const;

};
class EntropyMetricOperation : public MetricOperation 
{
  public:
    EntropyMetricOperation();

    virtual ~EntropyMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;

};
class MostLikelyLocationDistortionMetricOperation : public DistortionMetricOperation 
{
  public:
    MostLikelyLocationDistortionMetricOperation();

    virtual ~MostLikelyLocationDistortionMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;

};
class AnonymityMetricOperation : public MetricOperation 
{
  public:
    AnonymityMetricOperation();

    virtual ~AnonymityMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);


  private:
    map<ull, ull> anonymizationMap;


  public:
    void SetAnonymizationMap(const map<ull,ull>& map);

    virtual string GetTypeString() const;

    void SetSigma(ull* sigma);


  private:
    ull* trueSigma;

};
//!
//! \brief Default implementation of the distance function.
//!
//! The implementation defines the distance between two different locations to be 1, and 0 otherwise (if the locations are the same).
//!
class DefaultMetricDistance : public MetricDistance 
{
  public:
    virtual double ComputeDistance(ull firstLocation, ull secondLocation) const;

};
class MostLikelyTraceDistortionMetricOperation : public DistortionMetricOperation 
{
  public:
    MostLikelyTraceDistortionMetricOperation();

    ~MostLikelyTraceDistortionMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;

};
class DensityMetricOperation : public MetricOperation 
{
  public:
    DensityMetricOperation();

    ~DensityMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;

};
class MeetingDisclosureMetricOperation : public MetricOperation 
{
  public:
    MeetingDisclosureMetricOperation();

    ~MeetingDisclosureMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;

};

} // namespace lpm
#endif
