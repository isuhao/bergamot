#include "CPodcastServerSession.h"
#include "e32debug.h"
#include "PodcastData.h"

void CPodcastServerSession::ServiceL(const RMessage2& aMessage) 
{
	CPodcastServer* srv = (CPodcastServer*) Server();
	switch(aMessage.Function()) {
	case ESetTimer: {
		break;
	}
	case EGetTimer: {
		break;
	}
	};
	aMessage.Complete(KErrNone);
}

void CPodcastServerSession::PanicClient(const RMessage2& aMessage,TInt aPanic) const
{
	_LIT(KTxtServer,"Podcast Server");
	RDebug::Print(_L("PanicClient"));
	aMessage.Panic(KTxtServer,aPanic);
}
