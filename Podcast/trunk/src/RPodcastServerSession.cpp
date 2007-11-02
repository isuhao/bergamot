#include "RPodcastServerSession.h"
#include "e32debug.h"

TInt RPodcastServerSession::Connect()
{
	RDebug::Print(_L("RPodcastServerSession::Connect"));
	int error = CreateSession(KPodcastServerName, TVersion(1,0,0), KDefaultMessageSlots);
	
	if (error == KErrNone) {
		RDebug::Print(_L("Successfully connected to sync server"));
	} else {
		RDebug::Print(_L("Failed to connect to sync server, attempting to start"));
		RProcess server;

		RSemaphore s;
		s.CreateGlobal(KPodcastServerSemaphore, 0);
		
		error = server.Create(KPodcastServerExe, TPtr(NULL,0));
		if (error == KErrNone) {
			RDebug::Print(_L("Successfully started server"));
			server.Resume();			
	    	s.Wait();
			RDebug::Print(_L("Got semaphore"));

			error = CreateSession(KPodcastServerName, TVersion(1,0,0), KDefaultMessageSlots);
				
			if (error == KErrNone) {
				RDebug::Print(_L("Successfully connected to sync server"));
			} else {
				return error;
			}
	    	server.Close();
		} else {
			RDebug::Print(_L("Failed to start server"));
		}
		s.Close();	
	}
	
	RDebug::Print(_L("RPodcastServerSession::Connect returning %d"), error);
	return error;
}


void RPodcastServerSession::SetTimer(TSmlProfileId profileId, TPodcastPeriod period)
	{
	RDebug::Print(_L("RPodcastServerSession::SetTimer"));
	TIpcArgs args(profileId, period);
	SendReceive(ESetTimer, args);
	
	}

TPodcastPeriod RPodcastServerSession::GetTimer(TSmlProfileId profileId)
	{
	RDebug::Print(_L("RPodcastServerSession::GetTimer, profileId=%d"), profileId);
    TPckgBuf<TInt> pckg;
    TIpcArgs args(profileId, &pckg);
	
	SendReceive(EGetTimer, args);
	
	TInt res = pckg();
	RDebug::Print(_L("RPodcastServerSession::GetTimer returning %d"), res);
	return (TPodcastPeriod) res;
	}
