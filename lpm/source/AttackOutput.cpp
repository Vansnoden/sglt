//!
//! \file
//!
#include "../include/AttackOutput.h"
#include "../include/ObservedEvent.h"

namespace lpm {

AttackOutput::AttackOutput() 
{
  // Bouml preserved body begin 00094411

	probabilityDistribution = NULL;
	anonymizationMap = map<ull, ull>();
	mostLikelyTrace = NULL;
	mostLikelyTraceLL = NULL;

	genericRecSigma = NULL;
	genericRecDistribution = NULL;
	genericRecPseudonyms = NULL;
	genericRecPseudonymsSet = set<ull>();

  // Bouml preserved body end 00094411
}

AttackOutput::~AttackOutput() 
{
  // Bouml preserved body begin 00094491

	if(probabilityDistribution != NULL) { Free(probabilityDistribution); }
	anonymizationMap.clear();
	if(mostLikelyTrace != NULL) { Free(mostLikelyTrace); }
	if(mostLikelyTraceLL != NULL) { Free(mostLikelyTraceLL); }

	if(genericRecSigma != NULL) { Free(genericRecSigma); }
	if(genericRecDistribution != NULL) { Free(genericRecDistribution); }
	if(genericRecPseudonyms != NULL) { Free(genericRecPseudonyms); }

	genericRecPseudonymsSet.clear();

  // Bouml preserved body end 00094491
}

bool AttackOutput::GetProbabilityDistribution(double** output) const 
{
  // Bouml preserved body begin 00055811

	if(output == NULL) { return false; }

	*output = probabilityDistribution;

	return true;

  // Bouml preserved body end 00055811
}

void AttackOutput::SetProbabilityDistribution(double* prob) 
{
  // Bouml preserved body begin 00055891

	DEBUG_VERIFY(prob != NULL);
	probabilityDistribution = prob;

  // Bouml preserved body end 00055891
}

void AttackOutput::GetAnonymizationMap(map<ull, ull>& map) const 
{
  // Bouml preserved body begin 00076291

	map = anonymizationMap;

  // Bouml preserved body end 00076291
}

void AttackOutput::SetAnonymizationMap(const map<ull, ull>& map) 
{
  // Bouml preserved body begin 00076311

	anonymizationMap = map;

  // Bouml preserved body end 00076311
}

bool AttackOutput::GetMostLikelyTrace(ull** output) const 
{
  // Bouml preserved body begin 0007CA11

	if(output == NULL) { return false; }

	*output = mostLikelyTrace;

	return true;

  // Bouml preserved body end 0007CA11
}

void AttackOutput::SetMostLikelyTrace(const ull* trace) 
{
  // Bouml preserved body begin 0007CA91

	DEBUG_VERIFY(trace != NULL);
	mostLikelyTrace = const_cast<ull*>(trace);

  // Bouml preserved body end 0007CA91
}

bool AttackOutput::GetMostLikelyTraceLL(double** output) const
{
	if(output == NULL) { return false; }

	*output = mostLikelyTraceLL;

	return true;
}

void AttackOutput::SetMostLikelyTraceLL(const double* ll)
{
	DEBUG_VERIFY(ll != NULL);
	mostLikelyTraceLL = const_cast<double*>(ll);
}

bool AttackOutput::GetGenericReconstructionObjects(double** distribution, double** sigma, double** pseudonyms, set<ull>& pseudonymsSet) const 
{
  // Bouml preserved body begin 000D0D11

	if(distribution == NULL && sigma == NULL && pseudonyms == NULL) { return false; }

	if(distribution != NULL) { *distribution = genericRecDistribution; }
	if(sigma != NULL) { *sigma = genericRecSigma; }
	if(pseudonyms != NULL) { *pseudonyms = genericRecPseudonyms; }
	pseudonymsSet = genericRecPseudonymsSet;

	return true;

  // Bouml preserved body end 000D0D11
}

void AttackOutput::SetGenericReconstructionObjects(double* distribution, double* sigma, double* pseudonyms, set<ull>& pseudonymsSet) 
{
  // Bouml preserved body begin 000D0D91

	DEBUG_VERIFY(distribution != NULL);
	DEBUG_VERIFY(sigma != NULL);
	DEBUG_VERIFY(pseudonyms != NULL);
	DEBUG_VERIFY(pseudonymsSet.size() > 0);

	genericRecDistribution = distribution;
	genericRecSigma = sigma;
	genericRecPseudonyms = pseudonyms;
	genericRecPseudonymsSet = pseudonymsSet;


  // Bouml preserved body end 000D0D91
}

bool AttackOutput::GetObservedMatrix(ObservedEvent** matrix) const 
{
  // Bouml preserved body begin 000DAD91

	if(matrix == NULL) { return false; }

	*matrix = (ObservedEvent*)observedMatrix; // note: matrix is in fact treated as an ObservedEvent*** !

	return true;

  // Bouml preserved body end 000DAD91
}

void AttackOutput::SetObservedMatrix(ObservedEvent** matrix) 
{
  // Bouml preserved body begin 000DAD11

	observedMatrix = const_cast<ObservedEvent**>(matrix);

  // Bouml preserved body end 000DAD11
}


} // namespace lpm
