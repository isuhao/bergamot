#include <qikon.hrh>
#include <commdb.h>
#include <podcastclient.rsg>
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include "PodCastClientSettingsDlg.h"

const TInt KDefaultGranu = 5;

CPodcastModel* CPodcastModel::NewL()
{
	CPodcastModel* self = new (ELeave) CPodcastModel;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CPodcastModel::~CPodcastModel()
{
	delete iFeedEngine;
	delete iSoundEngine;
	delete iSettingsEngine;
	iActiveShowList.Close();
	delete iIapNameArray;
	iIapIdArray.Close();
	delete iCommDB;
}

CPodcastModel::CPodcastModel()
{
	 iZoomState =  EQikCmdZoomLevel2;
}

void CPodcastModel::ConstructL()
{
	iEnv = CEikonEnv::Static();
	iCommDB = CCommsDatabase::NewL (EDatabaseTypeUnspecified);
	//iCommDB ->ShowHiddenRecords(); // magic
	iIapNameArray = new (ELeave) CDesCArrayFlat(KDefaultGranu);

	UpdateIAPListL();

	iSettingsEngine = CSettingsEngine::NewL(*this);
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);
	iSoundEngine = CSoundEngine::NewL(*this);
	
	User::LeaveIfError(iSocketServ.Connect());
	User::LeaveIfError(iConnection.Open(iSocketServ));
}

void CPodcastModel::UpdateIAPListL()
{
	iIapNameArray->Reset();
	iIapIdArray.Reset();
    //TUint32 bearerset = KCommDbBearerWcdma | KCommDbBearerWLAN | KCommDbBearerLAN;
    //TCommDbConnectionDirection connDirection = ECommDbConnectionDirectionOutgoing;

    //CCommsDbTableView* table = iCommDB->OpenIAPTableViewMatchingBearerSetLC(bearerset, connDirection);

	CCommsDbTableView* table = iCommDB->OpenTableLC (TPtrC (IAP)); 
	TInt ret = table->GotoFirstRecord ();
	TPodcastIAPItem IAPItem;
	TBuf <KCommsDbSvrMaxFieldLength> bufName;
	while (ret == KErrNone) // There was a first record
	{
		table->ReadUintL(TPtrC(COMMDB_ID), IAPItem.iIapId);
		table->ReadTextL (TPtrC(COMMDB_NAME), bufName);
		table->ReadTextL (TPtrC(IAP_BEARER_TYPE), IAPItem.iBearerType);
		table->ReadTextL (TPtrC(IAP_SERVICE_TYPE), IAPItem.iServiceType);

		iIapIdArray.Append(IAPItem);
		iIapNameArray->AppendL(bufName); 
		//iIapNameArray->AppendL(IAPItem.iBearerType); 
		ret = table->GotoNextRecord();
	}
	CleanupStack::PopAndDestroy(); // Close table
}

CDesCArrayFlat* CPodcastModel::IAPNames()
{
	return iIapNameArray;
}

RArray<TPodcastIAPItem>& CPodcastModel::IAPIds()
{
	return iIapIdArray;
}



CEikonEnv* CPodcastModel::EikonEnv()
{
	return iEnv;
}

void CPodcastModel::SetPlayingPodcast(CShowInfo* aPodcast)
{
	iPlayingPodcast = aPodcast;
}

CShowInfo* CPodcastModel::PlayingPodcast()
{
	return iPlayingPodcast;
}

CFeedEngine& CPodcastModel::FeedEngine()
{
	return *iFeedEngine;
}
	
CShowEngine& CPodcastModel::ShowEngine()
{
	return *iShowEngine;
}

CSoundEngine& CPodcastModel::SoundEngine()
{
	return *iSoundEngine;
}

CSettingsEngine& CPodcastModel::SettingsEngine()
{
	return *iSettingsEngine;
}

void CPodcastModel::PlayPausePodcastL(CShowInfo* aPodcast) 
	{
	
	// special treatment if this podcast is already active
	if (iPlayingPodcast == aPodcast && SoundEngine().State() > ESoundEngineNotInitialized ) {
		if (aPodcast->PlayState() == EPlaying) {
			SoundEngine().Pause();
			aPodcast->SetPosition(iSoundEngine->Position());
			aPodcast->SetPlayState(EPlayed);
		} else {
			iSoundEngine->Play();
		}
	} else {
		// switching file, so save position
		iSoundEngine->Pause();
		if (iPlayingPodcast != NULL) {
			iPlayingPodcast->SetPosition(iSoundEngine->Position());
		}
		
		iSoundEngine->Stop();

		if(aPodcast != NULL) {
			RDebug::Print(_L("Starting: %S"), &(aPodcast->FileName()));
			TRAPD(error, iSoundEngine->OpenFileL(aPodcast->FileName()));
			if (error != KErrNone) {
				RDebug::Print(_L("Error: %d"), error);
			}
		}

		iPlayingPodcast = aPodcast;		
	}
}

CFeedInfo* CPodcastModel::ActiveFeedInfo()
{
	return iActiveFeed;
}

void CPodcastModel::SetActiveFeedInfo(CFeedInfo* aFeedInfo)
{
	iActiveFeed = aFeedInfo;
}

RShowInfoArray& CPodcastModel::ActiveShowList()
{
	return iActiveShowList;
}

void CPodcastModel::SetActiveShowList(RShowInfoArray& aShowArray)
{
	iActiveShowList.Reset();
	TInt cnt = aShowArray.Count();

	for(TInt loop = 0;loop < cnt; loop++)
	{
		iActiveShowList.Append(aShowArray[loop]);
	}
}


TBool CPodcastModel::SetZoomState(TInt aZoomState)
{
	if(iZoomState != aZoomState)
	{
		iZoomState = aZoomState;
		return ETrue;
	}

	return EFalse;
}

TInt CPodcastModel::ZoomState()
{
	return iZoomState;
}

RConnection& CPodcastModel::Connection()
{
	return iConnection;
}

TConnPref& CPodcastModel::ConnPref()
{
	return iConnPref;
}

void CPodcastModel::ConnectHttpSessionL(RHTTPSession &aSession)
{
	RDebug::Print(_L("ConnectHttpSessionL START"));
	iConnection.Stop();
	if(iSettingsEngine->SpecificIAP() == -1)
	{
		TInt iapSelected = iConnPref.IapId();
		CPodcastClientIAPDlg* selectIAPDlg = new (ELeave) CPodcastClientIAPDlg(*this, iapSelected);
		if(selectIAPDlg->ExecuteLD(R_PODCAST_IAP_DLG))
		{
			iConnPref.SetIapId(iapSelected);
		}
		else
		{
			return;
		}
	}

	User::LeaveIfError(iConnection.Start(iConnPref));

	RHTTPConnectionInfo connInfo = aSession.ConnectionInfo();
	RStringPool pool = aSession.StringPool();
	// Attach to socket server
	connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable()), THTTPHdrVal(iSocketServ.Handle()));
	// Attach to connection
	TInt connPtr = REINTERPRET_CAST(TInt, &iConnection);
	connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()), THTTPHdrVal(connPtr));
	RDebug::Print(_L("ConnectHttpSessionL END"));
}

void CPodcastModel::SetIap(TInt aIap)
	{
	if (aIap == -1) {
		RDebug::Print(_L("Will prompt"));
		iConnPref.SetDialogPreference(ECommDbDialogPrefPrompt);
		iConnPref.SetDirection(ECommDbConnectionDirectionOutgoing);
		iConnPref.SetIapId(0);
	} else {
		iConnPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
		iConnPref.SetDirection(ECommDbConnectionDirectionOutgoing);
		iConnPref.SetIapId(aIap);
	}
	
	}
