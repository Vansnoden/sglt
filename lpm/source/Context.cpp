//!
//! \file
//!
#include "../include/Context.h"
#include "../include/UserProfile.h"

namespace lpm {

Context::Context() 
{
  // Bouml preserved body begin 0002D191

	userProfiles = map<ull, UserProfile*>();

  // Bouml preserved body end 0002D191
}

Context::~Context() 
{
  // Bouml preserved body begin 0002D211

	ClearProfiles();

  // Bouml preserved body end 0002D211
}

bool Context::AddProfile(const UserProfile* profile) 
{
  // Bouml preserved body begin 00045D11

	const_cast<UserProfile*>(profile)->AddRef();

	userProfiles.insert(pair<ull, UserProfile*>(profile->GetUser(), const_cast<UserProfile*>(profile)));

	return true;

  // Bouml preserved body end 00045D11
}

bool Context::GetProfiles(map<ull, UserProfile*>& profiles) const 
{
  // Bouml preserved body begin 00045D91

	profiles = userProfiles;

	return true;

  // Bouml preserved body end 00045D91
}

bool Context::GetUserProfile(ull user, UserProfile** profile) const 
{
  // Bouml preserved body begin 0004ED91

	if(profile == NULL) { return false; }

	map<ull, UserProfile*>::const_iterator iter = userProfiles.find(user);
	if(iter != userProfiles.end()) // if we find the user profile
	{
		*profile = iter->second;

		return true;
	}

	return false;

  // Bouml preserved body end 0004ED91
}

bool Context::RemoveUserProfile(ull user) 
{
  // Bouml preserved body begin 0008F591

	map<ull, UserProfile*>::iterator iter = userProfiles.find(user);
	if(iter != userProfiles.end()) // if we find the user profile
	{
		UserProfile* profile = iter->second;
		profile->Release();

		userProfiles.erase(iter);

		return true;
	}

	return false;

  // Bouml preserved body end 0008F591
}

void Context::ClearProfiles() 
{
  // Bouml preserved body begin 00085411

	pair_foreach_const(map<ull, UserProfile*>, userProfiles, iter)
	{
		UserProfile* profile = iter->second;
		profile->Release();
	}

	userProfiles.clear();

  // Bouml preserved body end 00085411
}

ContextFactory::ContextFactory() 
{
  // Bouml preserved body begin 00096611
  // Bouml preserved body end 00096611
}

ContextFactory::~ContextFactory() 
{
  // Bouml preserved body begin 00096691
  // Bouml preserved body end 00096691
}

Context* ContextFactory::NewContext() const 
{
  // Bouml preserved body begin 00096511

	return new Context(); // default implementation

  // Bouml preserved body end 00096511
}


} // namespace lpm
