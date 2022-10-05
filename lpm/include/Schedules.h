#ifndef LPM_SCHEDULES_H
#define LPM_SCHEDULES_H

//!
//! \file
//!
#include <string>
using namespace std;
#include "Singleton.h"
#include <vector>
using namespace std;

#include "Defs.h"
#include "Private.h"

#include "ExampleApplicationOperations.h"
#include "ExampleLPPMOperations.h"
#include "WeakAttackOperation.h"
#include "StrongAttackOperation.h"

namespace lpm { class Schedule; } 
namespace lpm { class File; } 
namespace lpm { class ScheduleBuilder; } 
namespace lpm { class MetricDistance; } 
namespace lpm { class ApplicationOperation; } 
namespace lpm { class LPPMOperation; } 

namespace lpm {

//!
//! \brief Base class for schedule templates
//! 
//! \note Derived class should also derive from Singleton<T>.
//! As a consequence they must implement the ResetTemplate() method.
//!
//! \see BuildSchedule(), ResetTemplate()
//!

class ScheduleTemplate 
{
  public:
    ScheduleTemplate();

    virtual ~ScheduleTemplate();

    //! 
    //! \brief Builds the schedule specified by the template and the parameters given
    //!
    //! \param[in] contextFile 	File*, the background knowledge (context) file that the schedule will use.
    //! \param[in] name 	string, the name of the schedule built.
    //!
    //! \return a pointer to a Schedule object if call is successful, \a NULL otherwise 
    //!
    virtual Schedule* BuildSchedule(const File* contextFile, string name) = 0;


  protected:
    ScheduleBuilder* builder;


  public:
    //! 
    //! \brief Resets the schedule template parameters to their default values
    //!
    //! \return nothing
    //!
    virtual void ResetTemplate() = 0;

};
//!
//! \brief Provides a template for simple schedules
//!
//! \see the \a Quick \a Start \a Guide, ScheduleBuilder
//!

class SimpleScheduleTemplate : public ScheduleTemplate, public Singleton<SimpleScheduleTemplate> 
{
  private:
    double mu;

    ApplicationType appType;

    ull obfuscationLevel;

    FakeInjectionAlgorithm fakeInjectionAlgo;

    double fakeInjectionProb;

    double hidingProb;

    AttackType attackType;

    bool genericReconstruction;

    ull genericReconstructionSamples;

    MetricType metricType;

    MetricDistance* metricDistance;

    ApplicationOperation* applicationInstance;

    LPPMOperation* lppmInstance;


  public:
    SimpleScheduleTemplate();

    Schedule* BuildSchedule(const File* contextFile, string name);

    void SetApplicationParameters(ApplicationType type = Basic, double mu = 0.2);

    bool SetApplicationOperation(ApplicationOperation* application);

    void SetLPPMParameters(ull obfuscationLevel = 2, FakeInjectionAlgorithm fakeInjectionAlgo = UniformSelection, double fakeInjectionProb = 0.1, double hidingProb = 0.1);

    bool SetLPPMOperation(LPPMOperation* lppm);

    void SetAttackParameters(AttackType type = Strong, bool genericRec = false, ull genericRecSamples = 500);

    void SetMetricParameters(MetricType type = Distortion, MetricDistance* distance = NULL);

    void ResetTemplate();

};
//!
//! \brief Provides a simple schedules template for comparing LPPMs 
//!
//! \see ScheduleBuilder, SimpleScheduleTemplate
//!

class SimpleLPPMComparisonScheduleTemplate : public Singleton<SimpleLPPMComparisonScheduleTemplate>, public ScheduleTemplate 
{
  private:
    double mu;

    ApplicationType appType;

    vector<LPPMOperation*> lppmsVector;

    AttackType attackType;

    bool genericReconstruction;

    ull genericReconstructionSamples;

    MetricType metricType;

    MetricDistance* metricDistance;


  public:
    SimpleLPPMComparisonScheduleTemplate();

    Schedule* BuildSchedule(const File* contextFile, string name);

    void ResetTemplate();

    //! 
    //! \brief Sets the lppms the template will use to build schedules
    //!
    //! \param[in] lppms 	vector<LPPMOperation*>, the lppms to use.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool SetLPPMs(vector<LPPMOperation*> lppms);

    void SetApplicationParameters(ApplicationType type = Basic, double mu = 0.2);

    void SetAttackParameters(AttackType type = Strong, bool genericRec = false, ull genericRecSamples = 500);

    void SetMetricParameters(MetricType type = Distortion, MetricDistance* distance = NULL);

};

} // namespace lpm
#endif
