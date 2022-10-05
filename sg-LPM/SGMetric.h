#ifndef SGMETRIC_H_
#define SGMETRIC_H_

#include "include/Public.h"

using namespace lpm;
using namespace std;


class SGMetricOperation : public MetricOperation
{
  public:
	SGMetricOperation();

    ~SGMetricOperation();

    virtual bool Execute(const AttackOutput* input, File* output);

    virtual string GetTypeString() const;
};


#endif
