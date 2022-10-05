#include "SGLPPMOperation.h"

SGLPPMOperation::SGLPPMOperation(File& traceFile, map<ull, ull> ltcMap, vector<set<ull> >& clustersVec, double rmp, double mergep, double rmalp)
		: LPPMOperation("SGLPPMOperation", NoFlags), ssClustersMap(), ssClusterIdxMap(), locToClusterIdxMap(ltcMap), probRemoveActualLoc(rmalp)
{
	// subsampling
	RNG* rng = RNG::GetInstance();
	const double removeProp = rmp; const ull minLeft = 3;

	const double mergeProp = mergep;


	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);
	// ull numTimes = maxTime - minTime + 1;


	// read the traces, construct the clusters
	InputOperation* inputOperation = new InputOperation();

	TraceSet* actualTraceSet = new TraceSet(ActualTrace);

	bool readOk = inputOperation->Execute(&traceFile, actualTraceSet); inputOperation->Release();
	VERIFY(readOk == true);

	map<ull, Trace*> tracesMap;
	actualTraceSet->GetMapping(tracesMap);

	// do this in two steps, first we subsample the clusters, then we merge clusters around transitions
	pair_foreach_const(map<ull, Trace*>, tracesMap, iterMapping)
	{
		ull user = iterMapping->first;
		Trace* trace = iterMapping->second;

		vector<Event*> events = vector<Event*>();
		trace->GetEvents(events);

		vector<set<ull> > userClustersVec; vector<ull> userClusterIdxVec;
		map<ull, set<ull> > tiSSClustersMap; // time independent subsampled clusters

		stringstream ssl(""); ssl << "Subsampling clusters for user " << user;
		Log::GetInstance()->Append(ssl.str());

		// step 1: subsamples the clusters
		foreach_const(vector<Event*>, events, iterEvents)
		{
			ActualEvent* event = dynamic_cast<ActualEvent*>(*iterEvents);
			VERIFY(event != NULL); VERIFY(user == event->GetUser());

			// ull tm = event->GetTimestamp();
			ull loc = event->GetLocationstamp();

			map<ull, ull>::iterator iterltc = ltcMap.find(loc);
			VERIFY(iterltc != ltcMap.end());
			ull clusterIdx = iterltc->second; VERIFY(clusterIdx < clustersVec.size());

			map<ull, set<ull> >::iterator iterSSC = tiSSClustersMap.find(clusterIdx);

			if(iterSSC == tiSSClustersMap.end())
			{
				// do the subsampling

				set<ull>& clusterLocs = clustersVec[clusterIdx];
				set<ull> ssClusterLocs; ssClusterLocs.insert(clusterLocs.begin(), clusterLocs.end());

				ull count = ssClusterLocs.size();
				ll removeCount = (ll)(removeProp * count);
				if((ll)((ll)count - removeCount) >= (ll)minLeft)
				{
					while(removeCount > 0)
					{
						size_t setIdx = rng->GetUniformRandomULLBetween(0, count-1);
						set<ull>::iterator iter = ssClusterLocs.begin();
						std::advance(iter, setIdx);
						ssClusterLocs.erase(*iter);
						removeCount--;
					}
				}

				tiSSClustersMap.insert(make_pair(clusterIdx, ssClusterLocs));

				iterSSC = tiSSClustersMap.find(clusterIdx);
			}

			/*{ // logging
				ssl.str(""); ssl << "Time-indep. cluster " << clusterIdx << " (at loc: " << loc << ") - ";
				foreach_const(set<ull>, iterSSC->second, iterTmp) { if(iterTmp != iterSSC->second.begin()) { ssl << ", "; } ssl << *iterTmp;  }
				Log::GetInstance()->Append(ssl.str());
			}*/

			userClustersVec.push_back(iterSSC->second);
			userClusterIdxVec.push_back(clusterIdx);
		}

		// step 2: merging clusters around transitions
		ull prevtm = 0;
		ull tmIdx = 0;
		vector<set<ull> > tdUserClustersVec; // time dependent
		for(ull i=0; i< userClustersVec.size(); i++) { tdUserClustersVec.push_back(set<ull>()); }

		foreach_const(vector<Event*>, events, iterEvents)
		{
			ActualEvent* event = dynamic_cast<ActualEvent*>(*iterEvents);
			VERIFY(event != NULL); VERIFY(user == event->GetUser());

			ull tm = event->GetTimestamp();
			ull loc = event->GetLocationstamp();
			VERIFY(prevtm == 0 || tm == prevtm+1);

			VERIFY(tm == (minTime + tmIdx));

			set<ull>& currentClusterLocs = userClustersVec[tmIdx];

			// find next transition
			bool foundNext = false; ull distance = 0; ull fw2distance = 0; ull fwtmIdx = 0;
			for(ull fwtm = tm+1; fwtm <= maxTime; fwtm++)
			{
				distance = (fwtm - tm);
				fwtmIdx = tmIdx + distance;

				ActualEvent* fwevent = dynamic_cast<ActualEvent*>(events[fwtmIdx]);
				VERIFY(fwevent != NULL); VERIFY(user == fwevent->GetUser());
				VERIFY(fwevent->GetTimestamp() == fwtm);

				ull fwLoc = fwevent->GetLocationstamp();

				if(loc != fwLoc)
				{
					foundNext = true;
					for(ull fw2tm = fwtm+1; fw2tm <= maxTime; fw2tm++)
					{
						ull fw2dist = (fw2tm - fwtm);
						ull fw2tmIdx = fwtmIdx + fw2dist;

						ActualEvent* fw2event = dynamic_cast<ActualEvent*>(events[fw2tmIdx]);
						VERIFY(fw2event != NULL); VERIFY(user == fw2event->GetUser());
						VERIFY(fw2event->GetTimestamp() == fw2tm);

						ull fw2Loc = fw2event->GetLocationstamp();
						if(fw2Loc == fwLoc) { fw2distance++; }
						else { break; }
					}

					break;
				}
			}

			if(foundNext == true)
			{
				// do probabilistic cross merging based on distance
				set<ull>& fwClusterLocs = userClustersVec[fwtmIdx];

				if(currentClusterLocs != fwClusterLocs) // we can only merge the clusters if they are different
				{
					// compute the additive set
					set<ull> addLocSet;
					double effectiveMergeProp = pow(mergeProp, distance * distance);

					ull effectiveSampleCount = (ull)(effectiveMergeProp * currentClusterLocs.size());
					if(effectiveSampleCount > currentClusterLocs.size()) { effectiveSampleCount = currentClusterLocs.size(); }

					while(addLocSet.size() < effectiveSampleCount)
					{
						size_t setIdx = rng->GetUniformRandomULLBetween(0, effectiveSampleCount-1);
						set<ull>::iterator iterSample = currentClusterLocs.begin();
						std::advance(iterSample, setIdx);
						addLocSet.insert(*iterSample);
					}

					ull effectiveSampleCountfw = (ull)(effectiveMergeProp * fwClusterLocs.size());
					if(effectiveSampleCountfw > fwClusterLocs.size()) { effectiveSampleCountfw = fwClusterLocs.size(); }

					while(addLocSet.size() < effectiveSampleCount + effectiveSampleCountfw)
					{
						size_t setIdx = rng->GetUniformRandomULLBetween(0, effectiveSampleCountfw-1);
						set<ull>::iterator iterSample = fwClusterLocs.begin();
						std::advance(iterSample, setIdx);
						addLocSet.insert(*iterSample);
					}

					// now that we have computed the additive loc set, we just add it
					if(addLocSet.empty() == false)
					{
						ull maxvidx = min(tmIdx + distance + min(fw2distance, distance), maxTime - minTime);
						for(ull vidx = tmIdx; vidx <= maxvidx; vidx++)
						{
							tdUserClustersVec[vidx].insert(addLocSet.begin(), addLocSet.end());
						}
					}
				}
			}

			tdUserClustersVec[tmIdx].insert(currentClusterLocs.begin(), currentClusterLocs.end());

			/*{ // logging
				stringstream ssl(""); ssl << "Time-dep. cluster at tm: " << tm << " (loc: " << loc << ") - ";
				foreach_const(set<ull>, tdUserClustersVec[tmIdx], iterTmp) { if(iterTmp != tdUserClustersVec[tmIdx].begin()) { ssl << ", "; } ssl << *iterTmp;  }
				Log::GetInstance()->Append(ssl.str());
			}*/

			prevtm = tm;
			tmIdx++;
		}

		ssClustersMap.insert(make_pair(user, tdUserClustersVec)); // add the constructed clusters for that user
		ssClusterIdxMap.insert(make_pair(user, userClusterIdxVec));
	}
	actualTraceSet->Release();
}

SGLPPMOperation::~SGLPPMOperation() { }

bool SGLPPMOperation::LookupCluster(ull user, ull tm, set<ull>& out) const
{
	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);

	VERIFY(tm >= minTime && tm <= maxTime);

	map<ull, vector<set<ull> > >::const_iterator iterMap = ssClustersMap.find(user);
	if(iterMap == ssClustersMap.end()) { return false; }

	const vector<set<ull> >& ssVec = iterMap->second;
	ull tmIdx = tm - minTime;

	out = ssVec[tmIdx];

	return true;
}

bool SGLPPMOperation::Filter(const Context* context, const ActualEvent* inEvent, ObservedEvent** outEvent)
{
	// do the same regardless of whether the event is exposed.
	// all we do is substitute the actual loc with the clusters locations (which have been previously subsampled)

	ull user = inEvent->GetUser();
	ull tm = inEvent->GetTimestamp();
	ull actualLoc = inEvent->GetLocationstamp();

	set<ull> outputLocsSet;
	bool foundCluster = LookupCluster(user, tm, outputLocsSet); // actualLoc is not needed because of init
	VERIFY(foundCluster == true);

	ObservedEvent* observedEvent = *outEvent = new ObservedEvent(user);
	observedEvent->AddTimestamp(tm);

	// remove the actual loc from outputLocsSet (probabilistically)
	if(RNG::GetInstance()->GetUniformRandomDouble() <= probRemoveActualLoc)
	{ outputLocsSet.erase(actualLoc); }

	foreach_const(set<ull>, outputLocsSet, iter) { observedEvent->AddLocationstamp(*iter); }

	return true;
}

double SGLPPMOperation::PDF(const Context* context, const ActualEvent* inEvent, const ObservedEvent* outEvent) const
{
	ull minTime = 0; ull maxTime = 0;
	VERIFY(Parameters::GetInstance()->GetTimestampsRange(&minTime, &maxTime) == true);

	ull user = inEvent->GetUser();
	ull tm = inEvent->GetTimestamp();
	ull loc = inEvent->GetLocationstamp();

	ull nym = outEvent->GetPseudonym();
	set<ull> timestamps; outEvent->GetTimestamps(timestamps);
	if(user != nym || timestamps.size() != 1 || *(timestamps.begin()) != tm) { return 0.0; } // event of probability 0.0

	set<ull> locationstamps; outEvent->GetLocationstamps(locationstamps);

	// if loc is among locationstamps, prob 1.0, else 0.0;
	if(locationstamps.find(loc) != locationstamps.end()) { return 1.0; }
	return 0.0;
}
