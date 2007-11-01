#ifndef RKITCHENSYNCSERVERSESSION_H_
#define RKITCHENSYNCSERVERSESSION_H_

#include <e32std.h>
#include "CKitchenSyncServerSession.h"

_LIT(KKitchenSyncServerExe, "\\sys\\bin\\KitchenSyncServer.exe");
_LIT(KKitchenSyncServerSemaphore, "KSSSemaphore");

class RKitchenSyncServerSession : public RSessionBase
{
public:
	TInt Connect();
	void SetTimer(TSmlProfileId profileId, TKitchenSyncPeriod period);
	TKitchenSyncPeriod GetTimer(TSmlProfileId profileId);
	
private:
	RThread iServerThread;
};

#endif /*KITCHENSYNCSERVERSESSION_H_*/
