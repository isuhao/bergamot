#include "KitchenSyncTimer.h"
#include "e32std.h"
#include <EIKENV.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>

CKitchenSyncTimer::CKitchenSyncTimer(TSmlProfileId anId) : CTimer::CTimer(EPriorityIdle) {
	profileId = anId;
}

CKitchenSyncTimer::~CKitchenSyncTimer() {
}
void CKitchenSyncTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CKitchenSyncTimer::RunL() {
	RDebug::Print(_L("Sync account %d"), profileId);
	RSyncMLSession session;
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone)
		{
		CEikonEnv::InfoWinL(_L("KitchenSync"), _L("OpenL error"));
		}
	RSyncMLDataSyncJob job;
	TBuf<255> Buffer;
	
	RSyncMLDataSyncProfile profile;
	TRAP(error,profile.OpenL(session, profileId,ESmlOpenRead));
	if (error!=KErrNone)
	{
		_LIT(KRowFormatter, "OpenL id=%d error: %d");
		Buffer.Format(KRowFormatter, profileId, error);
		CEikonEnv::InfoWinL(_L("KitchenSync"), Buffer);
	}
	
	//CEikonEnv::InfoWinL(_L("KitchenSync"), profile.DisplayName());
	profile.Close();
	
	// sync!
	TRAP(error,job.CreateL(session, profileId));
	
	if (error!=KErrNone)
	{
		_LIT(KRowFormatter, "CreateL error: %d");
		Buffer.Format(KRowFormatter, error);
		CEikonEnv::InfoWinL(_L("KitchenSync"), Buffer);
	}
	
	job.Close();
	
	session.Close();
	// run again
	RunPeriodically();
}

void CKitchenSyncTimer::SetPeriod(int aPeriod) {
	RDebug::Print(_L("Setting period to %d"), aPeriod);
	thePeriod = aPeriod;
}

void CKitchenSyncTimer::RunPeriodically() {
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

