#ifndef LPM_ALGORITHMS_H
#define LPM_ALGORITHMS_H

//!
//! \file
//!
#include "Defs.h"
#include "Private.h"

namespace lpm {

//!
//! \brief Implements useful algorithms used by the library
//!
//! Static class which provides methods for some common useful algorithms such as MaximumWeightAssignment() and MultiplySquareMatrices().
//!
class Algorithms 
{
  private:
    //[MinimumCostAssignment]: Step 1: Reduce the cost matrix by subtracting the row min from every row, and the column min from every column.
    //This works because the resulting assignment (on the reduce matrix) corresponds to the same assignment on the non-reduced matrix
    static void ReduceCostMatrix(ll* M, ll n);

    //[MinimumCostAssignment]: Step 2: Star some arbitrary zeros (making sure that no two zeros in the same row or column are starred).
    static ll StarArbitraryZeros(ll* M, ll* starred, ll n);

    //[MinimumCostAssignment]: Find any uncovered zeros
    static bool FindAnyUncoveredZero(ll* M, ll n, ll& row, ll& col);

    //[MinimumCostAssignment]: Cover all zeros by changing the adding/removing cover lines.
    static ll CoverAllZeros(ll* M, ll* starred, ll* primed, ll n, ll& covers);


  public:
    //This implementation is heavily inspired from the code of Dariush Lotfi (June 2008).
    //See http://csclab.murraystate.edu/bob.pilgrim/445/munkres.html for the algorithm itself.
    static void MinimumCostAssignment(ll* costMatrix, ll numItems, ll* assignment);

    static int MaximumWeightAssignment(const ll numItems, ll* weight, ll* mapping);

    static void MultiplySquareMatrices(const double* leftMatrix, const double* rightMatrix, ull dimension, double* resultMatrix);

    static bool GetSteadyStateVectorOfSubChain(double* fullChainSS, ull timePeriodId, double** subChainSS, bool inclDummyTPs = false);

    static bool GetTransitionVectorOfSubChain(double* fullChainTransitionMatrix, ull tp1, ull loc1, ull tp2, double** transitionVector, bool inclDummyTPs = false);

};

} // namespace lpm
#endif
