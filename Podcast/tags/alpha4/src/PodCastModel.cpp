#include <qikon.hrh>

#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"

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
}

CPodcastModel::CPodcastModel()
{
	 iZoomState =  EQikCmdZoomLevel2;
}

void CPodcastModel::ConstructL()
{
	iEnv = CEikonEnv::Static();
	iSettingsEngine = CSettingsEngine::NewL(*this);
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);
	iSoundEngine = CSoundEngine::NewL(*this);
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

CShowInfoArray& CPodcastModel::ActiveShowList()
{
	return iActiveShowList;
}

void CPodcastModel::SetActiveShowList(CShowInfoArray& aShowArray)
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
