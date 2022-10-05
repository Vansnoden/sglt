//!
//! \file
//!
#include "../include/Algorithms.h"

namespace lpm {

//[MinimumCostAssignment]: Step 1: Reduce the cost matrix by subtracting the row min from every row, and the column min from every column.
//This works because the resulting assignment (on the reduce matrix) corresponds to the same assignment on the non-reduced matrix
void Algorithms::ReduceCostMatrix(ll* M, ll n)
{
  // Bouml preserved body begin 000C5091

	for(ll i=0;i<n;i++)
	{
		ll min = LONG_LONG_MAX;
		for(ll j=0;j<n;j++) {  if (M[GET_INDEX(i, j, n+1)] < min) { min = M[GET_INDEX(i, j, n+1)]; } }
		for(ll j=0;j<n;j++) {  M[GET_INDEX(i, j, n+1)] -= min; }
	}
	for(ll j=0;j<n;j++)
	{
		ll min = LONG_LONG_MAX;
		for(ll i=0;i<n;i++) {  if (M[GET_INDEX(i, j, n+1)] < min) { min = M[GET_INDEX(i, j, n+1)]; } }
		for(ll i=0;i<n;i++) {  M[GET_INDEX(i, j, n+1)] -=  min; }
	}

  // Bouml preserved body end 000C5091
}

//[MinimumCostAssignment]: Step 2: Star some arbitrary zeros (making sure that no two zeros in the same row or column are starred).
ll Algorithms::StarArbitraryZeros(ll* M, ll* starred, ll n)
{
  // Bouml preserved body begin 000C5111

	ll starredCount = 0;

	for(ll i=0;i<n;i++)
	{
		for(ll j=0;j<n;j++)
		{
			if(starred[i] == -1 && M[GET_INDEX(n, j, n+1)] == 0 && M[GET_INDEX(i, j, n+1)] == 0)
			{
				M[GET_INDEX(n, j, n+1)] = 1;
				starred[i] = j;
				starredCount++;
			}
		}
	}

	return starredCount;

  // Bouml preserved body end 000C5111
}

//[MinimumCostAssignment]: Find any uncovered zeros
bool Algorithms::FindAnyUncoveredZero(ll* M, ll n, ll& row, ll& col)
{
  // Bouml preserved body begin 000C5191

	for(ll i=0;i<n;i++)
	{
		for(ll j=0;j<n;j++)
		{
			if(M[GET_INDEX(i, n, n+1)] == 0 && M[GET_INDEX(n, j, n+1)] == 0 && M[GET_INDEX(i, j, n+1)] == 0)
			{
				row = i;
				col = j;
				return true; // found
			}
		}
	}

	return false;

  // Bouml preserved body end 000C5191
}

//[MinimumCostAssignment]: Cover all zeros by changing the adding/removing cover lines.
ll Algorithms::CoverAllZeros(ll* M, ll* starred, ll* primed, ll n, ll& covers)
{
  // Bouml preserved body begin 000C5211

	ll row = 0;
	ll col = 0;

	//Repeat until there are no uncovered zeros left
	while(FindAnyUncoveredZero(M, n, row, col) == true)
	{
		if(starred[row] != -1) // If there is a starred zero in the same row of uncovered zero
		{
			primed[row] = col; // Priming the uncovered zero
			M[GET_INDEX(row, n, n+1)] = 1; // Covering the row of uncovered zero
			M[GET_INDEX(n, starred[row], n+1)] = 0; // Uncovering the column containing the starred zero
		}
		else // If there is no starred zero in the same row of uncovered zero
		{
			// According to the algorithm uncovered zero should be primed first, but
			// because it will be starred later, I don't prime it.

			ll i = 0;
			// Looking for a starred zero in the same column of uncovered (primed) zero
			for(i=0;i<n;i++) { if(starred[i] == col) { break; } }

			// Repeat until there is no starred zero in the same column as primed zero
			while(i != n)
			{
				starred[row] = col; // Starring the primed zero
				starred[i] = -1; // Un-starring the starred zero in the same column of primed zero

				//Finding the primed zero in the same row of that starred zero
				//(there will always be one)
				col = primed[i];
				row = i;

				//Looking for a starred zero in the same column of the found primed zero
				for(i=0;i<n;i++) { if(starred[i] == col) { break; } }
			}

			//Starring the last primed zero that has no starred zero in its column
			starred[row] = col;

			//Erasing all primes and uncover every rows in the matrix while covering
			//all columns containing a starred zero
			for(ll j=0;j<n;j++)
			{
				primed[j] = -1;
				M[GET_INDEX(j, n, n+1)] = 0;
				if(starred[j] != -1) { M[GET_INDEX(n, starred[j], n+1)] = 1; }
			}
			covers++;
			if(covers == n) { return n; }
		}
	}

	return covers;

  // Bouml preserved body end 000C5211
}

//This implementation is heavily inspired from the code of Dariush Lotfi (June 2008).
//See http://csclab.murraystate.edu/bob.pilgrim/445/munkres.html for the algorithm itself.
void Algorithms::MinimumCostAssignment(ll* costMatrix, ll numItems, ll* assignment)
{
  // Bouml preserved body begin 000C5291

	ll n = numItems;

	// Starred zeros indicates assignment pairs when the algorithm
	// is finished; so there is no need to define a new vector.
	ll* starred = assignment;

	// Allocate memory for primed
	ull primedByteSize = n * sizeof(ll);
	ll* primed = (ll*)Allocate(primedByteSize);
	VERIFY(primed != NULL); memset(primed, 0, primedByteSize);

	// Allocate M, same as weight matrix but with an extra line and column
	ull matrixByteSize = (n+1) * (n+1) * sizeof(ll);
	ll* M = (ll*)Allocate(matrixByteSize);
	VERIFY(M != NULL); memset(M, 0, matrixByteSize);

	// Initialize M
	for(ll i = 0; i < n; i++)
	{
		for(ll j = 0; j < n; j++)
		{
			M[GET_INDEX(i, j, n+1)] = costMatrix[GET_INDEX(i, j, n)]; // notice: n+1 vs. n
		}
	}

	// Note: covered-flag row & column of M (i.e. row & column having index n) indicate which rows or columns are covered.
	for(ll i=0;i<n;i++) { M[GET_INDEX(i, n, n+1)] = 0; }
	for(ll j=0;j<n;j++) { M[GET_INDEX(n, j, n+1)] = 0; }

	// Initialize starred and primed arrays.
	// If M[GET_INDEX(i, j, n+1)] is a starred zero, starred[i] has the value of j and if
	// there is no starred zero at the row i, starred[i] has the value of -1.
	// The same is true for primed zeros and primed vector values.
	for(ll i=0;i<n;i++)
	{
		starred[i] = -1;
		primed[i] = -1;
	}

	//Step 0: Making reduced cost matrix
	ReduceCostMatrix(M, n);

	//Step 1: Marking some zeros with star for beginning
	ll covers = StarArbitraryZeros(M, starred, n);

	while(covers != n)
	{
		//Steps 2 and 3: Changing or adding the cover lines until all zeros are covered
		covers = CoverAllZeros(M, starred, primed, n, covers);
		if(covers == n) { break; }

		//Step 4: Finding smallest uncovered value and add it to every element of each covered row,
		// subtract it from every element of each uncovered column)
		ll min = LONG_LONG_MAX;
		for(ll i=0;i<n;i++) { for(ll j=0;j<n;j++) { if(M[GET_INDEX(i, n, n+1)] != 1 && M[GET_INDEX(n, j, n+1)] != 1 && M[GET_INDEX(i, j, n+1)] < min) { min = M[GET_INDEX(i, j, n+1)]; } } }

		for(ll i=0;i<n;i++)
		{
			if(M[GET_INDEX(i, n, n+1)] == 1)
			{
				for(ll j=0;j<n;j++) { M[GET_INDEX(i, j, n+1)] += min; }
			}
		}

		for(ll j=0;j<n;j++)
		{
			if(M[GET_INDEX(n, j, n+1)] == 0)
			{
				for(ll i=0;i<n;i++) { M[GET_INDEX(i, j, n+1)] -= min; }
			}
		}
	}

	// cleanup
	Free(primed);
	Free(M);

  // Bouml preserved body end 000C5291
}

int Algorithms::MaximumWeightAssignment(const ll numItems, ll* weight, ll* mapping)

{
  // Bouml preserved body begin 00057211

	ull byteSizeMatrix = (numItems * numItems) * sizeof(ll);
	ull byteSizeVector = numItems * sizeof(ll);

	//int weight[numItems][numItems]; Input

	//int mapping[numItems]; Output

	//int weight[numItems][numItems] input to be filled from likelihood[][]; I will change weight in this function


	// step 1: from each row subtract off the row min
	for (ll i = 0; i < numItems; i++)
	{
		ll min = weight[GET_INDEX(i, 0, numItems)];

		for (ll j = 0; j < numItems; j++)
		{
			if (weight[GET_INDEX(i, j, numItems)] < min)
			{ min = weight[GET_INDEX(i, j, numItems)]; }
		}

		for (ll j = 0; j < numItems; j++) { weight[GET_INDEX(i, j, numItems)] -= min; }
	}

	// step 2: from each column subtract off the column min
	for (ll j = 0; j < numItems; j++)
	{
		ll min = weight[GET_INDEX(0, j, numItems)];

		for (ll i = 0; i < numItems; i++)
		{
			if (weight[GET_INDEX(i, j, numItems)] < min)
			{	min = weight[GET_INDEX(i, j, numItems)]; }
		}

		for (ll i = 0; i < numItems; i++) {	weight[GET_INDEX(i, j, numItems)] -= min; }
	}

	// step 3: use as few lines as possible to cover all the zeros in the matrix
	ll* assignment = NULL;
	ll* coltick = NULL;
	ll* rowtick = NULL;
	ll* rowcovered = NULL;
	ll* colcovered = NULL;
	while (true)
	{
		//int assignment[numItems][numItems];
		assignment = (ll*)Allocate(byteSizeMatrix);
		VERIFY(assignment != NULL);
		memset(assignment, 0, byteSizeMatrix);

		// ll nassignments = 0;
		ll coveredzeros = 0;
		ll lastcoveredzeros = 0;

		//for (int i = 0; i < numItems; i++)
		//	for (int j = 0; j < numItems; j++)
		//		assignment[i][j] = 0;

		// assignment
		while (true)
		{
			// row assignment
			for (ll i = 0; i < numItems; i++)
			{
				ll assignablezeros = 0;
				ll col = -1;
				for (ll j = 0; j < numItems; j++)
				{
					if ((weight[GET_INDEX(i, j, numItems)] == 0) && (assignment[GET_INDEX(i, j, numItems)] == 0))
					{
						assignablezeros++;
						col = j;
					}
				}


				// check
				if(assignablezeros == 1)
				{
					ll zerosInCol = 0;
					for (ll k = 0; k < numItems; k++)
					{ if(weight[GET_INDEX(k, col, numItems)] == 0) { zerosInCol++; } }
					//if(zerosInCol > 1) { continue; }
				}


				if (assignablezeros == 1)
				{
					assignment[GET_INDEX(i, col, numItems)] = 1;
					coveredzeros++;

					for (ll k = 0; k < numItems; k++)
					{
						if ((k != i) && (weight[GET_INDEX(k, col, numItems)] == 0))
						{
							assignment[GET_INDEX(k, col, numItems)] = -1;
							coveredzeros++;
						}
					}
				}
			}

			// column assignment
			for (ll j = 0; j < numItems; j++)
			{
				ll assignablezeros = 0;
				ll row = -1;
				for (ll i = 0; i < numItems; i++)
				{
					if ((weight[GET_INDEX(i, j, numItems)] == 0) && (assignment[GET_INDEX(i, j, numItems)] == 0))
					{
						assignablezeros++;
						row = i;
					}
				}

				// check
				if(assignablezeros == 1)
				{
					ll zerosInRow = 0;
					for (ll k = 0; k < numItems; k++)
					{ if(weight[GET_INDEX(row, k, numItems)] == 0) { zerosInRow++; } }
					//if(zerosInRow > 1) { continue; }
				}


				if (assignablezeros == 1)
				{
					assignment[GET_INDEX(row, j, numItems)] = 1;
					coveredzeros++;

					for (ll k = 0; k < numItems; k++)
					{
						if ((k != j) && (weight[GET_INDEX(row, k, numItems)] == 0))
						{
							assignment[GET_INDEX(row, k, numItems)] = -1;
							coveredzeros++;
						}
					}
				}
			}

			if (lastcoveredzeros == coveredzeros) {	break; }

			lastcoveredzeros = coveredzeros;
		}

		// initializing the ticks

		//int coltick[numItems];
		coltick = (ll*)Allocate(byteSizeVector);
		VERIFY(coltick != NULL);

		//int rowtick[numItems];
		rowtick = (ll*)Allocate(byteSizeVector);
		VERIFY(rowtick != NULL);

		ll ticks = numItems;

		for (ll i = 0; i < numItems; i++)
		{
			coltick[i] = 0;
			rowtick[i] = 1;

			for (ll j = 0; j < numItems; j++)
			{
				if (assignment[GET_INDEX(i, j, numItems)] == 1)
				{
					rowtick[i] = 0;
					ticks--;
					break;
				}
			}
		}

		ll oldticks = ticks;

		// ticking
		while (true)
		{
			// ticking all columns that have a zero in any ticked row
			for (ll i = 0; i < numItems; i++)
			{
				if (rowtick[i])
				{
					for (ll j = 0; j < numItems; j++)
					{
						if ((weight[GET_INDEX(i, j, numItems)] == 0) && (coltick[j] == 0))
						{
							coltick[j] = 1;
							ticks++;
						}
					}
				}
			}

			// ticking all rows having any assignment in ticked columns
			for (ll j = 0; j < numItems; j++)
			{
				if (coltick[j])
				{
					for (ll i = 0; i < numItems; i++)
					{
						if ((weight[GET_INDEX(i, j, numItems)] == 0) && (assignment[GET_INDEX(i, j, numItems)] == 1) && (rowtick[i] == 0))
						{
							rowtick[i] = 1;
							ticks++;
						}
					}
				}
			}

			if (oldticks == ticks) { break; }

			oldticks = ticks;
		}

		ll lines = 0;

		//int rowcovered[numItems];
		rowcovered = (ll*)Allocate(byteSizeVector);
		VERIFY(rowcovered != NULL);

		//int colcovered[numItems];
		colcovered = (ll*)Allocate(byteSizeVector);
		VERIFY(colcovered != NULL);

		for (ll k = 0; k < numItems; k++)
		{
			rowcovered[k] = 1 - (rowtick[k] == 1);
			colcovered[k] = coltick[k];

			lines += (rowcovered[k] == 1) + (colcovered[k] == 1);
		}

		if (lines == numItems) { break; }

		// find the minimum weight[i][j] among uncovered cells: min
		// subtract min from every uncovered cell
		// add min to every cell covered with two lines

		ll min = LONG_LONG_MAX;

		for (ll i = 0; i < numItems; i++)
		{
			if (!rowcovered[i])
			{
				for (ll j = 0; j < numItems; j++)
				{
					if (!colcovered[j])
					{
						if (weight[GET_INDEX(i, j, numItems)] < min)
						{	min = weight[GET_INDEX(i, j, numItems)]; }
					}
				}
			}
		}

		for (ll i = 0; i < numItems; i++)
		{
			for (ll j = 0; j < numItems; j++)
			{
				if ((rowcovered[i]) && (colcovered[j]))
				{	weight[GET_INDEX(i, j, numItems)] += min; }
				if ((!rowcovered[i]) && (!colcovered[j]))
				{	weight[GET_INDEX(i, j, numItems)] -= min; }
			}
		}

		Free(assignment); assignment = NULL;
		Free(coltick); coltick = NULL;
		Free(rowtick); rowtick = NULL;
		Free(rowcovered); rowcovered = NULL;
		Free(colcovered); colcovered = NULL;
	}

	// we may have to free here as well because of the 'break' from the while(true);
	if(assignment != NULL) { Free(assignment); }
	if(coltick != NULL) { Free(coltick); }
	if(rowtick != NULL) { Free(rowtick); }
	if(rowcovered != NULL) { Free(rowcovered); }
	if(colcovered != NULL) { Free(colcovered); }

	// step 4: making assignments, deleting rows and columns
	ll* mapped = NULL;
	{
		//int assignment[numItems][numItems];
		//memset(assignment, 0, sizeof(assignment));
		assignment = (ll*)Allocate(byteSizeMatrix);
		VERIFY(assignment != NULL);
		memset(assignment, 0, byteSizeMatrix);

		ll matchings = 0;
		ll oldmatchings = 0;

		ll arbitrary = 0;

		ll iterations = 2*numItems;

		// assignment

		//	while (true)
		while(iterations > 0)
		{
			iterations--;

			// row assignment
			for (ll i = 0; i < numItems; i++)
			{
				ll assignablezeros = 0;
				ll col = -1;
				for (ll j = 0; j < numItems; j++)
				{
					if ((weight[GET_INDEX(i, j, numItems)] == 0) && (assignment[GET_INDEX(i, j, numItems)] == 0))
					{
						assignablezeros++;
						col = j;
					}
				}

				if ((assignablezeros == 1) || ( (assignablezeros > 1) && (arbitrary == 1) ))
				{
					assignment[GET_INDEX(i, col, numItems)] = 1;
					matchings++;

					for (ll k = 0; k < numItems; k++)
					{
						if ((k != i) && (weight[GET_INDEX(k, col, numItems)] == 0))
							assignment[GET_INDEX(k, col, numItems)] = -1;

						if (arbitrary == 1)
							if ((k != col) && (weight[GET_INDEX(i, k, numItems)] == 0))
								assignment[GET_INDEX(i, k, numItems)] = -1;
					}
					arbitrary = 0;
				}
			}

			// column assignment
			for (ll j = 0; j < numItems; j++)
			{
				ll assignablezeros = 0;
				ll row = -1;
				for (ll i = 0; i < numItems; i++)
					if ((weight[GET_INDEX(i, j, numItems)] == 0) && (assignment[GET_INDEX(i, j, numItems)] == 0))
					{
						assignablezeros++;
						row = i;
					}

				if ((assignablezeros == 1) || ( (assignablezeros > 1) && (arbitrary == 1) ))
				{
					assignment[GET_INDEX(row, j, numItems)] = 1;
					matchings++;

					for (ll k = 0; k < numItems; k++)
					{
						if ((k != j) && (weight[GET_INDEX(row, k, numItems)] == 0))
							assignment[GET_INDEX(row, k, numItems)] = -1;

						if (arbitrary == 1)
							if ((k != row) && (weight[GET_INDEX(k, j, numItems)] == 0))
								assignment[GET_INDEX(k, j, numItems)] = -1;
					}
					arbitrary = 0;
				}
			}

			if (matchings == numItems) { break; }

			if (oldmatchings == matchings) { arbitrary = 1; }

			oldmatchings = matchings;
		}


		//int mapped[numItems];
		mapped = (ll*)Allocate(byteSizeVector);
		VERIFY(mapped != NULL);

		for (ll i = 0; i < numItems; i++)
		{
			mapped[i] = -1;
			mapping[i] = -1;
		}

		ll sumi = 0, sumj = 0;

		for (ll i = 0; i < numItems; i++)
		{
			sumi += i;
			for (ll j = 0; j < numItems; j++)
				if ( (mapped[j] == -1) && (assignment[GET_INDEX(i, j, numItems)] == 1) )
				{
					sumj += j;
					mapping[i] = j;
					mapped[j] = i;
					break;
				}
		}

		// fixing the unassigned nodes
		if ((sumi - sumj) != 0)
		{
			ll nonopt = 0;

			for (ll i = 0; i < numItems; i++)
			{
				if (mapping[i] == -1)
				{
					ll min = LONG_LONG_MAX;
					ll index = numItems+1;

					for (ll j = 0; j < numItems; j++)
						if ((mapped[j] == -1)&&(weight[GET_INDEX(i, j, numItems)] < min))
						{
							min = weight[GET_INDEX(i, j, numItems)];
							index = j;
						}

					mapping[i] = index;
					mapped[index] = i;
					nonopt++;
				}
			}

			for (ll i = 0; i < numItems; i++) { VERIFY(mapping[i] != -1); }

			ll sumi = 0, sumj = 0;

			for (ll i = 0; i < numItems; i++)
			{
				sumi += i;
				sumj += mapping[i];
			}


			//"Number of non-optimal assignments: " nonopt

			if ((sumi - sumj) != 0)
			{
				for (ll i = 0; i < numItems; i++)
				{
					stringstream info("");
					info << "mapping: " << i << " : " << mapping[i] << ", mapped: " << mapped[i] << ", assignment: " << assignment[i];
					Log::GetInstance()->Append(info.str());
				}
			}

			Free(assignment); assignment = NULL;
			Free(mapped); mapped = NULL;

			VERIFY((sumi - sumj) == 0);
		}

	}

	ll unassigned = 0; ll firstUnassigned = -1;
	for (ll i = 0; i < numItems; i++)
	{
		if(mapping[i] == -1)
		{
			unassigned++;
			if(firstUnassigned == -1) { firstUnassigned = i; }
		}
		VERIFY(mapping[i] >= -1 && mapping[i] < numItems);
	}

	if(unassigned > 0)
	{
		VERIFY(unassigned == 1); // if we have more than 1 item unassigned we have a coding error!

		ll* seen = (ll*)Allocate(byteSizeVector);
		VERIFY(seen != NULL); memset(seen, 0, byteSizeVector);
		for(ll i = 0; i < numItems; i++)
		{
			if(mapping[i] != -1)
			{
				seen[mapping[i]] = 1;
			}
		}

		ll unassignedItem = -1;
		for(ll i = 0; i < numItems; i++)
		{
			if(seen[i] == 0) { unassignedItem = i; break; }
		}

		mapping[firstUnassigned] = unassignedItem;

		Free(seen);
	}

	for (ll i = 0; i < numItems; i++) { VERIFY(mapping[i] >= 0 && mapping[i] < numItems); }

	// we may have to free here as well because of the 'break'
	if(assignment != NULL) { Free(assignment); }
	if(mapped != NULL) { Free(mapped); }

	return 0;

  // Bouml preserved body end 00057211
}

void Algorithms::MultiplySquareMatrices(const double* leftMatrix, const double* rightMatrix, ull dimension, double* resultMatrix)

{
  // Bouml preserved body begin 00081A91

	VERIFY(leftMatrix != NULL && rightMatrix != NULL && resultMatrix != NULL && dimension != 0);

	for(ull i = 0; i < dimension; i++)
	{
		for(ull j = 0; j < dimension; j++)
		{
			double sum = 0.0;
			for(ull k = 0; k < dimension; k++)
			{
				ull leftIndex = GET_INDEX(i, k, dimension);
				ull rightIndex = GET_INDEX(k, j, dimension);
				sum += leftMatrix[leftIndex] * rightMatrix[rightIndex];
			}

			ull resIndex = GET_INDEX(i, j, dimension);
			resultMatrix[resIndex] = sum;
		}
	}

  // Bouml preserved body end 00081A91
}

bool Algorithms::GetSteadyStateVectorOfSubChain(double* fullChainSS, ull timePeriodId, double** subChainSS, bool inclDummyTPs)
{
  // Bouml preserved body begin 000ADF91

	if(fullChainSS == NULL || subChainSS == NULL) { return false; }

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(Parameters::GetInstance()->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	if(inclDummyTPs == true) { numPeriods = tpInfo.numPeriodsInclDummies; }
	ull maxPeriod = minPeriod + numPeriods - 1;

	if(timePeriodId < minPeriod || timePeriodId > maxPeriod) { return false; }

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// allocated here, but freed by the caller
	ull resVectorByteSize = numLoc * sizeof(double);
	double* resVector = (double*)Allocate(resVectorByteSize);
	memset(resVector, 0, resVectorByteSize);

	// extract the subvector
	ull startState = (timePeriodId - minPeriod)*numLoc; double sum = 0.0;
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		ull locIndex = (loc - minLoc);
		resVector[locIndex] = fullChainSS[startState + locIndex];
		sum += resVector[locIndex];
	}
	VERIFY(sum != 0.0);

	// renormalize
	double check = 0.0;
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		ull locIndex = (loc - minLoc);
		resVector[locIndex] /= sum;
		check += resVector[locIndex];
	}
	VERIFY(abs(check-1) < EPSILON); // sanity check


	*subChainSS = resVector;


	return true;

  // Bouml preserved body end 000ADF91
}

bool Algorithms::GetTransitionVectorOfSubChain(double* fullChainTransitionMatrix, ull tp1, ull loc1, ull tp2, double** transitionVector, bool inclDummyTPs)

{
  // Bouml preserved body begin 000AF911

	if(fullChainTransitionMatrix == NULL || transitionVector == NULL) { return false; }

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(Parameters::GetInstance()->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	if(inclDummyTPs == true) { numPeriods = tpInfo.numPeriodsInclDummies; }
	ull maxPeriod = minPeriod + numPeriods - 1;

	if(tp1 < minPeriod || tp1 > maxPeriod || tp2 < minPeriod || tp2 > maxPeriod) { return false; }

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(Parameters::GetInstance()->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	VERIFY(loc1 >= minLoc && loc1 <= maxLoc);

	ull numStates = numPeriods * numLoc;

	// allocated here, but freed by the caller
	ull resVectorByteSize = numLoc * sizeof(double);
	double* resVector = (double*)Allocate(resVectorByteSize);
	memset(resVector, 0, resVectorByteSize);

	// extract the subvector
	ull currentState = (tp1 - minPeriod)*numLoc + (loc1 - minLoc); double sum = 0.0;
	for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
	{
		ull locIdx = (loc2 - minLoc);
		ull tpIdx = (tp2 - minPeriod);
		ull nextState = tpIdx * numLoc + locIdx;
		resVector[locIdx] = fullChainTransitionMatrix[GET_INDEX(currentState, nextState, numStates)];
		sum += resVector[locIdx];
	}
	VERIFY(sum != 0.0);

	// renormalize
	double check = 0.0;
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		ull locIdx = (loc - minLoc);
		resVector[locIdx] /= sum;
		check += resVector[locIdx];
	}
	VERIFY(abs(check-1) < EPSILON); // sanity check


	*transitionVector = resVector;

	return true;

  // Bouml preserved body end 000AF911
}


} // namespace lpm
