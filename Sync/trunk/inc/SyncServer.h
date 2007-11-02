#ifndef KITCHENSYNCSERVER_H_
#define KITCHENSYNCSERVER_H_

#include <e32base.h>
#include "KitchenSyncData.h"

const TUint KDefaultHeapSize=0x10000;
const TUint KDefaultMessageSlots=4;

_LIT(KConfigFileName, "config.dat");
_LIT(KKitchenSyncServerName, "KitchenSyncServer");

enum TKitchenSyncServerPanic
{
	EBadRequest,
	EGetTimer,
	ESetTimer,
	EMainSchedulerError,
	ECreateTrapCleanup,
	EStartServer
};

class CKitchenSyncServer : public CServer2
{
public:
	CKitchenSyncServer(CActive::TPriority aActiveObjectPriority);
	CSession2 * NewSessionL(const TVersion& aVersion,
	const RMessage2& aMessage) const;
	static TInt ThreadFunction(TAny *aParam);
	static void PanicServer(TKitchenSyncServerPanic aPanic);
public:
	void SetTimer(TSmlProfileId profileId, TKitchenSyncPeriod period);
	TKitchenSyncPeriod GetTimer(TSmlProfileId profileId);

public:
	void LoadSettingsL();
	void SaveSettings();
	
private:
	void RunL();
	RArray<CKitchenSyncData> timerArray;
};

#endif /*KITCHENSYNCSERVER_H_*/
