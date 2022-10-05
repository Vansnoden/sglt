#ifndef SGLPPMOPERATION_H_
#define SGLPPMOPERATION_H_

#include "include/Public.h"

using namespace lpm;
using namespace std;

class SGLPPMOperation : public LPPMOperation
{
  public:
    SGLPPMOperation(File& traceFile, map<ull, ull> ltcMap, vector<set<ull> >& clustersVec, double rmp = 0.4, double mergep = 0.5, double rmacl = 1.0);

    virtual ~SGLPPMOperation();


  private:

    map<ull, vector<set<ull> > > ssClustersMap;
    map<ull, vector<ull> > ssClusterIdxMap;

    map<ull, ull> locToClusterIdxMap;

    double probRemoveActualLoc;


    bool LookupCluster(ull user, ull tm, set<ull>& out) const;

  public:
    virtual bool Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent);

    virtual double PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const;
};

#endif /* SGLPPMOPERATION_H_ */
