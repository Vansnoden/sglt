#ifndef LPM_SINGLETON_H
#define LPM_SINGLETON_H

//!
//! \file
//!
#include "Defs.h"

namespace lpm {

//!
//! \brief Implements the base Singleton functionality using templates
//!
//!
template<typename T>
class Singleton 
{
  private:
    static T* singletonObject;


  public:
    explicit Singleton();

    virtual ~Singleton();

    //! 
    //! \brief Returns the instance of the Singleton
    //!
    //! \return T*, the instance of the Singleton object
    //!
    static T* GetInstance();

};
template<typename T>
T* Singleton<T>::singletonObject = NULL;

template<typename T>
Singleton<T>::Singleton() 
{
  // Bouml preserved body begin 00023E91
	DEBUG_VERIFY(singletonObject == NULL);

#ifdef UGLY_TRICK
	ll singletonPartOffset = (ll)((T*)1) - (ll)(Singleton<T>*)((T*)1);
	singletonObject = (T*)((ull)this + singletonPartOffset);
#else
	singletonObject = static_cast<T*>(this); // <- should be portable
#endif

  // Bouml preserved body end 00023E91
}

template<typename T>
Singleton<T>::~Singleton() 
{
  // Bouml preserved body begin 00023F11

	DEBUG_VERIFY(singletonObject != NULL);
	delete singletonObject;

  // Bouml preserved body end 00023F11
}

//! 
//! \brief Returns the instance of the Singleton
//!
//! \return T*, the instance of the Singleton object
//!
template<typename T>
T* Singleton<T>::GetInstance()
{
  // Bouml preserved body begin 0001F511
	if(singletonObject == NULL) { new T(); }
	return singletonObject;

  // Bouml preserved body end 0001F511
}


} // namespace lpm
#endif
