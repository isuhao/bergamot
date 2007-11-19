#include "FeedTimer.h"
#include "e32std.h"
#include <eikenv.h>

CFeedTimer::CFeedTimer() : CTimer::CTimer(EPriorityIdle) {
}

CFeedTimer::~CFeedTimer() {
}

void CFeedTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CFeedTimer::RunL() {
	RDebug::Print(_L("RunL"));
	// run again
	RunPeriodically();
}

void CFeedTimer::SetPeriod(int aPeriod) {
	RDebug::Print(_L("Setting period to %d"), aPeriod);
	thePeriod = aPeriod;
}

void CFeedTimer::RunPeriodically() {
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

