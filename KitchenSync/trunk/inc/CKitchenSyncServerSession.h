#ifndef CKITCHENSYNCSERVERSESSION_H_
#define CKITCHENSYNCSERVERSESSION_H_

#include "KitchenSyncServer.h"
#include <e32base.h>

enum TKitchenSyncServerMessage {
	ETest
};

class CKitchenSyncServerSession : public CSession2
{
public:
	void ServiceL(const RMessage2& aMessage);
	void PanicClient(const RMessage2& aMessage,TInt aPanic) const;
	
private:
	const CKitchenSyncServer *iServer;
};

#endif /*CKITCHENSYNCSERVERSESSION_H_*/
