#include "SyncServerTimer.h"
#include "e32std.h"
#include <EIKENV.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include "SyncServerData.h"
#include "debug.h"

CSyncServerTimer::CSyncServerTimer(TSmlProfileId anId) : CTimer::CTimer(EPriorityIdle) {
	profileId = anId;
}

CSyncServerTimer::~CSyncServerTimer() {
}
void CSyncServerTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
//	mutex.CreateGlobal(KSyncServerMutex);
}

void CSyncServerTimer::RunL() {
//	mutex.OpenGlobal(KSyncServerMutex);
//	mutex.Wait();
	DP("Mutex Wait finished");
	DP1("Sync account %d", profileId);
	RSyncMLSession session;
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone)
		{
		DP1("Session OpenL error: %d", error);
		}
	RSyncMLDataSyncJob job;
	TBuf<255> Buffer;
	
	RSyncMLDataSyncProfile profile;
	TRAP(error,profile.OpenL(session, profileId,ESmlOpenRead));
	if (error!=KErrNone)
	{
		DP1("ProfileOpenL error: %d", error);
	}
	
	profile.Close();
	
	// sync!
	TRAP(error,job.CreateL(session, profileId));
	
	if (error!=KErrNone)
	{
		DP1("CreateL error: %d", error);
	}
	
	job.Close();
	
	session.Close();
//	DP("Mutex Signal"));
//	mutex.Signal();
//	mutex.Close();
	// run again
	RunPeriodically();
}

void CSyncServerTimer::SetPeriod(int aPeriod) {
	DP1("Setting period to %d", aPeriod);
	thePeriod = aPeriod;
}

void CSyncServerTimer::RunPeriodically() {
	DP1("RunPeriodically; thePeriod=%d", thePeriod);
	TTime time;
	time.UniversalTime();

	switch(thePeriod) {
	case ENever:
		return;
	case EFifteenMinutes: {
//		TTimeIntervalSeconds tmi; // for testing
		TTimeIntervalMinutes tmi;
		tmi = 15;
		time = time + tmi;
		break;
	}
	case EOneHour: {
		TTimeIntervalMinutes tmi;
		tmi = 60;
		time = time + tmi;
		break;
	}
	case EFourHours: {
		TTimeIntervalHours tmi;
		tmi = 4;
		time = time + tmi;
		break;
	}
	case ETwelveHours: {
		TTimeIntervalHours tmi;
		tmi = 12;
		time = time + tmi;
		break;
	}	
	case EDaily: {
		TTimeIntervalHours tmi;
		tmi = 24;
		time = time + tmi;
		break;
	}
	case EWeekly: {
		TTimeIntervalDays tmi;
		tmi = 7;
		time = time + tmi;
		break;
	}
	default:
		DP1("ERROR: Invalid period=%d", thePeriod);
		return;
	}

	DP("Running timer");
	TRAPD(error,AtUTC(time));
	
	if (error != KErrNone) {
		DP1("Error from AtUTC: %d", error);
	}
}

