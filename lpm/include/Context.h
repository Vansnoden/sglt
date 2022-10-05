#ifndef LPM_CONTEXT_H
#define LPM_CONTEXT_H

//!
//! \file
//!
#include "Reference.h"
#include <map>
using namespace std;


class DefaultContextFactory;

namespace lpm { class UserProfile; } 

namespace lpm {

//!
//! \brief Encompasses the knowledge of the adversary
//!
//! The knowledge of the adversary is composed (for each user) of a user profile.
//! 
//! \see UserProfile
//!

class Context : public Reference<Context> 
{
  private:
    map<ull, UserProfile*> userProfiles;


  public:
    Context();

    virtual ~Context();

    bool AddProfile(const UserProfile* profile);

    bool GetProfiles(map<ull, UserProfile*>& profiles) const;

    bool GetUserProfile(ull user, UserProfile** profile) const;

    bool RemoveUserProfile(ull user);

    void ClearProfiles();

};
//!
//! \brief Implements the default context factory
//!
//! \note The context factory used can be overriden through inheritance and the use of the \a LPM::SetContextFactory() method
//!
//! \see LPM, LPM::SetContextFactory()
//!

class ContextFactory 
{
  public:
    ContextFactory();

    virtual ~ContextFactory();

    virtual Context* NewContext() const;

};

} // namespace lpm
#endif
