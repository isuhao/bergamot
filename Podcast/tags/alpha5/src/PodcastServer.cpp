#include "PodcastServer.h"
#include "CPodcastServerSession.h"
#include "RPodcastServerSession.h"
#include "PodcastData.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <e32base.h>

CPodcastServer::CPodcastServer(CActive::TPriority aActiveObjectPriority) 
	: CServer2(aActiveObjectPriority) { }

CSession2* CPodcastServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
{
	return new (ELeave) CPodcastServerSession;
}

void CPodcastServer::PanicServer(TPodcastServerPanic aPanic)
{
    _LIT(KTxtServerPanic,"Sync server panic");
    User::Panic(KTxtServerPanic,aPanic);
}

void CPodcastServer::RunL() {
	RDebug::Print(_L("***Server RunL"));
	CServer2::RunL();
}
TInt CPodcastServer::ThreadFunction(TAny * /*aStarted */)
{
	CTrapCleanup* cleanup=CTrapCleanup::New();
	if (cleanup == NULL)
	 {
	    CPodcastServer::PanicServer(ECreateTrapCleanup);
	 }

	CActiveScheduler *pA=new CActiveScheduler;
	__ASSERT_ALWAYS(pA!=NULL,CPodcastServer::PanicServer(EMainSchedulerError));
	CActiveScheduler::Install(pA);
	
	CPodcastServer *pS = new CPodcastServer(EPriorityHigh);
	
    TRAPD(error, pS->LoadSettingsL());	
    if (error != KErrNone)
    {
        	RDebug::Print(_L("LoadSettingsL error %d"), error);
            CPodcastServer::PanicServer(EStartServer);
    }

    error = pS->Start(KPodcastServerName);
    if (error != KErrNone)
    {
    	RDebug::Print(_L("pS->Start error"));
        CPodcastServer::PanicServer(EStartServer);
    }
    
    
    RSemaphore s;
	error = s.OpenGlobal(KPodcastServerSemaphore);
	if (error == KErrNone) {
		RDebug::Print(_L("Signalling semaphore"));
		s.Signal();
	}
	s.Close();
	
    //User::RenameThread(KPodcastServerName);
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

void CPodcastServer::LoadSettingsL() {
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
		CPodcastData readData;
		instream  >> readData;	
		timerArray.Append(readData);
	}
	RDebug::Print(_L("Read all accounts"));
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);
}

void CPodcastServer::SaveSettings() 
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

GLDEF_C TInt E32Main()
{
	RDebug::Print(_L("PodcastServer: Starting"));

	CTrapCleanup* cleanup=CTrapCleanup::New();
	TInt r=KErrNoMemory;
	if (cleanup)
		{
		TRAP(r,CPodcastServer::ThreadFunction(NULL););
		delete cleanup;
		}
	__UHEAP_MARKEND;
	RDebug::Print(_L("PodcastServer: Exiting, error=%d"), r);	
	return r;
}

