#ifndef PODCASTSERVERTIMER_H
#define PODCASTSERVERTIMER_H

#include <e32base.h>

class CPodcastServerTimer : public CTimer {
public:
	CPodcastServerTimer();
	~CPodcastServerTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
private:
	int thePeriod;
};
#endif

