#include <qikon.hrh>
#include <commdb.h>

#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"

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
	iCommDB ->ShowHiddenRecords(); // magic
	iIapNameArray = new (ELeave) CDesCArrayFlat(KDefaultGranu);

	UpdateIAPListL();

	iSettingsEngine = CSettingsEngine::NewL(*this);
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);
	iSoundEngine = CSoundEngine::NewL(*this);
}

void CPodcastModel::UpdateIAPListL()
{
	iIapNameArray->Reset();
	iIapIdArray.Reset();
	CCommsDbTableView* table = iCommDB->OpenTableLC (TPtrC (IAP)); 
	TInt ret = table->GotoFirstRecord ();
	TUint32 iap = 0;
	TBuf <KCommsDbSvrMaxFieldLength> bufName;
	while (ret == KErrNone) // There was a first record
	{
		table->ReadUintL(TPtrC(COMMDB_ID), iap);
		table->ReadTextL (TPtrC(COMMDB_NAME), bufName);
		iIapIdArray.Append(iap);
		iIapNameArray->AppendL(bufName); 
		ret = table->GotoNextRecord();
	}
	CleanupStack::PopAndDestroy(); // Close table
}

CDesCArrayFlat* CPodcastModel::IAPNames()
{
	return iIapNameArray;
}

RArray<TInt>& CPodcastModel::IAPIds()
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
			iPlayingPodcast->SetPlayState(EPlayed);
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
