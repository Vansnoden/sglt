#include "include/Public.h" // <- LPM public header (note: this code used a modified version of LPM).

// The following files implement the logic of the generation of synthetic traces.
// These are named (and implemented) as LPPM, Attack, and Metric respectively, but
// this to leverage the infrastructure provided by LPM. Logically, all three components are
// part of a single synthetic location traces generative process.
#include "SGLPPMOperation.h"
#include "SGAttackOperation.h"

#include "SGMetric.h"

using namespace lpm;


bool ConstructKnowledge(string& traceFilePath, string& mobilityFilePath, string& knowledgeFilePath)
{
	// test if the output file exists, if so there is no need to re-create it...
	{
		File outputKCFileExists(knowledgeFilePath, true);
		if(outputKCFileExists.IsGood() == true) { return true; }
	}

	LPM* lpm = LPM::GetInstance();

	File learningTraceFile(traceFilePath, true);
	File mobilityFile(mobilityFilePath, true);
	File outputKC(knowledgeFilePath, false);

	KnowledgeInput knowledge;
	knowledge.transitionsFeasibilityFile = &mobilityFile;
	knowledge.transitionsCountFile = NULL;
	knowledge.learningTraceFilesVector = vector<File*>();
	knowledge.learningTraceFilesVector.push_back(&learningTraceFile);

	const ull maxGSIterations = 100000;
	const ull maxSeconds = 60;

	if(lpm->RunKnowledgeConstruction(&knowledge, &outputKC, maxGSIterations, maxSeconds) == false)
	{
		std::cout << Errors::GetInstance()->GetLastErrorMessage() << endl;
		return false;
	}
	return true;
}


bool ComputeAggregateStats(string& traceFilePath, string& locationsFilePath, string& aggregateStatsFilePath)
{
	// test if the output file exists, if so there is no need to re-create it...
	{
		File aggregateStatsFileExists(aggregateStatsFilePath, true);
		if(aggregateStatsFileExists.IsGood() == true)
		{
			Log::GetInstance()->Append("Aggregate statistics file found.");
			return true;
		}
	}

	Log::GetInstance()->Append("Computing aggregate statistics...");

	File learningTraceFile(traceFilePath, true);
	File aggregateStatsFile(aggregateStatsFilePath, false);

	double* steadyStateVector = NULL; double* transitionMatrix = NULL;

	File locationsFile(locationsFilePath, true);

	CreateContextOperation* createContextOp = new CreateContextOperation();
	if(createContextOp->ComputeAggregateStatistics(&learningTraceFile, &locationsFile, &transitionMatrix, &steadyStateVector) == false) { return false; }
	createContextOp->Release();


	// get user parameters
	ull minUserID = 1; ull maxUserID = Parameters::GetInstance()->GetUsersCount();

	// for the attack to be able to use the aggregate statistics profile later, it needs to be in the proper form
	// so, for each user we create a profile and populate it with the aggregate stats' steady-state vector and transition matrix
	Context* context = new Context();
	for(ull userID = minUserID; userID <= maxUserID; userID++)
	{
		UserProfile* profile = new UserProfile(userID);
		profile->SetTransitionMatrix(transitionMatrix);
		profile->SetSteadyStateVector(steadyStateVector);

		profile->SetAccuracyInfo(0, NULL);
		context->AddProfile(profile);
	}

	StoreContextOperation* storeContextOp = new StoreContextOperation();
	if(storeContextOp->Execute(context, &aggregateStatsFile) == false) { return false; }
	storeContextOp->Release();

	for(ull userID = minUserID; userID <= maxUserID; userID++)
	{
		UserProfile* profile = NULL;
		context->GetUserProfile(userID, &profile);

		profile->SetSteadyStateVector(NULL); profile->SetTransitionMatrix(NULL);
	}
	Free(steadyStateVector); Free(transitionMatrix);

	context->Release();

	Log::GetInstance()->Append("Done with the computation of aggregate statistics.");

	return true;
}

/*
 * There are two adjustments which users of this code may want to make.
 * The first is to choose between options (1) and (2) -- see below.
 * The second is to adjust the paths related to CLUTO, in particular: CLUTO_SCRIPT_REL_PATH.
 */
bool ClusterLocations(string& outputDir, string& knowledgeFilePath, string& locClustersFilePath, MetricDistance* distanceFunction = new DefaultMetricDistance())
{
	// test if the output file exists, if so there is no need to re-create it...
	{
		File locClustersFileExists(locClustersFilePath, true);
		if(locClustersFileExists.IsGood() == true)
		{
			Log::GetInstance()->Append("Locations clusters file found.");
			return true;
		}
	}


	Log::GetInstance()->Append("Starting clustering locations...");

	File knowledgeFile(knowledgeFilePath, true);
	File locClustersFile(locClustersFilePath, false);

	string clutoGraphFP = outputDir + "/" "cluto.graph";
	File textGraphTmp(clutoGraphFP, false);

	File* output = &locClustersFile;
	LoadContextOperation* loadContextOp = new LoadContextOperation();

	Context* context = new Context();
	bool ok = loadContextOp->Execute(&knowledgeFile, context); loadContextOp->Release();
	if(ok == false) { context->Release(); return false; }

	Log::GetInstance()->Append("Context loaded, calculating weights for clustering locations.");

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	ull numLoc = maxLoc - minLoc + 1;

	// get time period parameters
	ull numPeriods = 0; TPInfo tpInfo;
	VERIFY(params->GetTimePeriodInfo(&numPeriods, &tpInfo) == true);
	ull minPeriod = tpInfo.minPeriod;
	ull maxPeriod = tpInfo.maxPeriod;


	// create the location similarity matrix
	ull locSimMatrixByteSize = numLoc * numLoc * sizeof(double);
	double* locSimMatrix = (double*)Allocate(locSimMatrixByteSize);
	VERIFY(locSimMatrix != NULL); memset(locSimMatrix, 0, locSimMatrixByteSize);

	// set every element of the matrix to epsilon, so we don't have the issue that some pairs of locations have weight 0
	// this ensures the clustering will run smoothly
	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
		{
			ull matrixIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);
			locSimMatrix[matrixIdx] = EPSILON * EPSILON;
		}
	}

	map<ull, UserProfile*> profiles = map<ull, UserProfile*>();
	VERIFY(context->GetProfiles(profiles) == true);

	bool isDefaultDistance = (dynamic_cast<DefaultMetricDistance*>(distanceFunction) != NULL);

	pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles)
	{
		ull user1 = iterProfiles->first;
		UserProfile* profile1 = iterProfiles->second;

		double* steadyStateVector1 = NULL;
		double* transitionMatrix1 = NULL;

		VERIFY(profile1->GetSteadyStateVector(&steadyStateVector1) == true);
		VERIFY(profile1->GetTransitionMatrix(&transitionMatrix1) == true);


		pair_foreach_const(map<ull, UserProfile*>, profiles, iterProfiles2)
		{
			ull user2 = iterProfiles2->first;
			UserProfile* profile2 = iterProfiles2->second;

			double* steadyStateVector2 = NULL;
			double* transitionMatrix2 = NULL;

			VERIFY(profile2->GetSteadyStateVector(&steadyStateVector2) == true);
			VERIFY(profile2->GetTransitionMatrix(&transitionMatrix2) == true);

			double sim0 = 0.0;
			/*
			{
				Log::GetInstance()->Append("#########################################\n");
				stringstream info("");
				info << "Computation for the pair: " <<  user1 << ", " << user2;
				Log::GetInstance()->Append(info.str());
			}
			*/


/*
 * There are two ways to compute the locations semantic graphs:
 * (1) use the best mapping (will always work), and
 * (2) use explicit per loc pair score (only makes sense for zeroth order, requires not defining ONLY_BEST_MAPPING)
 *
 * Both options should give good (and similar) results in most cases.
 * (However, this fact has not been comprehensively experimentally tested.)
 */

// Comment out (i.e., undefine) the following for option (2).
#define ONLY_BEST_MAPPING 1

			//if(user1 == user2) { ; }
			//else
			{
#ifdef ONLY_BEST_MAPPING
				// zeroth-order
				ull sigmasArrayByteSize = numPeriods * sizeof(ll*);
				ll** sigmasArray = (ll**)Allocate(sigmasArrayByteSize);
				VERIFY(sigmasArray != NULL); memset(sigmasArray, 0, sigmasArrayByteSize);

				for(ull tp = minPeriod; tp <= maxPeriod; tp++)
				{
					// Compute the best sigma
					ull sigmaByteSize = numLoc * sizeof(ll);
					ll* sigma = (ll*)Allocate(sigmaByteSize);
					VERIFY(sigma != NULL); memset(sigma, 0, sigmaByteSize);

					ull wdByteSize = numLoc * numLoc * sizeof(double);
					double* wd = (double*)Allocate(wdByteSize);
					VERIFY(wd != NULL); memset(wd, 0, wdByteSize);

					// fill in the weight matrix
					double maxVal = 0.0; double maxOverlapProb = 0.0;
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						//stringstream info(""); info.precision(5);

						ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
						for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
						{
							ull idx2 = GET_INDEX((tp - minPeriod), (loc2 - minLoc), numLoc);

							ull wIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);
							double w = min(steadyStateVector1[idx1], steadyStateVector2[idx2]);

							if(w > maxOverlapProb) { maxOverlapProb = w; }

							//info << w << (loc2 < maxLoc ? ", " : "");

							w = log2(w);
							wd[wIdx] = w;

							double mw = -w;
							if(mw > maxVal) { maxVal = mw; } // keep track of max
						}

						//Log::GetInstance()->Append(info.str());
					}

					ull costMatrixByteSize = numLoc * numLoc * sizeof(ll);
					ll* costMatrix = (ll*)Allocate(costMatrixByteSize);
					VERIFY(costMatrix != NULL); memset(costMatrix, 0, costMatrixByteSize);

					//Log::GetInstance()->Append("***********************");

					// make the probability an integer: multiply by 1/minVal
					const ll bigNumber = (maxVal == 0.0) ? -1 : (ll)(-((double)(1000000/2.0)/maxVal) + 10);
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						//stringstream info(""); info.precision(5);

						for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
						{
							ull wIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);

							// Note: we are transforming the maximization problem (maximum weight assignment) in a minimization problem (minimum cost assignment)
							ll c = wd[wIdx] > 0 ? 0 : (ll)(wd[wIdx]*bigNumber);
							costMatrix[wIdx] = c;

							//info << c << (loc2 < maxLoc ? ", " : "");
						}

						//Log::GetInstance()->Append(info.str());
					}
					Free(wd); wd = NULL; // free wd
					//Log::GetInstance()->Append("------------");

					Algorithms::MinimumCostAssignment(costMatrix, numLoc, sigma); // Hungarian (munkres) algorithm to find sigma
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						//stringstream info("");
						VERIFY(sigma[loc - minLoc] >= 0 && sigma[loc - minLoc] < (ll)numLoc);
						//info << "Assignment: loc u1 -> loc u2: " << loc << " -> " << sigma[loc - minLoc] + minLoc;
						//Log::GetInstance()->Append(info.str());
					}

					Free(costMatrix); costMatrix = NULL; // free costMatrix

					// finally compute the similarity value according to sigma
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
						ull idx2 = GET_INDEX((tp - minPeriod), sigma[(loc - minLoc)], numLoc);

						sim0 += min(steadyStateVector1[idx1], steadyStateVector2[idx2]);
					}


					sigmasArray[tp - minPeriod] = sigma; // save sigma;
				}
#endif

				for(ull tp = minPeriod; tp <= maxPeriod; tp++)
				{
					// now that we have the best mapping, instead of computing the similarity score,
					// we use the mapping information to update the location similarity matrix
					// that will be the basis of clustering

					// basically there are two ways to do:
					// (1) we only consider the locations part of the mapping (and weight based on the users similarity), or
					// (2) we consider all pairs of locations (note we can do this second thing efficiently only for zeroth order)


#ifdef ONLY_BEST_MAPPING
					ll* sigma = sigmasArray[tp - minPeriod];
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull loc2idx = sigma[(loc - minLoc)];

						ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);
						ull idx2 = GET_INDEX((tp - minPeriod), loc2idx, numLoc);

						ull matrixIdx = GET_INDEX((loc - minLoc), loc2idx, numLoc);

						double score = min(steadyStateVector1[idx1], steadyStateVector2[idx2]);
						locSimMatrix[matrixIdx] += score * sim0;
					}
#else
					for(ull loc = minLoc; loc <= maxLoc; loc++)
					{
						ull idx1 = GET_INDEX((tp - minPeriod), (loc - minLoc), numLoc);

						for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
						{
							ull idx2 = GET_INDEX((tp - minPeriod), (loc2 - minLoc), numLoc);

							ull matrixIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);

							double score = min(steadyStateVector1[idx1], steadyStateVector2[idx2]);
							locSimMatrix[matrixIdx] += score;


						}
					}
#endif
#ifdef ONLY_BEST_MAPPING
					if(sigma != NULL) { Free(sigma); sigmasArray[tp - minPeriod] = NULL;  }
#endif
				}
#ifdef ONLY_BEST_MAPPING
				if(sigmasArray != NULL) { Free(sigmasArray); }
#endif
			}

			/*
			stringstream ss("");
			ss << user1 << DEFAULT_FIELDS_DELIMITER << " " << user2 << ": "; // leader, follower
			ss << sim0 << DEFAULT_FIELDS_DELIMITER << " " << sim1;
			output->WriteLine(ss.str());
			*/
		}
	}


	stringstream ss1(""); ss1 << numLoc; // the first row of the graph for cluto just contains the number of objects
	textGraphTmp.WriteLine(ss1.str());

	for(ull loc = minLoc; loc <= maxLoc; loc++)
	{
		stringstream ss("");
		for(ull loc2 = minLoc; loc2 <= maxLoc; loc2++)
		{
			ull matrixIdx = GET_INDEX((loc - minLoc), (loc2 - minLoc), numLoc);

			double score = locSimMatrix[matrixIdx];

			ss << score; if(loc2 != maxLoc) { ss << ' '; }
		}
		textGraphTmp.WriteLine(ss.str());
	}

	context->Release();

	Log::GetInstance()->Append("Similarity graph constructed, starting clustering (CLUTO)...");

#define CLUTO_SCRIPT_REL_PATH "../../../cluto/do-cluto.sh"
	string scriptFP = CLUTO_SCRIPT_REL_PATH;
	//string scriptFP = "./cluto/do-cluto.sh";

	stringstream ss(""); ss << scriptFP << " " << textGraphTmp.GetFilePath();

	Log::GetInstance()->Append(string("Invoking: " + ss.str()));
	int exitCode = system(ss.str().c_str());
	VERIFY(exitCode == 0);

	File outputGraphTmp(clutoGraphFP + ".clustering", true);
	VERIFY(outputGraphTmp.IsGood() == true);

	LineParser<ull>* parser = LineParser<ull>::GetInstance();
	vector<set<ull> > clusterVec;

	ull loc = minLoc;
	while(outputGraphTmp.IsGood())
	{
		string line = "";

		bool readOk = outputGraphTmp.ReadNextLine(line);
		VERIFY(readOk == true);

		if(line.empty() == true && outputGraphTmp.IsGood() == false) { break; }

		ull clusterIdx = 0; size_t pos = 0;
		bool parseOk = parser->ParseValue(line, &clusterIdx, &pos);
		VERIFY(parseOk == true && pos == string::npos);

		while(clusterVec.size() <= clusterIdx) { clusterVec.push_back(set<ull>()); }
		clusterVec[clusterIdx].insert(loc);

		{
			stringstream ssl(""); ssl << "(Clustering) loc: " << loc << " -> clusterIdx: " << clusterIdx;
			Log::GetInstance()->Append(ssl.str());
		}

		loc++;
	}
	VERIFY(loc == maxLoc+1);

	ull clusterIdx = 0;
	foreach_const(vector<set<ull> >, clusterVec, iter)
	{
		stringstream ss("");
		//ss << clusterIdx << ": "; // we make the clusterIdx implicit
		bool empty = true;
		const set<ull>& thisSet = *iter;
		foreach_const(set<ull>, thisSet, iterSet)
		{
			if(iterSet != thisSet.begin()) { ss << DEFAULT_FIELDS_DELIMITER; }
			ss << (*iterSet);

			empty = false;
		}

		if(empty == false)
		{
			stringstream ssl(""); ssl << "Cluster " << clusterIdx << ": " << ss.str();
			Log::GetInstance()->Append(ssl.str());

			output->WriteLine(ss.str());
			clusterIdx++;
		}
	}

	Log::GetInstance()->Append("Clustering done.");

	return true;
}

/*
 * Part of the synthetic trajectories generative model is implemented in SGLPPMOperation.
 * Note that LPPMs is what LPM uses to obfuscate locations. Here, however, SGLPPMOperation
 * is not an obfuscation mechanism (but is implemented as one to leverage the infrastructure provided by LPM) but
 * part of the synthetic location trace generation process.
 */
bool RunSGLPPM(string& outputDir, string& traceFilePath, string& knowledgeFilePath, string& aggregateStatsFilePath,
				string& locClustersFilePath, double removeProp, double mergeProp, double removeActualLocProb, LPPMOperation** lppmOp)
{
	LPM* lpm = LPM::GetInstance();

	Parameters* params = Parameters::GetInstance();

	// get location parameters
	ull minLoc = 0; ull maxLoc = 0;
	VERIFY(params->GetLocationstampsRange(&minLoc, &maxLoc) == true);
	//ull numLoc = maxLoc - minLoc + 1;

	File traceFile(traceFilePath, true);
	File knowledgeFile(knowledgeFilePath, true);

	File aggregateStatsFile(aggregateStatsFilePath, true);
	File locClustersFile(locClustersFilePath, true);

	ScheduleBuilder* builder = new ScheduleBuilder("Run SGLPPM schedule");

	VERIFY(builder->SetInputs(&knowledgeFile) == true);

	// the app exposes all events
	ApplicationOperation* app = new DefaultApplicationOperation(1.0, Basic);
	VERIFY(builder->SetApplicationOperation(app) == true);
	app->Release();

	// read in the clusters, then subsample some locs in each
    vector<set<ull> > clustersVec;
    map<ull, ull> locToClusterMap;

    LineParser<ull>* parser = LineParser<ull>::GetInstance();

    ull clusterIdx = 0;
	while(locClustersFile.IsGood())
	{
		string line = "";

		bool readOk = locClustersFile.ReadNextLine(line);
		VERIFY(readOk == true);

		if(line.empty() == true && locClustersFile.IsGood() == false) { break; }

		vector<ull> locs; size_t pos = 0;
		bool parseOk = parser->ParseFields(line, locs, ANY_NUMBER_OF_FIELDS, &pos);
		VERIFY(parseOk == true && pos == string::npos);

		clustersVec.push_back(set<ull>());
		clustersVec[clusterIdx].insert(locs.begin(), locs.end());

		foreach_const(vector<ull>, locs, iter) { locToClusterMap.insert(make_pair(*iter, clusterIdx)); }

		clusterIdx++;
	}

	LPPMOperation* lppm = *lppmOp = new SGLPPMOperation(traceFile, locToClusterMap, clustersVec, removeProp, mergeProp, removeActualLocProb);
	VERIFY(builder->SetLPPMOperation(lppm) == true);
	// lppm->Release(); // don't release it here, we do it later

	VERIFY(builder->InsertOutputOperation() == true); // insert intermediary output

	Schedule* schedule = builder->GetSchedule();
	VERIFY(schedule != NULL);

	delete builder;

	VERIFY(traceFile.Rewind() == true);

	string outputPrefix = outputDir + "/" "output";
	if(lpm->RunSchedule(schedule , &traceFile, outputPrefix.c_str()) == false) // run the schedule
	{
		std::cout << Errors::GetInstance()->GetLastErrorMessage() << endl ; // print the error message
		return false;
	}

	schedule->Release(); // release the schedule object (since it is no longer needed)

	return true;
}

bool RunViterbi(string& outputDir, string& traceFilePath, string& observedTraceFilePath,
		string& aggregateStatsFilePath, string& locClustersFilePath, double multFactor, LPPMOperation* lppm)
{
	LPM* lpm = LPM::GetInstance();

	File knowledgeFile(aggregateStatsFilePath, true); // use aggregate statistics as the mobility profile

	// create instances of the application and use the instance of the LPPM used, so that we can retrieve their PDFs.
	// note that in this case, we must *not* release ownership of these objects before we're done running the schedule
	ApplicationOperation* app = new DefaultApplicationOperation(1.0, Basic); // the app exposes all events

	// retrieve the application and LPPM PDFs
	FilterFunction* applicationPDF = dynamic_cast<FilterFunction*>(app);
	FilterFunction* lppmPDF = dynamic_cast<FilterFunction*>(lppm);

	SchedulePosition startPos = ScheduleInvalidPosition;
	File actualTraceFile(traceFilePath);

	ScheduleBuilder* builder = new ScheduleBuilder("Run Viterbi schedule");

	// set the inputs: we give the knowledge file, the actual trace file, the application and LPPM PDFs
	VERIFY(builder->SetInputs(&knowledgeFile, &actualTraceFile, &startPos, applicationPDF, lppmPDF) == true);

	VERIFY(startPos == ScheduleBeforeAttackOperation);

	// create and set the attack -> derive from Viterbi
	AttackOperation* attack = new SGAttackOperation(multFactor);
	VERIFY(builder->SetAttackOperation(attack) == true);
	attack->Release(); // release now

	// set the metric type
	VERIFY(builder->SetMetricType(SGMetric) == true);

	Schedule* schedule = builder->GetSchedule(); // retrieve the schedule
	VERIFY(schedule != NULL);

	// Free the builder (this essentially severs the tie between the builder and the schedule)
	delete builder;

	// because the schedule starts after the LPPM, the input to RunSchedule is an observed trace !
	File input(observedTraceFilePath, true);
	string outputPrefix = outputDir + "/" "output";
	if(lpm->RunSchedule(schedule , &input, outputPrefix.c_str()) == false) // run the schedule
	{
		std::cout << Errors::GetInstance()->GetLastErrorMessage() << endl ; // print the error message
		return false;
	}

	schedule->Release(); // release the schedule object (since it is no longer needed)

	app->Release(); // it is now safe to release the application and LPPM instances

	return true;
}

typedef struct _SampleTrace
{
	ull seedUserID;
	ull* trace;
	double logLikelihood;
	double geographicSim;
	double semanticSim;
	double intersection;
}
SampleTrace;

int main(int argc, char **argv)
{
	if(argc < 6)
	{
		cout << "Not enough arguments provided, exiting..." << endl;
		return -1;
	}

	char* fp = argv[1];
	string filePath = string(fp);

	char* ofp = argv[2];
	string outputDir = string(ofp);

	char* minU = argv[3];
	char* maxU = argv[4];
	char* maxT = argv[5];
	char* maxL = argv[6];

	/*
	ull sampleTraces = 100; // number of sample traces per user (S)
	if(argc >= 8) { char* st = argv[7];  stringstream ss(""); ss << st; ss >> sampleTraces; }

	ull numOutputTraces = 25; // number of output traces per user (K)
	if(argc >= 9) { char* ot = argv[8];  stringstream ss(""); ss << ot; ss >> numOutputTraces; }
	*/

	bool initOnly = false;
	if (argc >= 8) { string kct = string(argv[7]); if(kct == "initonly") { initOnly = true; } }

	//if(numOutputTraces > sampleTraces) { numOutputTraces = sampleTraces; }

	double removeProp = 0.4; double mergeProp = 0.5;
	if(argc >= 8) { char* rmp = argv[7];  stringstream ss(""); ss << rmp; ss >> removeProp; }
	if(argc >= 9) { char* mp = argv[8];  stringstream ss(""); ss << mp; ss >> mergeProp; }
	double removeActualLocProb = 1.0;
	if(argc >= 10) { char* rmalp = argv[9];  stringstream ss(""); ss << rmalp; ss >> removeActualLocProb; }

	double multFactor = 1.0;
	if(argc >= 11) { char* mf = argv[10];  stringstream ss(""); ss << mf; ss >> multFactor; }

	ull minUserID = 1; ull maxUserID = 0;
	{ stringstream ss(""); ss << minU; ss >> minUserID; }
	{ stringstream ss(""); ss << maxU; ss >> maxUserID; }

	const ull minTimestamp = 1; ull maxTimestamp = 0;
	{ stringstream ss(""); ss << maxT; ss >> maxTimestamp; }

	const ull minLoc = 1; ull maxLoc = 0;
	{ stringstream ss(""); ss << maxL; ss >> maxLoc; }

	VERIFY(minUserID <= maxUserID && minTimestamp < maxTimestamp && minLoc < maxLoc);

	LPM* lpm = LPM::GetInstance();
	RNG* rng = RNG::GetInstance();
	Log* logPtr = Log::GetInstance();

	Parameters* params = Parameters::GetInstance();
	params->AddUsersRange(minUserID, maxUserID);
	params->SetTimestampsRange(minTimestamp, maxTimestamp);
	params->SetLocationstampsRange(minLoc, maxLoc);

	const ull numTimes = maxTimestamp - minTimestamp + 1;

#if 0
	// simple time partitioning:
	// week days partitioned into morning (7am - 12pm), afternoon (12pm - 7pm), night (0am - 7am, 7pm - 12am)
	// weekend days partitioned into a single time period
	ull dayLength = 24; // time instants in a day
	ull days = timestamps / dayLength; // number of days
	const ull weeks = 1; // number of weeks to partition

	TPNode* timePart = Parameters::GetInstance()->CreateTimePartitioning(1, weeks * days * dayLength); // create a time partitioning from timestamp 1 to 168
    TPNode* week = NULL; TPNode* weekdays = NULL; TPNode* weekend = NULL;

	VERIFY(timePart->SliceOut(0, days * dayLength, weeks, &week) == true); // slice out a week

	VERIFY(week->SliceOut(0, dayLength, 5, &weekdays) == true); // slice out the week days (first 5 days, assuming the first timestamp is on a Monday)

	VERIFY(week->SliceOut(5*dayLength, dayLength, 2, &weekend) == true); // slice out the weekend days (the last 2 remaining days)

	// create time periods for the week days: morning (7am - 12pm), afternoon (12pm - 7pm), night (0am - 7am, 7pm - 12am)
	TimePeriod morningwd; morningwd.start = 7 * dayLength/24; morningwd.length=5 * dayLength/24; morningwd.id = 1; morningwd.dummy = false;
	TimePeriod afternoonwd; afternoonwd.start = 12 * dayLength/24; afternoonwd.length=7 * dayLength/24; afternoonwd.id = 2; afternoonwd.dummy = false;
	TimePeriod nightpart1; nightpart1.start = 0 * dayLength/24; nightpart1.length=7 * dayLength/24; nightpart1.id = 3; nightpart1.dummy = false;
	TimePeriod nightpart2; nightpart2.start = 19 * dayLength/24; nightpart2.length=5 * dayLength/24; nightpart2.id = 3; nightpart2.dummy = false;

	vector<TimePeriod> periods = vector<TimePeriod>();
	periods.push_back(morningwd);	periods.push_back(afternoonwd);
	periods.push_back(nightpart1); periods.push_back(nightpart2);
	VERIFY(weekdays->Partition(periods) == true); // partition each of the 5 week days

	// create time periods for the weekend: a single time period for each day
	TimePeriod we; we.start = 0 * dayLength/24; we.length=24 * dayLength/24; we.id = 4; we.dummy = false;
	periods.clear(); periods.push_back(we);
	VERIFY(weekend->Partition(periods) == true); // partition each of the 2 weekend days

	Parameters::GetInstance()->SetTimePartitioning(timePart); // set the time partitioning
#else
	TPNode* timePart = Parameters::GetInstance()->CreateTimePartitioning(1, maxTimestamp);

	TimePeriod whole; whole.start = 0; whole.length = numTimes; whole.id = 1; whole.dummy = false;
	vector<TimePeriod> periods = vector<TimePeriod>(); periods.clear(); periods.push_back(whole);
	VERIFY(timePart->Partition(periods) == true); // partition each of the 2 weekend days
	Parameters::GetInstance()->SetTimePartitioning(timePart); // set the time partitioning
#endif

	logPtr->SetEnabled(true);
	logPtr->SetOutputFileName(outputDir + "/" "output");

	string traceFilePath = filePath + ".trace";
	string mobilityFilePath = filePath + ".mobility";

	// print out the time partitioning
	string str = ""; VERIFY(timePart->GetStringRepresentation(str) == true);
	std::cout << "Time Partitioning:" << endl << str << endl;

	// log stuff
	{
		stringstream ssl("");
		ssl << "[SG Parameters] " << "traceFilePath: " << traceFilePath << endl;
		ssl << "Output Directory: " << outputDir;
		logPtr->Append(ssl.str());

		ssl.str("");
		ssl << "Users: (" << minUserID << ", " << maxUserID << "), ";
		ssl << "Times: (" << minTimestamp << ", " << maxTimestamp << "), ";
		ssl << "Locs: (" << minLoc << ", " << maxLoc << ")";
		logPtr->Append(ssl.str());

		//ssl.str(""); ssl << "# of traces to sample per user (S): " << sampleTraces; logPtr->Append(ssl.str());
		//ssl.str(""); ssl << "# of traces to output per user (K): " << numOutputTraces; logPtr->Append(ssl.str());

		ssl.str(""); ssl << "Remove proportion: " << removeProp; logPtr->Append(ssl.str());
		ssl.str(""); ssl << "Merge proportion: " << mergeProp; logPtr->Append(ssl.str());

		ssl.str(""); ssl << "Remove actual loc prob: " << removeActualLocProb; logPtr->Append(ssl.str());
		ssl.str(""); ssl << "Viterbi m-factor: " << multFactor; logPtr->Append(ssl.str());

		ssl.str(""); ssl << "Time Partitioning:" << str; logPtr->Append(ssl.str());
	}

	size_t pos = filePath.rfind('/');
	string inputDir = filePath.substr(0, pos);

	string aggregateStatsFilePath = inputDir + "/" "aggregate.stats";
	string locationsFilePath = inputDir + "/" "locations";
	if(ComputeAggregateStats(traceFilePath, locationsFilePath,  aggregateStatsFilePath) == false) { return -1; }

	string knowledgeFilePath = inputDir + "/" "knowledge";
	if(ConstructKnowledge(traceFilePath, mobilityFilePath, knowledgeFilePath) == false) { return -1; }

	string locClustersFilePath = inputDir + "/" "locations.clusters";
	if(ClusterLocations(outputDir, knowledgeFilePath, locClustersFilePath) == false) { return -1; }


	if(initOnly == true) { return 0; }

	string observedTraceFilePath = outputDir + "/" "output-lppm";

	string outputFilePath = outputDir + "/" "output-metric_sg";

	// load seed traces
	TraceSet* actualTraceSet = NULL;
	{
		File seedTraceFile(traceFilePath);

		// read the traces
		InputOperation* inputOperation = new InputOperation();

		actualTraceSet = new TraceSet(ActualTrace);

		bool readOk = inputOperation->Execute(&seedTraceFile, actualTraceSet); inputOperation->Release();
		VERIFY(readOk == true);
	}


    LineParser<ull>* parser = LineParser<ull>::GetInstance();
    LineParser<double>* parserd = LineParser<double>::GetInstance();

    const double llBigFactor = MIN((double)numTimes, log(sqrt(DBL_MAX)));

	map<ull, ull> traceIdxMap;

	map<ull, Trace*> tracesMap;
	actualTraceSet->GetMapping(tracesMap);

	//const string tmpDir = "/tmp/";
	const string tmpDir = outputDir + "/";

	//for(ull i = 0; i < sampleTraces; i++)
	// generate traces until killed...
    for(ull i = 0; ; i++)
	{
		map<ull, SampleTrace> sampledTracesMap;

		stringstream ssl(""); ssl << "Starting sampling for trace " << i;
		Log::GetInstance()->Append(ssl.str());

		LPPMOperation* lppm = NULL;
		if(RunSGLPPM(outputDir, traceFilePath, knowledgeFilePath, aggregateStatsFilePath, locClustersFilePath,
									removeProp, mergeProp, removeActualLocProb, &lppm) == false) { return -1; }

		if(RunViterbi(outputDir, traceFilePath, observedTraceFilePath,
									aggregateStatsFilePath, locClustersFilePath, multFactor, lppm) == false) { return -1; }

		lppm->Release(); // release ownership


		File traceTempOutputFile(outputFilePath, true);


		while(traceTempOutputFile.IsGood())
		{
			string line = "";

			bool readOk = traceTempOutputFile.ReadNextLine(line);
			VERIFY(readOk == true);

			if(traceTempOutputFile.IsGood() == false && line.empty() == true) { break; }

			size_t posColon = line.find(':');
			string firstPart = line.substr(0, posColon);
			string secondPart = line.substr(posColon + 1);

			ull user = 0.0; size_t pos = 0;
			bool parseOk = parser->ParseValue(firstPart, &user, &pos);
			VERIFY(parseOk == true && pos != string::npos);

			double llk = 0.0; firstPart = firstPart.substr(pos+1);
			parseOk = parserd->ParseValue(firstPart, &llk, &pos);
			VERIFY(parseOk == true && pos == string::npos);

			vector<ull> trace;
			parseOk = parser->ParseFields(secondPart, trace, ANY_NUMBER_OF_FIELDS, &pos);
			VERIFY(parseOk == true && pos == string::npos);

			VERIFY(trace.size() == numTimes);

			SampleTrace sampleTrace;

			// make sure we don't over/under-flow
			double minExponent = log(SQRT_DBL_MIN);
			if(llk < minExponent) { llk = minExponent; }

			//sampleTrace.logLikelihood = exp(llk + llBigFactor);
			double unnorml = exp(llk + llBigFactor);

			sampleTrace.seedUserID = user;
			sampleTrace.logLikelihood = llk;
			sampleTrace.geographicSim = -1.0;
			sampleTrace.semanticSim = -1.0;
			sampleTrace.intersection = -1.0;

			ull traceByteSize = sizeof(ull) * numTimes;
			sampleTrace.trace = (ull*)Allocate(traceByteSize);
			VERIFY(sampleTrace.trace != NULL); memset(sampleTrace.trace, 0, traceByteSize);
			for(ull tm = minTimestamp; tm <= maxTimestamp; tm++)
			{
				ull idx = tm - minTimestamp;
				sampleTrace.trace[idx] = trace[idx];
			}

			sampledTracesMap.insert(make_pair(user, sampleTrace));

			{ // logging
				stringstream ssl(""); ssl << "Seed user " << user << ", trace: " << i;
				ssl << ", logLikelihood: " << llk << ", likelihood (not normalized): " << unnorml;
				Log::GetInstance()->Append(ssl.str());
			}
		}

		const ull seedUserID = 1;
		const ull sampleTraceUserID = 2;
		pair_foreach_const(map<ull, Trace*>, tracesMap, iterSeedTrace)
		{
			TraceSet* seedTraceSet = new TraceSet(ActualTrace);

			ull userID = iterSeedTrace->first;
			Trace* trace = iterSeedTrace->second;
			vector<Event*> events; trace->GetEvents(events);

			ull traceByteSize = sizeof(ull) * numTimes;
			ull* strace = (ull*)Allocate(traceByteSize);
			VERIFY(strace != NULL); memset(strace, 0, traceByteSize);

			foreach_const(vector<Event*>, events, iterEvents)
			{
				ActualEvent* e = dynamic_cast<ActualEvent*>(*iterEvents);
				VERIFY(e != NULL);

				ull tm = e->GetTimestamp();
				ull loc =  e->GetLocationstamp();

				ActualEvent* modifiedEvent = new ActualEvent(seedUserID, tm, loc);
				seedTraceSet->AddEvent(modifiedEvent);
				modifiedEvent->Release();

				ull tmIdx = (tm - minTimestamp);
				VERIFY(tmIdx >= 0 && tmIdx < numTimes);
				strace[tmIdx] = loc;
			}

			map<ull, SampleTrace>::iterator iterST = sampledTracesMap.find(userID);
			VERIFY(iterST != sampledTracesMap.end());

			SampleTrace& sampleTrace = iterST->second;

			VERIFY(userID == sampleTrace.seedUserID);

			double intersect = 0.0;
			for(ull tm = minTimestamp; tm <= maxTimestamp; tm++)
			{
				ull tmIdx = tm - minTimestamp;
				ull loc = sampleTrace.trace[tmIdx];

				ActualEvent* modifiedEvent = new ActualEvent(sampleTraceUserID, tm, loc);
				seedTraceSet->AddEvent(modifiedEvent);
				modifiedEvent->Release();

				// compute intersection
				if(loc == strace[tmIdx]) { intersect += 1.0; }
			}
			intersect /= numTimes;

			Free(strace); // free the seed trace
			sampleTrace.intersection = intersect;


			stringstream sst("");
			ull r = rng->GetUniformRandomULLBetween(0, LLONG_MAX);
			sst << tmpDir << "sg-LPM__T_" << r;
			string tempTraceFilePath = sst.str();

			{
				File tempTraceFile(tempTraceFilePath, false);
				VERIFY(tempTraceFile.IsGood() == true);

				// write down the traces
				OutputOperation* outputOperation = new OutputOperation();
				VERIFY(outputOperation->Execute(seedTraceSet, &tempTraceFile) == true); outputOperation->Release();
			}

			seedTraceSet->Release();

			sst.str(""); sst << tmpDir << "sg-LPM__K_" << r;
			string tempKnowledgeFilePath = sst.str();


			set<ull> rusersSet; VERIFY(params->GetUsersSet(rusersSet) == true);

			// temporary knowledge construction
			{
				File newTempTraceFile(tempTraceFilePath, true);
				File mobilityFile(mobilityFilePath, true);
				File tempKnowledgeFile(tempKnowledgeFilePath, false);

				VERIFY(newTempTraceFile.IsGood() == true);
				VERIFY(tempKnowledgeFile.IsGood() == true);

				KnowledgeInput knowledge;
				knowledge.transitionsFeasibilityFile = &mobilityFile;
				knowledge.transitionsCountFile = NULL;
				knowledge.learningTraceFilesVector = vector<File*>();
				knowledge.learningTraceFilesVector.push_back(&newTempTraceFile);

				const ull maxGSIterations = 100000;
				const ull maxSeconds = 60;

				// set the user set, so the knowledge construction happens correctly
				params->ClearUsersSet();
				params->AddUsersRange(seedUserID, seedUserID);
				params->AddUsersRange(sampleTraceUserID, sampleTraceUserID);

				VERIFY(lpm->RunKnowledgeConstruction(&knowledge, &tempKnowledgeFile, maxGSIterations, maxSeconds) == true);

				remove(tempTraceFilePath.c_str()); // remove the temp file (trace)
			}

			// do context analysis to get similarity
			{
				const bool zerothOrderOnly = true;

				for(ull i = 0; i<2; i++)
				{
					File ntempKnowledgeFile(tempKnowledgeFilePath, true);

					VERIFY(ntempKnowledgeFile.IsGood() == true);

					string name = "Temporary Similarity Analysis Schedule.";
					ContextAnalysisOperation* contextAnalysisOp = NULL;

					if(i == 0) { contextAnalysisOp = new AbsoluteSimilarityAnalysisOperation("GeographicSimilarityAnalysis", zerothOrderOnly); }
					else { contextAnalysisOp = new HiddenSemanticsSimilarityAnalysisOperation("SemanticSimilarityAnalysis", zerothOrderOnly); }
					VERIFY(contextAnalysisOp != NULL);

					ContextAnalysisSchedule* schedule = new ContextAnalysisSchedule(name, contextAnalysisOp);
					VERIFY(schedule != NULL);

					stringstream ssts(""); ssts << tmpDir << "sg-LPM__S_" << r;
					string outputSFP = ssts.str();
					VERIFY(lpm->RunContextAnalysisSchedule(schedule, &ntempKnowledgeFile, outputSFP.c_str()) == true); // run the schedule

					File simOutputFile(outputSFP, true);

					string line = ""; VERIFY(simOutputFile.IsGood() == true);
					bool readOk = simOutputFile.ReadNextLine(line); VERIFY(readOk == true);
					; // ignore first line

					line = ""; VERIFY(simOutputFile.IsGood() == true);
					readOk = simOutputFile.ReadNextLine(line); VERIFY(readOk == true);

					size_t posColon = line.find(':');
					string firstPart = line.substr(0, posColon);
					string secondPart = line.substr(posColon + 1);


					VERIFY(firstPart == "1, 2");
					double sim = 0.0; size_t pos = 0;
					bool parseOk = parserd->ParseValue(secondPart, &sim, &pos);
					VERIFY(parseOk == true && pos == string::npos);

					VERIFY(sim >= 0 && sim <= 1.0);

					if(i==0) { sampleTrace.geographicSim = sim; }
					else { sampleTrace.semanticSim = sim; }

					contextAnalysisOp->Release();
					delete schedule;

					remove(outputSFP.c_str()); // remove the temp file (sim)
				}
			}

			remove(tempKnowledgeFilePath.c_str()); // remove the temp file (knowledge)
			string tempKnowledgeFilePathAcc = tempKnowledgeFilePath + ".accuracy";
			remove(tempKnowledgeFilePathAcc.c_str()); // remove the temp file (knowledge.accuracy)

			// restore the user set;
			params->ClearUsersSet();
			foreach_const(set<ull>, rusersSet, iterUsers) { params->AddUsersRange(*iterUsers, *iterUsers); }

			{ // logging
				stringstream ssl("");
				ssl << "Seed user: " << sampleTrace.seedUserID << ", trace: " << i;
				ssl << ", llk: " << sampleTrace.logLikelihood;
				ssl << ", geo-sim: " << sampleTrace.geographicSim;
				ssl << ", sem-sim: " << sampleTrace.semanticSim;
				ssl << ", intersection: " << sampleTrace.intersection;
				Log::GetInstance()->Append(ssl.str());
			}

			// finally write the trace and suppl file to disk
			{
				ull traceIdx = 0;

				// load traceIdx
				map<ull, ull>::iterator iterTraceIdx = traceIdxMap.find(userID);
				if(iterTraceIdx == traceIdxMap.end())
				{
					traceIdxMap.insert(make_pair(userID, traceIdx));
					iterTraceIdx = traceIdxMap.find(userID);
				}
				traceIdx = iterTraceIdx->second;

				// find appropriate traceIdx
				{
					const ull maxAttempts = 10000;
					ull attempts = 0;

					bool done = false;
					while(done == false)
					{
						attempts++;

						{ // see if file exists
							stringstream ssotmp("");
							ssotmp << "out" << "/" << "user" << userID << "/" "synthetic-trace" << traceIdx;
							string tryOutputTraceFilePath = outputDir + "/" + ssotmp.str();
							File tryOutputTraceFile(tryOutputTraceFilePath, true);

							if(tryOutputTraceFile.IsGood() == false) { done = true; } // file does not exist -> go ahead
							else { traceIdx++; }
						}

						if(attempts >= maxAttempts) { traceIdx = rng->GetUniformRandomULLBetween(0, 2*attempts); }
					}
				}
				// save traceIdx
				iterTraceIdx->second = traceIdx + 1;

				stringstream ssot("");
				ssot << "out" << "/" << "user" << userID << "/" "synthetic-trace" << traceIdx;
				string outputTraceFilePath = outputDir + "/" + ssot.str();
				File outputTraceFile(outputTraceFilePath, false);
				VERIFY(outputTraceFile.IsGood() == true);

				// write down info
				ssot << ".info";
				{
					string outputInfoFilePath = outputDir + "/" + ssot.str();
					File outputInfoFile(outputInfoFilePath, false);
					VERIFY(outputInfoFile.IsGood() == true);

					stringstream ss2("");
					ss2 << sampleTrace.seedUserID << DEFAULT_FIELDS_DELIMITER;
					ss2 << sampleTrace.logLikelihood << DEFAULT_FIELDS_DELIMITER;
					ss2 << sampleTrace.geographicSim << DEFAULT_FIELDS_DELIMITER;
					ss2 << sampleTrace.semanticSim << DEFAULT_FIELDS_DELIMITER;
					ss2 << sampleTrace.intersection;

					outputInfoFile.WriteLine(ss2.str());

					// save the parameters, just in case we need them later
					ss2.str("");

					ss2 << removeProp << DEFAULT_FIELDS_DELIMITER;
					ss2 << mergeProp << DEFAULT_FIELDS_DELIMITER;
					ss2 << removeActualLocProb << DEFAULT_FIELDS_DELIMITER;
					ss2 << multFactor;

					outputInfoFile.WriteLine(ss2.str());
				}

				ull* trace = sampleTrace.trace;
				VERIFY(trace != NULL);

				for(ull tm = minTimestamp; tm <= maxTimestamp; tm++)
				{
					ull idx = tm - minTimestamp;
					ull loc = trace[idx];

					stringstream ss2("");
					ss2 << userID << DEFAULT_FIELDS_DELIMITER;
					ss2 << tm << DEFAULT_FIELDS_DELIMITER;
					ss2 << loc;

					outputTraceFile.WriteLine(ss2.str());
				}

				Free(sampleTrace.trace); sampleTrace.trace = NULL;
			}

		}

	}

	actualTraceSet->Release();

	stringstream ssl(""); ssl << "Done with sampling.";
	Log::GetInstance()->Append(ssl.str());

	ssl.str(""); ssl << "All done, exiting.";
	Log::GetInstance()->Append(ssl.str());

	return 0;
}
