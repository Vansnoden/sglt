#ifndef LPM_CONTEXTANALYSISOPERATIONS_H
#define LPM_CONTEXTANALYSISOPERATIONS_H


#include <string>
using namespace std;
#include "ContextAnalysisOperation.h"

#include "Defs.h"
#include "Private.h"
#include "Metrics.h"

namespace lpm { class MetricDistance; } 
namespace lpm { class Context; } 
namespace lpm { class File; } 

namespace lpm {

class PredictabilityAnalysisOperation : public ContextAnalysisOperation 
{
  public:
    PredictabilityAnalysisOperation(string name = "PredictabilityAnalysisOperation", MetricDistance* distance = new DefaultMetricDistance(), bool normalize = false);

    //! 
    //! \brief Execute the operation
    //!
    //! Pure virtual method which executes the operation. 
    //!
    //! \tparam[[in] input 	InputType* to the input object of the operation.
    //! \tparam[[in,out] output 	OutputType* to the output object of the operation.
    //!
    //! \return true if the operation is successful, false otherwise
    //!
    virtual bool Execute(const Context* input, File* output);


  private:
    bool normalize;

};
class RandomnessAnalysisOperation : public ContextAnalysisOperation 
{
  public:
    RandomnessAnalysisOperation(string name = "RandomnessAnalysisOperation", bool normalize = true);

    //! 
    //! \brief Execute the operation
    //!
    //! Pure virtual method which executes the operation. 
    //!
    //! \tparam[[in] input 	InputType* to the input object of the operation.
    //! \tparam[[in,out] output 	OutputType* to the output object of the operation.
    //!
    //! \return true if the operation is successful, false otherwise
    //!
    virtual bool Execute(const Context* input, File* output);


  private:
    bool normalize;

};
class AbsoluteSimilarityAnalysisOperation : public ContextAnalysisOperation 
{
  public:
    AbsoluteSimilarityAnalysisOperation(string name = "AbsoluteSimilarityAnalysisOperation",
    				bool zerothOrderOnly = false, MetricDistance* distance = new DefaultMetricDistance());

    //! 
    //! \brief Execute the operation
    //!
    //! Pure virtual method which executes the operation. 
    //!
    //! \tparam[[in] input 	InputType* to the input object of the operation.
    //! \tparam[[in,out] output 	OutputType* to the output object of the operation.
    //!
    //! \return true if the operation is successful, false otherwise
    //!
    virtual bool Execute(const Context* input, File* output);

  private:
    bool zerothOrderOnly;

};
class HiddenSemanticsSimilarityAnalysisOperation : public ContextAnalysisOperation 
{
  public:
    HiddenSemanticsSimilarityAnalysisOperation(string name = "HiddenSemanticsSimilarityAnalysisOperation",
    				bool zerothOrderOnly = false, MetricDistance* distance = new DefaultMetricDistance());

    //! 
    //! \brief Execute the operation
    //!
    //! Pure virtual method which executes the operation. 
    //!
    //! \tparam[[in] input 	InputType* to the input object of the operation.
    //! \tparam[[in,out] output 	OutputType* to the output object of the operation.
    //!
    //! \return true if the operation is successful, false otherwise
    //!
    virtual bool Execute(const Context* input, File* output);


  private:
    ull maxIterations;

    ull maxSeconds;

    bool zerothOrderOnly;

  public:
    void SetLimits(ull iterations = 100, ull seconds = 30);

};

} // namespace lpm
#endif
