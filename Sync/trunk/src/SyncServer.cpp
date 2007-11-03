#include "SyncServer.h"
#include "CSyncServerSession.h"
#include "RSyncServerSession.h"
#include "SyncServerData.h"
#include <f32file.h>
#include <BAUTILS.H>
#include <S32FILE.H>
#include <e32base.h>

CSyncServer::CSyncServer(CActive::TPriority aActiveObjectPriority) 
	: CServer2(aActiveObjectPriority) { }

CSession2* CSyncServer::NewSessionL(const TVersion& /*aVersion*/, const RMessage2& /*aMessage*/) const
{
	return new (ELeave) CSyncServerSession;
}

void CSyncServer::PanicServer(TSyncServerPanic aPanic)
{
    _LIT(KTxtServerPanic,"Sync server panic");
    User::Panic(KTxtServerPanic,aPanic);
}

void CSyncServer::RunL() {
	CServer2::RunL();
}
TInt CSyncServer::ThreadFunction(TAny * /*aStarted */)
{
	CTrapCleanup* cleanup=CTrapCleanup::New();
	if (cleanup == NULL)
	 {
	    CSyncServer::PanicServer(ECreateTrapCleanup);
	 }

	CActiveScheduler *pA=new CActiveScheduler;
	__ASSERT_ALWAYS(pA!=NULL,CSyncServer::PanicServer(EMainSchedulerError));
	CActiveScheduler::Install(pA);
	
	CSyncServer *pS = new CSyncServer(EPriorityHigh);
	
    TRAPD(error, pS->LoadSettingsL());	
    if (error != KErrNone)
    {
        	RDebug::Print(_L("LoadSettingsL error %d"), error);
            CSyncServer::PanicServer(EStartServer);
    }

    error = pS->Start(KSyncServerName);
    if (error != KErrNone)
    {
    	RDebug::Print(_L("pS->Start error"));
        CSyncServer::PanicServer(EStartServer);
    }
    
    
    RSemaphore s;
	error = s.OpenGlobal(KSyncServerSemaphore);
	if (error == KErrNone) {
		RDebug::Print(_L("Signalling semaphore"));
		s.Signal();
	}
	s.Close();
	
    //User::RenameThread(KSyncServerName);
    //RDebug::Print(_L("Rendezvous"));
   
    //RProcess::Rendezvous(KErrNone);

	RDebug::Print(_L("CActiveScheduler::Start"));
	CActiveScheduler::Start();
	RDebug::Print(_L("CActiveScheduler::Start done"));
    delete pS;
    delete pA;
    delete cleanup;
    
    return KErrNone;
}

void CSyncServer::LoadSettingsL() {
	RDebug::Print(_L("LoadSettings"));
	RFs fsSession;
	int error = fsSession.Connect(); 
	RDebug::Print(_L("RFs error: %d"), error);
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	privatePath.Append(KConfigFileName);
	RDebug::Print(_L("File: %S"), &privatePath);
	fsSession.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(fsSession, privatePath)) {
		RDebug::Print(_L("No config file"));	
		return;
	}
	
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

	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	for (int i=0;i<count;i++) {
		CSyncServerData readData;
		instream  >> readData;	
		timerArray.Append(readData);
		RDebug::Print(_L("Read settings %d: profile %d with period %d"), i, readData.profileId, readData.period);
		SetTimer(readData.profileId, readData.period);
	}
	RDebug::Print(_L("Read all accounts"));
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);
}

void CSyncServer::SaveSettings() 
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
	privatePath.Append(KConfigFileName);
	RDebug::Print(_L("File: %S"), &privatePath);
	fsSession.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(fsSession, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	outstream.WriteInt32L(timerArray.Count());
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

void CSyncServer::SetTimer(TSmlProfileId profileId, TSyncServerPeriod period)
	{
	CSyncServerData* sData = NULL;
	RDebug::Print(_L("SetTimer for profile %d"), profileId);
	
	for (int i=0;i<timerArray.Count();i++) {
		if (timerArray[i].profileId == profileId) {
			RDebug::Print(_L("Found profile %d"), profileId);
			sData = &timerArray[i];
		}
	}
	
	if (sData == NULL) {
		RDebug::Print(_L("Creating new sync data for profile %d"), profileId);
		sData = new CSyncServerData();
		sData->timer = new CSyncServerTimer(profileId);
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
			sData->timer = new CSyncServerTimer(profileId);
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
}

TSyncServerPeriod CSyncServer::GetTimer(TSmlProfileId profileId)
	{
	for (int i=0;i<timerArray.Count();i++)
		{
			if (timerArray[i].profileId == profileId) {
				RDebug::Print(_L("Found profile %d with period %d"), profileId, timerArray[i].period);
				return (TSyncServerPeriod) timerArray[i].period;
			}
		}
	
	return ENoPeriod;
}

GLDEF_C TInt E32Main()
{
	RDebug::Print(_L("SyncServer: Starting"));

	CTrapCleanup* cleanup=CTrapCleanup::New();
	TInt r=KErrNoMemory;
	if (cleanup)
		{
		TRAP(r,CSyncServer::ThreadFunction(NULL););
		delete cleanup;
		}
	__UHEAP_MARKEND;
	RDebug::Print(_L("SyncServer: Exiting, error=%d"), r);	
	return r;
}

