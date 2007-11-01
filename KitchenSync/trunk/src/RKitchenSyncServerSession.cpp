#include "RKitchenSyncServerSession.h"
#include "e32debug.h"

TInt RKitchenSyncServerSession::Connect()
{
	RDebug::Print(_L("RKitchenSyncServerSession::Connect"));
	int error = CreateSession(KKitchenSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
	
	if (error == KErrNone) {
		RDebug::Print(_L("Successfully connected to sync server"));
	} else {
		RDebug::Print(_L("Failed to connect to sync server, attempting to start"));
		RProcess server;

		RSemaphore s;
		s.CreateGlobal(KKitchenSyncServerSemaphore, 0);
		
		error = server.Create(KKitchenSyncServerExe, TPtr(NULL,0));
		if (error == KErrNone) {
			RDebug::Print(_L("Successfully started server"));
			server.Resume();			
	    	s.Wait();
			RDebug::Print(_L("Got semaphore"));

			error = CreateSession(KKitchenSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
				
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
	
	RDebug::Print(_L("RKitchenSyncServerSession::Connect returning %d"), error);
	return error;
}


void RKitchenSyncServerSession::SetTimer(TSmlProfileId profileId, TKitchenSyncPeriod period)
	{
	RDebug::Print(_L("RKitchenSyncServerSession::SetTimer"));
	TIpcArgs args(profileId, period);
	SendReceive(ESetTimer, args);
	
	}

TKitchenSyncPeriod RKitchenSyncServerSession::GetTimer(TSmlProfileId profileId)
	{
	RDebug::Print(_L("RKitchenSyncServerSession::GetTimer, profileId=%d"), profileId);
    TPckgBuf<TInt> pckg;
    TIpcArgs args(profileId, &pckg);
	
	SendReceive(EGetTimer, args);
	
	TInt res = pckg();
	RDebug::Print(_L("RKitchenSyncServerSession::GetTimer returning %d"), res);
	return (TKitchenSyncPeriod) res;
	}
