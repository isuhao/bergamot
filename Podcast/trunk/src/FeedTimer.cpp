#include "FeedTimer.h"
#include "e32std.h"
#include <eikenv.h>
#include "FeedEngine.h"

CFeedTimer::CFeedTimer(CFeedEngine *aFeedEngine) : CTimer::CTimer(EPriorityIdle), iFeedEngine(aFeedEngine) {
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
	iFeedEngine->UpdateAllFeeds();
	RunPeriodically();
}

void CFeedTimer::SetPeriod(int aPeriodMinutes) {
	RDebug::Print(_L("Setting period to %d"), aPeriodMinutes);
	iPeriodMinutes = aPeriodMinutes;
}

void CFeedTimer::RunPeriodically() {
	RDebug::Print(_L("RunPeriodically; thePeriod=%d"), iPeriodMinutes);
	TTime time;
	time.UniversalTime();

	//TTimeIntervalMinutes tmi;
	TTimeIntervalSeconds tmi;
	tmi = iPeriodMinutes;
	time = time + tmi;
	RDebug::Print(_L("Running timer"));

	TRAPD(error,AtUTC(time));
	
	if (error != KErrNone) {
		RDebug::Print(_L("Error from AtUTC: %d"), error);
	}
}

