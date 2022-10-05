#ifndef LPM_PARAMETERS_H
#define LPM_PARAMETERS_H

//!
//! \file
//!
#include <map>
using namespace std;
#include "Singleton.h"
#include <vector>
using namespace std;

#include "Defs.h"

#ifndef WIN32
#include "TimePartitioning.h"
#endif

#define PARAMETERS_DEFAULT_MAX_USERS (1 << 6) // 64
#define PARAMETERS_DEFAULT_MAX_TIMESTAMPS (1 << 4) // 16
#define PARAMETERS_DEFAULT_MAX_LOCATIONSTAMPS (1 << 4) // 16

namespace lpm { class TPNode; } 
namespace lpm { struct TimePeriod; } 

namespace lpm {

struct TPInfo 
{
    ull numPeriods;

    ull minPeriod;

    ull maxPeriod;

    ull numPeriodsInclDummies;

    TPNode* partitioning;

    TPNode* canonicalPartitionParentNode;

    map<ull, TPNode*> partParentNodeMap;

    double* propTPVector;

    double* propTransMatrix;

};
//!
//! \brief Encompasses the parameters of the simulation
//!
//! Singleton class which stores the parameters of the simulation (i.e. min and max user IDs, timestamps, locationstamps).
//!
class Parameters : public Singleton<Parameters> 
{
  private:
    ull minTimestamp;

    ull maxTimestamp;

    ull minLocationstamp;

    ull maxLocationstamp;

    vector<pair<ull, ull> > usersRanges;

    TPInfo tpInfo;


  public:
    Parameters();

    ~Parameters();

    bool GetUsersSet(set<ull>& users);

    ull GetUsersCount();

    bool UserExists(ull user);

    bool AddUsersRange(ull min, ull max);

    bool RemoveUsersRange(ull min, ull max);

    void ClearUsersSet();

    bool GetTimestampsRange(ull* min, ull* max) const;

    bool SetTimestampsRange(ull min, ull max);

    bool GetLocationstampsRange(ull* min, ull* max) const;

    bool SetLocationstampsRange(ull min, ull max);

    TPNode* CreateTimePartitioning(ull offset, ull length);

    bool SetTimePartitioning(TPNode* partitioning);

    ull LookupTimePeriod(ull tm, bool inclDummies = false, TimePeriod* absTP = NULL, TPNode** partParentNode = NULL);

    bool GetTimePeriodInfo(ull* numPeriods, TPInfo* tpInfo = NULL);


  private:
    bool InitializeTPInfo(TPNode* partitioning);

};

} // namespace lpm
#endif
