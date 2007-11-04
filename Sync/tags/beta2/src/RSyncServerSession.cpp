#include "RSyncServerSession.h"
#include "e32debug.h"

TInt RSyncServerSession::Connect()
{
	RDebug::Print(_L("RSyncServerSession::Connect"));
	int error = CreateSession(KSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
	
	if (error == KErrNone) {
		RDebug::Print(_L("Successfully connected to sync server"));
	} else {
		RDebug::Print(_L("Failed to connect to sync server, attempting to start"));
		RProcess server;

		RSemaphore s;
		s.CreateGlobal(KSyncServerSemaphore, 0);
		
		error = server.Create(KSyncServerExe, TPtr(NULL,0));
		if (error == KErrNone) {
			RDebug::Print(_L("Successfully started server"));
			server.Resume();			
	    	s.Wait();
			RDebug::Print(_L("Got semaphore"));

			error = CreateSession(KSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
				
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
	
	RDebug::Print(_L("RSyncServerSession::Connect returning %d"), error);
	return error;
}


void RSyncServerSession::SetTimer(TSmlProfileId profileId, TSyncServerPeriod period)
	{
	RDebug::Print(_L("RSyncServerSession::SetTimer"));
	TIpcArgs args(profileId, period);
	SendReceive(ESetTimer, args);
	
	}

TSyncServerPeriod RSyncServerSession::GetTimer(TSmlProfileId profileId)
	{
	RDebug::Print(_L("RSyncServerSession::GetTimer, profileId=%d"), profileId);
    TPckgBuf<TInt> pckg;
    TIpcArgs args(profileId, &pckg);
	
	SendReceive(EGetTimer, args);
	
	TInt res = pckg();
	RDebug::Print(_L("RSyncServerSession::GetTimer returning %d"), res);
	return (TSyncServerPeriod) res;
	}
