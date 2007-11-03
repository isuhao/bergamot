#include "SyncServerTimer.h"
#include "e32std.h"
#include <EIKENV.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>

CSyncServerTimer::CSyncServerTimer(TSmlProfileId anId) : CTimer::CTimer(EPriorityIdle) {
	profileId = anId;
}

CSyncServerTimer::~CSyncServerTimer() {
}
void CSyncServerTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CSyncServerTimer::RunL() {
	RDebug::Print(_L("Sync account %d"), profileId);
	RSyncMLSession session;
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone)
		{
		RDebug::Print(_L("Session OpenL error: %d"), error);
		}
	RSyncMLDataSyncJob job;
	TBuf<255> Buffer;
	
	RSyncMLDataSyncProfile profile;
	TRAP(error,profile.OpenL(session, profileId,ESmlOpenRead));
	if (error!=KErrNone)
	{
		RDebug::Print(_L("ProfileOpenL error: %d"), error);
	}
	
	profile.Close();
	
	// sync!
	TRAP(error,job.CreateL(session, profileId));
	
	if (error!=KErrNone)
	{
		RDebug::Print(_L("CreateL error: %d"), error);
	}
	
	job.Close();
	
	session.Close();
	// run again
	RunPeriodically();
}

void CSyncServerTimer::SetPeriod(int aPeriod) {
	RDebug::Print(_L("Setting period to %d"), aPeriod);
	thePeriod = aPeriod;
}

void CSyncServerTimer::RunPeriodically() {
	RDebug::Print(_L("RunPeriodically; thePeriod=%d"), thePeriod);
	TTime time;
	time.UniversalTime();

	switch(thePeriod) {
	case 0:
		return;
	case 1: {
//		TTimeIntervalSeconds tmi; // for testing
		TTimeIntervalMinutes tmi;
		tmi = 5;
		time = time + tmi;
		break;
	}
	case 2: {
		TTimeIntervalMinutes tmi;
		tmi = 60;
		time = time + tmi;
		break;
	}
	case 3: {
		TTimeIntervalHours tmi;
		tmi = 4;
		time = time + tmi;
		break;
	}
	case 4: {
		TTimeIntervalHours tmi;
		tmi = 24;
		time = time + tmi;
		break;
	}
	default:
		return;
	}

	RDebug::Print(_L("Running timer"));
	TRAPD(error,AtUTC(time));
	
	if (error != KErrNone) {
		RDebug::Print(_L("Error from AtUTC: %d"), error);
	}
}

