#ifndef LPM_LPM_H
#define LPM_LPM_H

//!
//! \file
//!
#include "Singleton.h"
#include "Schedule.h"
#include <map>
using namespace std;
#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"
#include "Metrics.h"
#include "ExampleTraceGeneratorOperations.h"

namespace lpm { class ContextFactory; } 
namespace lpm { class Context; } 
namespace lpm { class TraceSet; } 
namespace lpm { class FilterFunction; } 
namespace lpm { class AttackOperation; } 
namespace lpm { class AttackOutput; } 
namespace lpm { class Schedule; } 
namespace lpm { class File; } 
namespace lpm { struct KnowledgeInput; } 
namespace lpm { class ContextAnalysisSchedule; } 
namespace lpm { class TraceGeneratorOperation; } 
namespace lpm { class TraceGeneratorInput; } 

namespace lpm {

//! 
//! \brief Main class of the library
//! 
//! The LPM singleton class can be thought of as the engine of the library. As such, it is responsible for running arbitrary schedules
//! and performing procedures such as the knowledge construction.
//!
class LPM : public Singleton<LPM> 
{
  private:
    ContextFactory* contextFactory;


  public:
    struct State 
    {
        SchedulePosition currentPosition;

        void* input;

        Context* context;

        TraceSet* actualTraceSet;

        char* outputPrefix;

        FilterFunction* appPDF;

        FilterFunction* lppmPDF;

        AttackOperation* attackOperation;

        AttackOutput* attackOutput;

        map<ull, ull> anonymizationMap;

        ull* sigma;

    };
    
    LPM();

    ~LPM();

    bool SetContextFactory(ContextFactory* factory);

    //! 
    //! \brief Runs the specified schedule
    //!
    //! \param[in] schedule 	Schedule*, the schedule to run.
    //! \param[in] actualTraceFile 	File*, the \a actual trace file.
    //! \param[in,out] outputPrefix string, the prefix to use when naming output files.
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the schedule executes successfully)
    //!
    bool RunSchedule(const Schedule* schedule, const File* actualTraceFile, const char* outputPrefix) const;


  private:
    bool RunSubschedule(const Schedule* subschedule, State* state) const;

    bool DoOutput(const Schedule* schedule, SchedulePosition currentPosition, string outputFilePath, const TraceSet* currentTraceSet) const;

    void LogParameters() const;


  public:
    //! 
    //! \brief Runs the knowledge construction
    //!
    //! \param[in] knowledgeFiles 	KnowledgeInput*, the files to use for the construction.
    //! \param[in] outputFile 	File*, the output file.
    //! \param[in] maxGSIterationsPerUser [optional] ull, the maximum number of Gibbs sampling iterations, for each user.
    //! \param[in] maxSecondsPerUser [optional] ull, the maximum number of seconds to spend in the Gibbs sampling procedure, for each user.
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the knowledge is constructed successfully)
    //!
    bool RunKnowledgeConstruction(const KnowledgeInput* knowledgeFiles, File* outputFile, ull maxGSIterationsPerUser = KC_DEFAULT_GS_ITERATIONS, ull maxSecondsPerUser = KC_NO_LIMITS) const;

    bool RunContextAnalysisSchedule(ContextAnalysisSchedule* schedule, const File* contextFile, string outputFileName) const;

    //! 
    //! \brief Generates an artificial actual trace by sampling from the provided background knowledge
    //!
    //! \param[in] knowledge 	File*, the knowledge file to use.
    //! \param[in] output 	File*, the ouput trace file.
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the trace was generated successfully)
    //!
    bool GenerateTracesFromKnowledge(const File* knowledge, File* output) const;

    //! 
    //! \brief Generates an artificial actual trace using the provided trace generator
    //!
    //! \param[in] generator 	TraceGeneratorOperation*, the generator to use.
    //! \param[in] input 	TraceGeneratorInput*, the generator's input.
    //! \param[in] output 	File*, the ouput trace file.
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the trace was generated successfully)
    //!
    bool GenerateTraces(TraceGeneratorOperation* generator, const TraceGeneratorInput* input, File* output) const;

};

} // namespace lpm
#endif
