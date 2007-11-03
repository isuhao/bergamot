#include "CSyncServerSession.h"
#include "e32debug.h"
#include "SyncServerData.h"

void CSyncServerSession::ServiceL(const RMessage2& aMessage) 
{
	CSyncServer* srv = (CSyncServer*) Server();
	switch(aMessage.Function()) {
	case ESetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		TSyncServerPeriod period = (TSyncServerPeriod) aMessage.Int1();
		RDebug::Print(_L("SetTimer: profileId=%d, period=%d"), profileId, period);
		srv->SetTimer(profileId, period);
		srv->SaveSettings();
		break;
	}
	case EGetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		RDebug::Print(_L("GetTimer: profileId=%d"), profileId);
		TSyncServerPeriod ret = srv->GetTimer(profileId);
	    TPckgBuf<TInt> p(ret);
	    aMessage.WriteL(1,p);
		break;
	}
	};
	aMessage.Complete(KErrNone);
}

void CSyncServerSession::PanicClient(const RMessage2& aMessage,TInt aPanic) const
{
	_LIT(KTxtServer,"Sync server");
	RDebug::Print(_L("PanicClient"));
	aMessage.Panic(KTxtServer,aPanic);
}
