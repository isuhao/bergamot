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
	iSettingsEngine = CSettingsEngine::NewL(*this);
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);
	iSoundEngine = CSoundEngine::NewL(*this);
}

void CPodcastModel::SetPlayingPodcast(TShowInfo* aPodcast)
{
	iPlayingPodcast = aPodcast;
}

TShowInfo* CPodcastModel::PlayingPodcast()
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

void CPodcastModel::PlayPausePodcastL(TShowInfo* aPodcast) 
	{
	
	// special treatment if this podcast is already active
	if (iPlayingPodcast == aPodcast && SoundEngine().State() > ESoundEngineNotInitialized ) {
		if (aPodcast->iPlayState == EPlaying) {
			SoundEngine().Pause();
			aPodcast->iPosition = iSoundEngine->Position();
			aPodcast->iPlayState = EPlayed;
		} else {
			iSoundEngine->Play();
		}
	} else {
		// switching file, so save position
		iSoundEngine->Pause();
		if (iPlayingPodcast != NULL) {
			iPlayingPodcast->iPosition = iSoundEngine->Position();
			iPlayingPodcast->iPlayState  = EPlayed;
		}
		
		iSoundEngine->Stop();

		if(aPodcast != NULL) {
			RDebug::Print(_L("Starting: %S"), &(aPodcast->iFileName));
			TRAPD(error, iSoundEngine->OpenFileL(aPodcast->iFileName));
			if (error != KErrNone) {
				RDebug::Print(_L("Error: %d"), error);
			}
		}

		iPlayingPodcast = aPodcast;
	}
}

TFeedInfo& CPodcastModel::ActiveFeedInfo()
{
	return iActiveFeed;
}

void CPodcastModel::SetActiveFeedInfo(const TFeedInfo& aFeedInfo)
{
	iActiveFeed = aFeedInfo;
}

TShowInfoArray& CPodcastModel::ActiveShowList()
{
	return iActiveShowList;
}

void CPodcastModel::SetActiveShowList(TShowInfoArray& aShowArray)
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
