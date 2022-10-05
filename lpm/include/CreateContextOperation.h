#ifndef LPM_CREATECONTEXTOPERATION_H
#define LPM_CREATECONTEXTOPERATION_H

//!
//! \file
//!
#include <vector>
using namespace std;
#include "Operations.h"
#include <string>
using namespace std;
#include "Context.h"
#include <map>
using namespace std;

#include "Defs.h"
#include "Private.h"

#define KC_DEFAULT_GS_ITERATIONS 10
#define KC_NO_LIMITS 0

namespace lpm { class File; } 
namespace lpm { struct TraceVector; } 
namespace lpm { class UserProfile; } 

namespace lpm {

//!
//! \brief Encompasses the input files for knowledge construction
//!
//! Struct containing the input files (i.e. a vector of learning trace files, a mobility file, and, a transition count file) 
//! for the knowledge construction procedure.
//!

struct KnowledgeInput 
{
    File* transitionsFeasibilityFile;

    File* transitionsCountFile;

    vector<File*> learningTraceFilesVector;

};
//!
//! \brief Creates a context (background knowledge)
//!
//! using a KnowledgeInput* as input, constructs a Context* object encompassing the knowledge of the adversary. 
//!
class CreateContextOperation : public Operation<KnowledgeInput, Context> 
{
  public:
    CreateContextOperation(string name = "DefaultCreateContextOperation");

    virtual ~CreateContextOperation();


  private:
    ull maxGSPerUser;

    ull maxSecondsPerUser;


  public:
    //! \brief Executes the knowledge construction
    //! 
    //! \param[in] input 	KnowledgeInput*, input object 
    //! \param[in,out] output 	Context*, output knowledge object. 
    //!
    //! \note The procedure used to construct the knowledge is Gibbs sampling. 
    //! The number of iterations can be specified by calling \a SetLimits() before this.
    //!
    //! \return true or false, depending on whether the call is successful
    virtual bool Execute(const KnowledgeInput* input, Context* output);

    virtual string GetDetailString();

    //! 
    //! \brief Sets the limits of the Gibbs sampling procedure.
    //!
    //! \param[in] maxSamples 	ull, the maximum number of samples (i.e. iterations) for each user.
    //! \param[in] maxSeconds 	ull, the maximum time (in number of seconds) to spend for each user.
    //!
    //! \note The constant KC_NO_LIMITS can be used as a unlimited value for \a maxSamples or \a maxSeconds.
    //! If both arguments are set to a value different from KC_NO_LIMITS, the iteration procedure stops when either limit is reached.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool SetLimits(ull maxSamples = KC_DEFAULT_GS_ITERATIONS, ull maxSeconds = KC_NO_LIMITS);


    bool ComputeAggregateStatistics(File* tracesFile, File* locationsFile, double** outTransitionMatrix, double** outSteadyStateVector) const;

  private:
    inline bool TransitionMatrixFromCountMatrix(const double* count, double* alpha, double* theta, double* transitionMatrix, bool sample = true) const;

    inline void GetIntermediaryTransitionVector(map<ull, double*>& cache, const double* transitionMatrix, ull loc1, ull loc3, ull tp1, ull tp2, ull tp3, double** vector) const;

    inline void ComputeSteadyStateVector(const double* transitionMatrix, double* steadyStateVector) const;

    bool DoGibbsSampling(vector<TraceVector>& learningTraces, double* priorTransitionsCount, UserProfile* profile) const;

    bool ReadKnowledgeFiles(const KnowledgeInput* input, map<ull, vector<TraceVector> >& learningTraces, map<ull, double*>& priorTransitionsCount, bool** transitionsFeasibilityMatrix = NULL);

    bool ReadTransitionsFeasibility(const File* transFeasibilityFile, bool* transFeasibilityMatrix);

    bool ReadTransitionsCount(const File* transitionsCountFile, map<ull, double*>& priorTransitionsCount);

    bool ReadLearningTraces(const vector<File*>& learningTracesFileVector, map<ull, vector<TraceVector> >& learningTraces) const;
};

} // namespace lpm
#endif
