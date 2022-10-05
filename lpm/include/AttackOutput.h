#ifndef LPM_ATTACKOUTPUT_H
#define LPM_ATTACKOUTPUT_H

//!
//! \file
//!
#include <map>
using namespace std;
#include "Reference.h"

#include "Defs.h"
#include "Memory.h"

namespace lpm { class ObservedEvent; } 

namespace lpm {

//!
//! \brief Encompasses the output of the attack operation
//!
//! The attack operation outputs the probability distribution of each user at each timestamp (localization attack),
//! the most likely trace for each user (tracking attack), and, the anonymity map (de-anomyzation attack).
//!

class AttackOutput : public Reference<AttackOutput> 
{
  public:
    AttackOutput();

    ~AttackOutput();


  private:
    double* probabilityDistribution;


  public:
    bool GetProbabilityDistribution(double** output) const;

    void SetProbabilityDistribution(double* prob);


  private:
    map<ull, ull> anonymizationMap;


  public:
    void GetAnonymizationMap(map<ull, ull>& map) const;

    void SetAnonymizationMap(const map<ull, ull>& map);


  private:
    ull* mostLikelyTrace;

    double* mostLikelyTraceLL;

  public:
    bool GetMostLikelyTrace(ull** output) const;

    void SetMostLikelyTrace(const ull* trace);

    bool GetMostLikelyTraceLL(double** ll) const;

    void SetMostLikelyTraceLL(const double* ll);


  private:
    double* genericRecDistribution;

    double* genericRecPseudonyms;

    set<ull> genericRecPseudonymsSet;

    double* genericRecSigma;


  public:
    bool GetGenericReconstructionObjects(double** distribution, double** sigma, double** pseudonyms, set<ull>& pseudonymsSet) const;

    void SetGenericReconstructionObjects(double* distribution, double* sigma, double* pseudonyms, set<ull>& pseudonymsSet);


  private:
    ObservedEvent** observedMatrix;


  public:
    bool GetObservedMatrix(ObservedEvent** matrix) const;

    void SetObservedMatrix(ObservedEvent** matrix);

};

} // namespace lpm
#endif
