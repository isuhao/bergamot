#ifndef SYNCSERVERTIMER_H
#define SYNCSERVERTIMER_H

#include <e32base.h>
#include "SyncMLDef.h"

class CSyncServerTimer : public CTimer {
public:
	CSyncServerTimer(TSmlProfileId anId);
	~CSyncServerTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
private:
	TSmlProfileId profileId;
	int thePeriod;
};
#endif

