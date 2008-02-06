#include "FeedTimer.h"
#include "e32std.h"
#include <eikenv.h>
#include "FeedEngine.h"

CFeedTimer::CFeedTimer(CFeedEngine *aFeedEngine) : CTimer(EPriorityIdle), iFeedEngine(aFeedEngine) {

}

CFeedTimer::~CFeedTimer() {
}

void CFeedTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CFeedTimer::RunL() {
	RDebug::Print(_L("FeedTimer RunL"));

	iFeedEngine->UpdateAllFeeds();
	// run again

	RunPeriodically();
}

void CFeedTimer::SetPeriod(int aPeriodMinutes) {
	RDebug::Print(_L("Setting sync period to %d"), aPeriodMinutes);
	iPeriodMinutes = aPeriodMinutes;
}

void CFeedTimer::SetSyncTime(TTime aTime) {
	TTime time;
	time.HomeTime();

	RDebug::Print(_L("Now is %4d-%02d-%02d, %02d:%02d"), time.DateTime().Year(), time.DateTime().Month()+1, time.DateTime().Day()+1, time.DateTime().Hour(), time.DateTime().Minute());

	int hour = aTime.DateTime().Hour();
	int minute = aTime.DateTime().Minute();
	
	
	TDateTime dTime;
	
	dTime.Set(time.DateTime().Year(), time.DateTime().Month(),
			time.DateTime().Day(),aTime.DateTime().Hour(),
			aTime.DateTime().Minute(), 0, 0);

	TTimeIntervalMinutes tmi = 0;

	// if this time already passed, add one day
	if (time.DateTime().Hour() > hour || 
			time.DateTime().Hour() == hour && time.DateTime().Minute() > minute) {
			RDebug::Print(_L("Adding one day"));
			tmi = 60*24;
			}

	
	TTime atTime(dTime);
	atTime = atTime + tmi;
	RDebug::Print(_L("Setting sync timer to %4d-%02d-%02d, %02d:%02d"), atTime.DateTime().Year(), atTime.DateTime().Month()+1, atTime.DateTime().Day()+1, atTime.DateTime().Hour(), atTime.DateTime().Minute());

	TRAPD(error,At(atTime));
	
	if (error != KErrNone) {
		RDebug::Print(_L("Error from At: %d"), error);
	}
	
}

void CFeedTimer::RunPeriodically() {
	RDebug::Print(_L("RunPeriodically; thePeriod=%d"), iPeriodMinutes);
	TTime time;
	time.UniversalTime();

	TTimeIntervalMinutes tmi;
	//TTimeIntervalSeconds tmi;
	tmi = iPeriodMinutes;
	time = time + tmi;
	RDebug::Print(_L("Running timer"));

	TRAPD(error,AtUTC(time));
	
	if (error != KErrNone) {
		RDebug::Print(_L("Error from AtUTC: %d"), error);
	}
}

