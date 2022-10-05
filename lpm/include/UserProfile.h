#ifndef LPM_USERPROFILE_H
#define LPM_USERPROFILE_H

//!
//! \file
//!
#include "Reference.h"

#include "Defs.h"

namespace lpm {

//!
//! \brief Represents the mobility profile a given user
//!
//! The user profile of a given user contains a transition matrix and a steady-state vector.
//!
//! \see Context
//!
class UserProfile : public Reference<UserProfile> 
{
  private:
    ull user;

    double* transitionMatrix;

    double* steadystateVector;

    ull numSamples;

    double* varianceMatrix;


  public:
    explicit UserProfile(ull u);

    virtual ~UserProfile();

    ull GetUser() const;

    bool SetTransitionMatrix(const double * matrix);

    bool SetSteadyStateVector(const double* vector);

    bool SetAccuracyInfo(ull samples, const double* variance);

    //! 
    //! \brief Returns the transition matrix
    //!
    //! \param[out] matrix 	double**, a pointer which will point to the output transition matrix (if the call is successful).
    //!
    //! \note The transition matrix is a two dimensional array of doubles (of size \a numLoc x \a numLoc, 
    //! where \a numLoc := \a maxLoc - \a minLoc + 1).
    //! \note \a minLoc and \a maxLoc can be retrieved using the \a GetLocationstampsRange() method of the Parameters singleton class.
    //!
    //! \return true or false, depending on whether the call is successful.
    //!
    bool GetTransitionMatrix(double** matrix) const;

    //! 
    //! \brief Returns the steady-state vector
    //!
    //! \param[out] vector 	double**, a pointer which will point to the output vector (if the call is successful).
    //!
    //! \note The steady-state vector an array of doubles (of size \a numLoc, where \a numLoc := \a maxLoc - \a minLoc + 1).
    //! \note \a minLoc and \a maxLoc can be retrieved using the \a GetLocationstampsRange() method of the Parameters singleton class.
    //!
    //! \return true or false, depending on whether the call is successful.
    //!
    bool GetSteadyStateVector(double** vector) const;

    bool GetAccuracyInfo(ull* samples, double** variance);

};

} // namespace lpm
#endif
