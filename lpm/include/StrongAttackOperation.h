#ifndef LPM_STRONGATTACKOPERATION_H
#define LPM_STRONGATTACKOPERATION_H

//!
//! \file
//!
#include "AttackOperation.h"
#include <vector>
using namespace std;
#include <map>
using namespace std;

#include "Defs.h"
#include "Private.h"
#include "Metrics.h"


namespace lpm { class TraceSet; } 
namespace lpm { class ObservedEvent; } 
namespace lpm { class UserProfile; } 
namespace lpm { class MetricOperation; } 
namespace lpm { class AttackOutput; } 

namespace lpm {

//!
//! \note The StrongAttackOperation implements the strong adversary which uses both the steady-state vector (of the mobility of users) 
//! and the transition matrix, unlike the weak adversary which is restricted to the use of the former only.
//!
class StrongAttackOperation : public AttackOperation 
{
  private:
    TraceSet* actualTraceHint;

    TraceSet* exposedTraceHint;

    ull* sigmaHint;

    struct GenericReconstructionProposalPackage 
    {
        ull step;

        ull* currentSampleA;

        bool* currentSampleX;

        ull* currentSampleSigma;

        ObservedEvent** observedMatrix;

        vector<ull> users;

        ull user;

        ull userIdx;

        UserProfile* profile;

        ull tm;

        ull tmIdx;

    };
    
    bool viterbiInsteadOfAlphaBeta;

  public:
    StrongAttackOperation(bool genericRec = false, ull genericRecSamples = 500, bool viterbiNotAlphaBeta = false);

    ~StrongAttackOperation();

    virtual bool CreateMetric(MetricType type, MetricOperation** metric) const;

    virtual bool Execute(const TraceSet* input, AttackOutput* output);

    void SetGenericReconstructionHint(const TraceSet* actual, const TraceSet* exposed, ull* sigma);


  private:
    bool ComputeAlphaBeta(const TraceSet* traces, double** alpha, double** beta, double** lrnrm) const;

    bool ComputeMostLikelyTrace(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, ull* mostLikelyTrace, double* logLikelihoods);

    bool ProposeSampleA(GenericReconstructionProposalPackage* package, bool training = false) const;

    bool ProposeSampleX(GenericReconstructionProposalPackage* package, bool training = false) const;

    bool ProposeSampleSigma(GenericReconstructionProposalPackage* package, bool training = false) const;

    bool GenericReconstruction(const TraceSet* traces, const map<ull, ull>& userToPseudonymMap, const ull* mostLikelyTrace, AttackOutput* output);

    bool genericReconstruction;

    ull genericReconstructionSamples;

};

} // namespace lpm
#endif
