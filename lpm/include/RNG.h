#ifndef LPM_RNG_H
#define LPM_RNG_H

//!
//! \file
//! The implementation of some of the methods below is inspired by the GNU Scientific Library (GSL)
//! See: http://www.gnu.org/software/gsl/.
//!
#include "Singleton.h"

#include "Defs.h"
namespace lpm {

//!
//! \brief Implements random number generating facilities
//!
//! Singleton class which provides convenient methods to generate random numbers (integers and real numbers) uniformly or
//! according to a distribution (e..g Normal, Gamma), and, to sample from a distribution (e.g. Dirichlet).
//!
class RNG : public Singleton<RNG> 
{
friend class Singleton<RNG>;
  private:
    RNG();

    inline uint64 RandomUINT64() const;


  public:
    //! 
    //! \brief Returns a uniform random double in ]0; 1[
    //!
    //! \return double, the number generated
    //!
    double GetUniformRandomDouble() const;


  private:
    inline ull GetUniformRandomULL() const;


  public:
    //! 
    //! \brief Returns a uniform random unsigned long long (\a ull) between \a min and \a max (inclusive)
    //!
    //! \param[in] min 	ull, the lower bound.
    //! \param[in] max 	ull, the upper bound.
    //!
    //! \return ull, the number generated
    //!
    ull GetUniformRandomULLBetween(ull min, ull max) const;

    //! 
    //! \brief Returns a random permutation of the given \a input array
    //!
    //! \param[in] input 	ull*, the input array (an array of \a count \a ulls).
    //! \param[in] count 	ull, the number of elements in the \a input array
    //! \param[in,out] output ull*, the output array (an array of \a count \a ull numbers) which will be filled with the output permutation (if the call is successful).
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool RandomPermutation(const ull* input, const ull count, ull* output) const;

    //! 
    //! \brief Samples (returns an index) from the given probability vector
    //!
    //! Samples an index in the range \[\a 0; \a length - 1\] according to the probabilities given by \a probVector.
    //!
    //! \note Naturally, this only make sense if the elements of \a probVector sum up to 1.
    //!
    //! \param[in] probVector 	ull*, the probability vector (an array of \a length \a doubles).
    //! \param[in] length 	ull, the number of elements in the probability vector
    //!
    //! \return ull, the sampled index
    //!
    ull SampleIndexFromVector(double* probVector, ull length) const;


  private:
    inline double GetGaussianRandomDouble(double sigma) const;

    inline double GetGammaRandomDouble(double a, double b) const;

    inline void GetDirichletRandomSampleSmall(const double* alpha, ull K, double* theta) const;


  public:
    void GetDirichletRandomSample(const double* alpha, ull K, double* theta) const;

    void GetDirichletExpectedValue(double* alpha, ull K, double* theta) const;

};

} // namespace lpm
#endif
