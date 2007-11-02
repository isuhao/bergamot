#ifndef PODCASTSERVERTIMER_H
#define PODCASTSERVERTIMER_H

#include <e32base.h>
#include "SyncMLDef.h"

class CPodcastServerTimer : public CTimer {
public:
	CPodcastServerTimer(TSmlProfileId anId);
	~CPodcastServerTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
private:
	TSmlProfileId profileId;
	int thePeriod;
};
#endif

