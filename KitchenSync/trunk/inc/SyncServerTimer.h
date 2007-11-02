#ifndef KITCHENSYNCTIMER_H
#define KITCHENSYNCTIMER_H

#include <e32base.h>
#include "SyncMLDef.h"

class CKitchenSyncTimer : public CTimer {
public:
	CKitchenSyncTimer(TSmlProfileId anId);
	~CKitchenSyncTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
private:
	TSmlProfileId profileId;
	int thePeriod;
};
#endif

