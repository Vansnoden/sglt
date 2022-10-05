#ifndef LPM_DEFS_H
#define LPM_DEFS_H

//!
//! \file
//!
// language includes
#include <cassert>
#include <climits>
#include <stdint.h>

#include <cstring>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>

#include <iostream>
#include <fstream>
#include <sstream>

#include <set>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <cerrno>

using namespace std;

#define ASSERT assert

#ifdef DEBUG
	#define VERIFY ASSERT
#else
	#define VERIFY RELEASE_VERIFY
#endif

#define RELEASE_VERIFY RUNTIME_VERIFY
#define DEBUG_VERIFY ASSERT

#define CODING_ERROR ASSERT(0 == 1)

#ifdef FORCE_RELEASE
	#ifdef DEBUG
		#define Allocate(_s) Memory::GetInstance()->AllocateChunk((_s), __FILE__, __LINE__)
		#define Free Memory::GetInstance()->FreeChunk
	#else
		#define Allocate malloc
		#define Free free
	#endif
#else
	#define Allocate(_s) Memory::GetInstance()->AllocateChunk((_s), __FILE__, __LINE__)
	#define Free Memory::GetInstance()->FreeChunk
#endif

#define CONTAINS_FLAG(_v, _f) (((ull)(_v) & (ull)(_f)) != 0 ? true : false)

#define foreach(T, _c, i) \
	for(T::iterator i = (_c).begin(); i != (_c).end(); ++i)

#define foreach_const(T, _c, i) \
	for(T::const_iterator i = (_c).begin(); i != (_c).end(); ++i)

#define foreach_const_reverse(T, _c, i) \
	for(T::const_reverse_iterator i = (_c).rbegin(); i != (_c).rend(); ++i)

#define pair_foreach(T1, T2, _c, i) \
	for(T1, T2::iterator i = (_c).begin(); i != (_c).end(); ++i)

#define pair_foreach_const(T1, T2, _c, i) \
	for(T1, T2::const_iterator i = (_c).begin(); i != (_c).end(); ++i)

#define GET_INDEX(_i, _j, _numCol) ((_i) * (_numCol) + (_j))

#define GET_INDEX_3D(_i, _j, _k, _numRow, _numCol) (((_i) * (_numRow) * (_numCol)) + (_j) * (_numCol) + (_k))

#define GET_INDEX_4D(_i, _j, _k, _l, _numPage, _numRow, _numCol) (((_i) * (_numPage) * (_numRow) * (_numCol)) + (_j) * (_numRow) * (_numCol) + (_k) * (_numCol) + (_l))

#define DEFAULT_FIELDS_DELIMITER ','
#define DEFAULT_FIELD_VALUES_DELIMITER '|'

#define STRINGIFY(_sth) #_sth

#define ABS(_a) (((_a) >= 0) ? (_a) : -(_a))
#define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
#define MAX(_a, _b) (((_a) < (_b)) ? (_b) : (_a))

#define EPSILON 0.0001

#define SQRT_DBL_MIN 1.4916681462400413e-154

#define NORMALIZE_VECTOR(_pvec, _n) { double sum = 0.0; for(ull normIdx=0; normIdx < (_n); normIdx++) { sum += (_pvec)[normIdx]; } for(ull normIdx=0; normIdx < (_n); normIdx++) { (_pvec)[normIdx] /= sum; } }

namespace lpm {

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64, ll;
typedef uint64_t uint64, ull;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;

//! 
//! \brief Defines trace types (actual, exposed, observed) 
//! 
//! \see EventType
//! 
enum TraceType 
{
  ActualTrace = 0, 
  ExposedTrace = ActualTrace + 1, 
  ObservedTrace = ExposedTrace + 1 

};
//!
//! \brief Defines events types (actual, exposed, observed)
//! 
//! \see TraceType
//!
enum EventType 
{
  Actual = ActualTrace, 
  Exposed = ExposedTrace, 
  Observed = ObservedTrace 

};
//!
//! \brief Defines metric types (distortion, anonymity, entropy, etc.)
//!

enum MetricType 
{
  Distortion = 0, 
  MostLikelyLocationDistortion = Distortion + 1, 
  MostLikelyTraceDistortion = MostLikelyLocationDistortion + 1, 
  Entropy = MostLikelyTraceDistortion + 1, 
  Density = Entropy + 1, 
  MeetingDisclosure = Density + 1, 
  Anonymity = MeetingDisclosure + 1,
  SGMetric = Anonymity + 1

};
//!
//! \brief Algorithm used for fake injection (either \a UniformSelection, or \a GeneralStatisticsSelection)
//!

enum FakeInjectionAlgorithm 
{
  UniformSelection = 0, 
  GeneralStatisticsSelection = UniformSelection + 1 

};
//!
//! \brief Defines application types
//!

enum ApplicationType 
{
  Basic = 0, 
  LocalSearch = Basic + 1 

};
enum LPPMFlags 
{
  NoFlags = 0,
  Anonymization = 1, 
  PseudonymChange = 2 

};
//!
//! \brief Defines adversary's power (weak, strong)
//!

enum AttackType 
{
  Weak = 0, 
  Strong = Weak + 1 

};

} // namespace lpm
#endif
