#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"

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
	iActiveShowList.Close();
}

CPodcastModel::CPodcastModel()
{
}

void CPodcastModel::ConstructL()
{
	iFeedEngine = CFeedEngine::NewL();
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
	
CSoundEngine& CPodcastModel::SoundEngine()
{
	return *iSoundEngine;
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

		RDebug::Print(_L("Starting: %S"), &(aPodcast->iFileName));
		TRAPD(error, iSoundEngine->OpenFileL(aPodcast->iFileName));
	    if (error != KErrNone) {
	    	RDebug::Print(_L("Error: %d"), error);
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
