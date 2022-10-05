#ifndef LPM_VERIFY_H
#define LPM_VERIFY_H

//!
//! \file
//!
#include "Defs.h"
#include "NoDepend.h"

#define RUNTIME_VERIFY(_c) { Verify::RuntimeVerify((_c), #_c, __FILE__, __LINE__); }

namespace lpm {

//!
//! \brief Implements run-time (release mode) assertion mechanism.
//!
//! \note Methods of this class should not be called directly. The VERIFY macro (defined in \a "Defs.h") should be used instead.
//!
//

class Verify 
{
  public:
    static void RuntimeVerify(bool condition, const char* conditionString, const char* file, int line);

};

} // namespace lpm
#endif
