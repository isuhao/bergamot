#include "PodcastServerTimer.h"
#include "e32std.h"
#include <EIKENV.h>

CPodcastServerTimer::CPodcastServerTimer(TSmlProfileId anId) : CTimer::CTimer(EPriorityIdle) {
	profileId = anId;
}

CPodcastServerTimer::~CPodcastServerTimer() {
}
void CPodcastServerTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CPodcastServerTimer::RunL() {
	// run again
	RunPeriodically();
}

void CPodcastServerTimer::SetPeriod(int aPeriod) {
	RDebug::Print(_L("Setting period to %d"), aPeriod);
	thePeriod = aPeriod;
}

void CPodcastServerTimer::RunPeriodically() {
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

