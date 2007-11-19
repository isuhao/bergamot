#ifndef FEEDTIMER_H
#define FEEDTIMER_H

#include <e32base.h>

class CFeedTimer : public CTimer {
public:
	CFeedTimer();
	~CFeedTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
private:
	int thePeriod;
};
#endif

