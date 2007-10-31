#include "KitchenSyncServer.h"
#include "CKitchenSyncServerSession.h"
#include "RKitchenSyncServerSession.h"
#include "KitchenSyncData.h"
#include <f32file.h>
#include <BAUTILS.H>
#include <S32FILE.H>
#include <e32base.h>
CKitchenSyncServer::CKitchenSyncServer(CActive::TPriority aActiveObjectPriority) 
	: CServer2(aActiveObjectPriority) { }

CSession2* CKitchenSyncServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
{
	return new (ELeave) CKitchenSyncServerSession;
}

void CKitchenSyncServer::PanicServer(TKitchenSyncServerPanic aPanic)
{
    _LIT(KTxtServerPanic,"Sync server panic");
    User::Panic(KTxtServerPanic,aPanic);
}

TInt CKitchenSyncServer::ThreadFunction(TAny * /*aStarted */)
{
	CTrapCleanup* cleanup=CTrapCleanup::New();
	if (cleanup == NULL)
	 {
	    CKitchenSyncServer::PanicServer(ECreateTrapCleanup);
	 }

	CActiveScheduler *pA=new CActiveScheduler;
	__ASSERT_ALWAYS(pA!=NULL,CKitchenSyncServer::PanicServer(EMainSchedulerError));
	
	CKitchenSyncServer *pS = new CKitchenSyncServer(EPriorityStandard);
	
	CActiveScheduler::Install(pA);
	
	TInt err = pS->Start(KKitchenSyncServerName);
    if (err != KErrNone)
    {
           CKitchenSyncServer::PanicServer(EStartServer);
    }
    TRAPD(error, pS->LoadSettingsL());
	RThread::Rendezvous(KErrNone);
	CActiveScheduler::Start();
	RDebug::Print(_L("CActiveScheduler::Start done"));
    delete pS;
    delete pA;
    delete cleanup;
    
    return KErrNone;
}

void CKitchenSyncServer::LoadSettingsL() {
	RDebug::Print(_L("LoadSettings"));
	RFs fsSession;
	int error = fsSession.Connect(); 
	RDebug::Print(_L("RFs error: %d"), error);
	TFileName path;
	TParse	filestorename;
	if (!BaflUtils::FileExists(fsSession, KFullNameOfFileStore)) {
		RDebug::Print(_L("No config file"));	
		return;
	}
	
	error = fsSession.Parse(KFullNameOfFileStore,filestorename);
	RDebug::Print(_L("Parse error: %d"), error);
	CFileStore* store;
	TRAP(error, store = CDirectFileStore::OpenL(fsSession,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		fsSession.Close();
		CleanupStack::Pop(store);
		return;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	CKitchenSyncData readData;
	
	TRAP(error, instream  >> readData);	
	while (error==0) {
		// iterate over system profiles
		timerArray.Append(readData);
		RDebug::Print(_L("Read settings: profile %d with period %d"), readData.profileId, readData.period);
		SetTimer(readData.profileId, readData.period);
		TRAP(error, instream  >> readData);
	}
	
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);
}

void CKitchenSyncServer::SaveSettings() 
{
	RDebug::Print(_L("SaveSettings"));
	RFs fsSession;
	fsSession.Connect(); 
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	RDebug::Print(_L("Folder: %S"), &KDirNameOfFileStore);
	RDebug::Print(_L("File: %S"), &KFullNameOfFileStore);
	fsSession.Parse(KFullNameOfFileStore, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(fsSession, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	for (int i=0;i<timerArray.Count();i++) {
		RDebug::Print(_L("Storing account %i"), i);
		outstream  << timerArray[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	fsSession.Close();
	
	CleanupStack::Pop(store);
}

void CKitchenSyncServer::SetTimer(TSmlProfileId profileId, TKitchenSyncPeriod period)
	{
	CKitchenSyncData* sData = NULL;
	RDebug::Print(_L("SetTimer for profile %d"), profileId);
	
	for (int i=0;i<timerArray.Count();i++) {
		if (timerArray[i].profileId == profileId) {
			RDebug::Print(_L("Found profile %d"), profileId);
			sData = &timerArray[i];
		}
	}
	
	if (sData == NULL) {
		RDebug::Print(_L("Creating new sync data for profile %d"), profileId);
		sData = new CKitchenSyncData();
		sData->timer = new CKitchenSyncTimer(profileId);
		sData->timer->ConstructL();
		sData->profileId = profileId;
		sData->period = period;
		if (period != ENever) {
			RDebug::Print(_L("Starting timer for profile %d, period=%d"), profileId, period);
			sData->timer->SetPeriod(period);
			sData->timer->RunPeriodically();
		}
		timerArray.Append(*sData);
	} else {
		if (sData->timer == NULL) {
			RDebug::Print(_L("Creating new timer for profile %d"), profileId);
			sData->timer = new CKitchenSyncTimer(profileId);
			sData->timer->ConstructL();
		} else if (sData->timer != NULL) {
			RDebug::Print(_L("Stopping existing timer for profile %d"), profileId);
			sData->timer->Cancel();
		}

		sData->profileId = profileId;
		sData->period = period;

		if (period != ENever) {
			RDebug::Print(_L("Starting timer for profile %d, period=%d"), profileId, period);
			sData->timer->SetPeriod(period);
			sData->timer->RunPeriodically();
		}

	}

	SaveSettings();
}

TKitchenSyncPeriod CKitchenSyncServer::GetTimer(TSmlProfileId profileId)
	{
	for (int i=0;i<timerArray.Count();i++)
		{
			if (timerArray[i].profileId == profileId) {
				RDebug::Print(_L("Found profile %d with period %d"), profileId, timerArray[i].period);
				return (TKitchenSyncPeriod) timerArray[i].period;
			}
		}
	
	return ENoPeriod;
	}

void RunServer()
{
		
	CKitchenSyncServer x(CActive::EPriorityIdle);
	x.ThreadFunction(NULL);
	
	
/*	RThread serverThread;
	RDebug::Print(_L("Starting server thread"));
	_LIT(KKitchenSyncServerName, "KitchenSyncServer");
	serverThread.Create(KKitchenSyncServerName, x.ThreadFunction,
			KDefaultStackSize, KDefaultHeapSize, KDefaultHeapSize, NULL);
	
	TRequestStatus rdvzStatus;
	
	serverThread.Rendezvous(rdvzStatus);
	serverThread.Resume();
	RDebug::Print(_L("Server thread running"));*/
	RSemaphore s;
	int error = s.OpenGlobal(KKitchenSyncServerSemaphore);
	if (error == KErrNone) {
		RDebug::Print(_L("Signalling semaphore"));
		s.Signal();
	}
	s.Close();
	User::WaitForAnyRequest();
}

GLDEF_C TInt E32Main()
{
	RDebug::Print(_L("KitchenSyncServer: Starting"));
	CTrapCleanup* cleanup=CTrapCleanup::New(); 
	TRAPD(error,RunServer()); 
	delete cleanup; 
	RDebug::Print(_L("KitchenSyncServer: Exiting, error=%d"), error);	
	return 0;
}

