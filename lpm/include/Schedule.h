#ifndef LPM_SCHEDULE_H
#define LPM_SCHEDULE_H

//!
//! \file
//!
#include <string>
using namespace std;
#include <vector>
using namespace std;
#include <map>
using namespace std;
#include "Reference.h"

#include "Defs.h"
#include "Private.h"

namespace lpm { class InputOperation; } 
namespace lpm { class File; } 
namespace lpm { class LoadContextOperation; } 
namespace lpm { class ApplicationOperation; } 
namespace lpm { class LPPMOperation; } 
namespace lpm { class AttackOperation; } 
namespace lpm { class MetricDistance; } 
namespace lpm { class OutputOperation; } 
namespace lpm { class FilterFunction; } 

namespace lpm {

//!
//! \brief Defines a positional scheme for the stages of a schedule.
//!
enum SchedulePosition 
{
  ScheduleBeginning = 0, 
  ScheduleBeforeInputs = ScheduleBeginning, 
  ScheduleBeforeApplicationOperation = ScheduleBeforeInputs + 1, 
  ScheduleBeforeLPPMOperation = ScheduleBeforeApplicationOperation + 1, 
  ScheduleBeforeAttackOperation = ScheduleBeforeLPPMOperation + 1, 
  ScheduleBeforeMetricOperation = ScheduleBeforeAttackOperation + 1, 
  ScheduleEnd = ScheduleBeforeMetricOperation + 1, 
  ScheduleInvalidPosition = -1 

};
//!
//! \brief Models a schedule
//!
//! \note The IsValid() method can be called to verify that the Schedule* object is valid (i.e. makes sense with respect to the library and the framework)
//!
class Schedule : public Reference<Schedule> 
{
  private:
    string name;

    void* builder;

    SchedulePosition startPosition;

    SchedulePosition endPosition;

    explicit Schedule(string name);


  public:
    virtual ~Schedule();


  private:
    bool root;

    InputOperation* inputOperation;

    File* actualTraceFile;

    InputOperation* actualTraceInputOperation;

    File* contextFile;

    LoadContextOperation* loadContextOperation;

    ApplicationOperation* applicationOperation;

    LPPMOperation* lppmOperation;

    AttackOperation* attackOperation;

    MetricType metricType;

    MetricDistance* distanceFunction;

    vector<Schedule*> branches;

    SchedulePosition branchingPosition;

    map<SchedulePosition, OutputOperation*> outputOperations;


  public:
    bool IsValid() const;

    string GetDetailString(ull level = 0) const;


  private:
    void ClearOutputOperations();

    bool AddOutputOperation(const OutputOperation* outputOperation, SchedulePosition position = ScheduleEnd);

    OutputOperation* GetOutputOperationAt(SchedulePosition pos) const;

    SchedulePosition afterInputPosition;

    FilterFunction* appPDF;

    FilterFunction* lppmPDF;


friend class LPM;
friend class ScheduleBuilder;
};

} // namespace lpm
#endif
