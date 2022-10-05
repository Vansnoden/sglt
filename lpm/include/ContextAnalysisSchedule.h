#ifndef LPM_CONTEXTANALYSISSCHEDULE_H
#define LPM_CONTEXTANALYSISSCHEDULE_H


#include <string>
using namespace std;

#include "Defs.h"
#include "Private.h"

namespace lpm { class LoadContextOperation; } 
namespace lpm { class ContextAnalysisOperation; } 

namespace lpm {

class ContextAnalysisSchedule 
{
  private:
    string name;

    LoadContextOperation* loadContextOperation;

    ContextAnalysisOperation* contextAnalysisOperation;


  public:
    ContextAnalysisSchedule(string name, ContextAnalysisOperation* analysis, LoadContextOperation* load = new LoadContextOperation());

    virtual ~ContextAnalysisSchedule();


friend class LPM;
};

} // namespace lpm
#endif
