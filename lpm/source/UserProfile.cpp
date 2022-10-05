//!
//! \file
//!
#include "../include/UserProfile.h"

namespace lpm {

UserProfile::UserProfile(ull u) 
{
  // Bouml preserved body begin 00045F11

	user = u;
	steadystateVector = NULL;
	transitionMatrix = NULL;

	numSamples = 0;
	varianceMatrix = NULL;

  // Bouml preserved body end 00045F11
}

UserProfile::~UserProfile() 
{
  // Bouml preserved body begin 00049311

	if(steadystateVector != NULL) { Free(steadystateVector); steadystateVector = NULL; }
	if(transitionMatrix != NULL) { Free(transitionMatrix); transitionMatrix = NULL; }

	if(varianceMatrix != NULL) { Free(varianceMatrix); varianceMatrix = NULL; }

  // Bouml preserved body end 00049311
}

ull UserProfile::GetUser() const 
{
  // Bouml preserved body begin 00045E91

	return user;

  // Bouml preserved body end 00045E91
}

bool UserProfile::SetTransitionMatrix(const double * matrix) 
{
  // Bouml preserved body begin 00045F91

	if(matrix == NULL) { return false; }
	// TODO: Add more checks

	transitionMatrix = (double*)matrix;

	return true;

  // Bouml preserved body end 00045F91
}

bool UserProfile::SetSteadyStateVector(const double* vector) 
{
  // Bouml preserved body begin 00046011

	if(vector == NULL) { return false; }
	// TODO: Add more checks

	steadystateVector = (double*)vector;

	return true;

  // Bouml preserved body end 00046011
}

bool UserProfile::SetAccuracyInfo(ull samples, const double* variance) 
{
  // Bouml preserved body begin 000C0311

	if(variance == NULL) { return false; }

	numSamples = samples;
	varianceMatrix = (double*)variance;

	return true;

  // Bouml preserved body end 000C0311
}

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
bool UserProfile::GetTransitionMatrix(double** matrix) const 
{
  // Bouml preserved body begin 00049391

	if(matrix == NULL) { return false; }

	*matrix = transitionMatrix;

	return true;

  // Bouml preserved body end 00049391
}

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
bool UserProfile::GetSteadyStateVector(double** vector) const 
{
  // Bouml preserved body begin 00049411

	if(vector == NULL) { return false; }

	*vector = steadystateVector;

	return true;

  // Bouml preserved body end 00049411
}

bool UserProfile::GetAccuracyInfo(ull* samples, double** variance) 
{
  // Bouml preserved body begin 000C0391

	if(samples == NULL || variance == NULL || varianceMatrix == NULL) { return false; }

	*samples = numSamples;
	*variance = varianceMatrix;

	return true;

  // Bouml preserved body end 000C0391
}


} // namespace lpm
