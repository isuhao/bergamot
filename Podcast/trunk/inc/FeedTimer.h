#ifndef FEEDTIMER_H
#define FEEDTIMER_H

#include <e32base.h>


class CFeedEngine;

class CFeedTimer : public CTimer {
public:
	CFeedTimer(CFeedEngine *aFeedEngine);
	~CFeedTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int aPeriodMinutes);
	
private:
	int iPeriodMinutes;
	CFeedEngine *iFeedEngine;
};
#endif

