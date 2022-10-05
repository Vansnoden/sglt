#ifndef SGATTACKOPERATION_H_
#define SGATTACKOPERATION_H_

#include "include/Public.h"

using namespace lpm;
using namespace std;

/**
 * This is not an attack but part of the synthetic trace generation process.
 * The modified Viterbi algorithm is implemented in this class.
 */
class SGAttackOperation : public AttackOperation
{
  public:
	  SGAttackOperation(double maxMult = 1.0);

      ~SGAttackOperation();

      virtual bool CreateMetric(MetricType type, MetricOperation** metric) const;

      virtual bool Execute(const TraceSet* input, AttackOutput* output);


    private:
      bool ModifiedViterbi(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, ull* mostLikelyTrace, double* logLikelihoods);

      double maxMultFactor;
};

#endif /* SGATTACKOPERATION_H_ */
