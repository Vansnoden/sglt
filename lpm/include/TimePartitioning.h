#ifndef LPM_TIMEPARTITIONING_H
#define LPM_TIMEPARTITIONING_H

//!
//! \file
//!
#include <vector>
using namespace std;
#include <string>
using namespace std;

#include "Defs.h"
#define INVALID_TIME_PERIOD 0
#define TIME_PARTITIONING_NODE_DELIMITER '\t'

#define IS_ROOT_NODE 	((this == parent) ? true : false)

#include "Memory.h"

namespace lpm { class File; } 

namespace lpm {

struct TimePeriod 
{
    ull start;

    ull length;

    ull id;

    bool dummy;

    bool operator<(const TimePeriod& right) const;

};
class TPNode 
{
  protected:
    TPNode* parent;

    ull offset;

    ull length;


  private:
    vector<TPNode*> slices;

    vector<TPNode*> groups;

    bool partitioned;

    TPNode(ull off, ull len);


  public:
    static TPNode* FromFile(File* file);


  private:
    static TPNode* FromStrings(vector<string>& strings);


  public:
    virtual ~TPNode();

    virtual bool SliceOut(ull offset, ull length, ull repetitions, TPNode** slice);

    virtual bool Partition(vector<TimePeriod>& periods);

    virtual ull LookupTimePeriod(ull tm, TimePeriod* absTP = NULL, TPNode** leaf = NULL);

    virtual bool GetStringRepresentation(string& str, ull level = 0);

    ull GetOffset(bool abs) const;

    ull GetLength() const;


  private:
    virtual ull FillWithDummyTimePeriods(ull nextTPIdx);


  public:
    static bool IsLessTan(const TPNode* left, const TPNode* right);


friend class Parameters;
friend class TPGroup;
friend class TPLeaf;
};
class TPLeaf : public TPNode 
{
  private:
    TimePeriod* periods;

    ull numPeriods;

    TPLeaf(ull len);


  public:
    virtual ~TPLeaf();

    virtual bool SliceOut(ull offset, ull length, ull repetitions, vector<TPNode*>& slice);

    virtual bool Partition(vector<TimePeriod>& periods);

    virtual ull LookupTimePeriod(ull tm, TimePeriod* absTP = NULL, TPNode** leaf = NULL);

    virtual bool GetStringRepresentation(string& str, ull level = 0);


  private:
    virtual ull FillWithDummyTimePeriods(ull nextTPIdx);


friend class TPNode;
friend class Parameters;
};
class TPGroup : public TPNode 
{
  private:
    vector<TPNode*> group;

    TPGroup(ull off, ull len, vector<TPNode*>& members);


  public:
    virtual ~TPGroup();

    virtual bool SliceOut(ull offset, ull length, ull repetitions, TPNode** slice);

    virtual bool Partition(vector<TimePeriod>& periods);


  private:
    virtual ull FillWithDummyTimePeriods(ull nextTPIdx);

    virtual ull LookupTimePeriod(ull tm, TimePeriod* absTP = NULL, TPNode** leaf = NULL);

    virtual bool GetStringRepresentation(string& str, ull level = 0);

    ull GetOffset(bool abs) const;

    ull GetLength() const;


friend class TPNode;
};

} // namespace lpm
#endif
