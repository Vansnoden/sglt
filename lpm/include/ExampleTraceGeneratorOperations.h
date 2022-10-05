#ifndef LPM_EXAMPLETRACEGENERATOROPERATIONS_H
#define LPM_EXAMPLETRACEGENERATOROPERATIONS_H

//!
//! \file
//!
#include "TraceGeneratorOperation.h"

#include "Defs.h"
#include "Private.h"

namespace lpm { class File; } 
namespace lpm { class UserProfile; } 
namespace lpm { class TraceSet; } 
namespace lpm { class Context; } 

namespace lpm {

//! 
//! \brief Generates a trace by sampling from the background knowledge
//! 
//! The KnowledgeSamplingTraceGeneratorOperation generates the artificial actual trace 
//! by sampling from transition matrix (in the knowledge) of each user.
//!
class KnowledgeSamplingTraceGeneratorOperation : public TraceGeneratorOperation 
{
  public:
    KnowledgeSamplingTraceGeneratorOperation(bool piOnly = false);

    virtual ~KnowledgeSamplingTraceGeneratorOperation();

    virtual bool Execute(const TraceGeneratorInput* input, File* output);

    bool GenerateUserTrace(const UserProfile* profile, TraceSet* traces);

  protected:
    bool usePiOnly;
};
//!
//! \brief Represents the input of the knowledge sampling trace generator
//!
//! \see KnowledgeSamplingTraceGeneratorOperation
//!
class KnowledgeTraceGeneratorInput : public TraceGeneratorInput 
{
  private:
    Context* context;


  public:
    explicit KnowledgeTraceGeneratorInput(Context* context);

    virtual ~KnowledgeTraceGeneratorInput();

    Context* GetContext() const;

};

} // namespace lpm
#endif
