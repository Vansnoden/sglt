#ifndef LPM_MEMORY_H
#define LPM_MEMORY_H

//!
//! \file
//!
#include "Singleton.h"
#include <map>
using namespace std;
#include <string>
using namespace std;

#include "Defs.h"
#include "NoDepend.h"
#include "Verify.h"

namespace lpm {

//!
//! \brief Provides debug-level memory management functionality
//!
//! Singleton class which allows to allocate/free memory chunks and monitor the reference counting process. 
//! The Report() method can be used to find memory leaks.
//!
//! \note The methods of the class, except the Report() method should never be called directly.
//! The \a Allocate and \a Free macros defined in \a Defs.h should be used instead !
//! 
//! \see Reference, AllocateChunk(), FreeChunk(), UpdateReference(), Report()
//!

class Memory : public Singleton<Memory> 
{
  public:
    Memory();

    ~Memory();


  private:
    map<void*, ull> references;

    map<void*, string> chunks;


  public:
    void* AllocateChunk(ull bytes, const char* file, int line);

    void FreeChunk(void* chunk);

    void UpdateReference(void* object, ull newCount);

    void Report();

};

} // namespace lpm
#endif
