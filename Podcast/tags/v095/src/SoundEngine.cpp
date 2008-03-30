#include <e32std.h>
#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "PodcastModel.h"
#include "ShowInfo.h"
#include "ShowEngine.h"

CSoundEngine* CSoundEngine::NewL(CPodcastModel& aPodcastModel)
{
	CSoundEngine* self = new (ELeave) CSoundEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CSoundEngine::~CSoundEngine()
{
	delete iPlayer;
}

CSoundEngine::CSoundEngine(CPodcastModel& aPodcastModel): iPodcastModel(aPodcastModel)
{
}

void CSoundEngine::ConstructL()
{
    iPlayer = CMdaAudioPlayerUtility::NewL(*this);
}

void CSoundEngine::SetObserver(MSoundEngineObserver* aObserver)
{
	iObserver = aObserver;
}

void CSoundEngine::MapcPlayComplete(TInt aError) {
	RDebug::Print(_L("MapcPlayComplete: %d"), aError);
		
	if (iPodcastModel.PlayingPodcast() != NULL) {
		if(aError == KErrNone) // normal ending
			{
			iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
			iPodcastModel.PlayingPodcast()->SetPosition(0);
			}
	}

	iState = ESoundEngineStopped;
	if(iObserver != NULL)
	{
		TRAPD(err, iObserver->PlaybackStoppedL());
	}

	if(iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->ShowType() == EAudioBook)
	{
		CShowInfo* nextShow = iPodcastModel.ShowEngine().GetNextShowByTrackL(iPodcastModel.PlayingPodcast());
		if(nextShow != NULL)
		{
			iPodcastModel.PlayPausePodcastL(nextShow, ETrue);
		}
	}
}	

void CSoundEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	if (aError != KErrNone)
		{
		RDebug::Print(_L("MapcInitComplete error=%d"), aError);	
		
		iState = ESoundEngineNotInitialized;		
		}
	else
		{
		iState = ESoundEngineStopped;
		
		iPlayer->SetVolume((iPodcastModel.SettingsEngine().Volume() * iPlayer->MaxVolume()) / 100);
		
		if (iPodcastModel.PlayingPodcast() != NULL) 
			{
			RDebug::Print(_L("Resuming from position: %ld"), iPodcastModel.PlayingPodcast()->Position().Int64());
			iPodcastModel.PlayingPodcast()->SetPlayTime(iPlayer->Duration().Int64()/1000000);
			iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->Position());			
			}
		}

	if(iObserver != NULL && aError == KErrNone)
		{
		TRAPD(err, iObserver->PlaybackInitializedL());
		if(iPlayOnInit)
			{
			
			iPodcastModel.PlayPausePodcastL(iPodcastModel.PlayingPodcast());
			}
		}

	iPlayOnInit = EFalse;
}

void CSoundEngine::OpenFileL(const TDesC& aFileName, TBool aPlayOnInit)
{
	iState = ESoundEngineNotInitialized;
	iLastOpenedFileName= aFileName;
	iPlayer->Stop();
	iPlayer->OpenFileL(aFileName);
	iPlayOnInit = aPlayOnInit;
	iState = ESoundEngineOpening;
}

const TFileName& CSoundEngine::LastFileName()
{
	return iLastOpenedFileName;
}

TTimeIntervalMicroSeconds CSoundEngine::Position()
{
	TTimeIntervalMicroSeconds pos = 0;
	
	if(iState > ESoundEngineOpening)
	{
		iPlayer->GetPosition(pos);
	}

	return pos;
}

void CSoundEngine::SetPosition(TUint aPos)
{
	if(iState > ESoundEngineOpening)
	{
		TTimeIntervalMicroSeconds pos = aPos*1000000;
		if(iState == ESoundEnginePlaying)
		{
			iPlayer->Pause();
		}

		iPlayer->SetPosition(pos);
		
		if(iState == ESoundEnginePlaying)
		{
			iPlayer->Play();
		}
	}
}


TUint CSoundEngine::PlayTime()
{
	if(iState > ESoundEngineOpening)
	{
		return iPlayer->Duration().Int64()/1000000;
	}

	return 0;
}


void CSoundEngine::Play()
{
	if(iState > ESoundEngineOpening)
	{
		iPlayer->Play();
		iState = ESoundEnginePlaying;

		// better to do this when the file finishes playing
//		if (iPodcastModel.PlayingPodcast() != NULL) {
//			iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
//		}

		if(iObserver != NULL)
		{
			TRAPD(err, iObserver->PlaybackStartedL());
		}
	}
}

void CSoundEngine::Stop()
{
	if(iState > ESoundEngineOpening)
	{
		// seem to need to do this here, even though we do it in MapcPlayComplete
		iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);

		iState = ESoundEngineStopped;
		SetPosition(0);
		iPlayer->Stop();
	}
}

void CSoundEngine::Pause()
{
	RDebug::Print(_L("Pause"));
	if(iState > ESoundEngineOpening)
	{
		iState = ESoundEnginePaused;
		iPlayer->Pause();
		TTimeIntervalMicroSeconds pos = 0;
		iPlayer->GetPosition(pos);
		RDebug::Print(_L("Pos: %lu"), pos.Int64());
		// had a crash here, so we check for NULL first
		if (iPodcastModel.PlayingPodcast() != NULL) {
			RDebug::Print(_L("Setting position..."));
			iPodcastModel.PlayingPodcast()->SetPosition(pos);
			// really wasteful saving EVERYTHING every time
			iPodcastModel.ShowEngine().SaveShows();
		}
	}
}

TSoundEngineState CSoundEngine::State()
{
	return iState;
}

void CSoundEngine::SetVolume(TUint aVolume)
{
	if(iState > ESoundEngineOpening)
		{
			iPlayer->SetVolume((aVolume*iPlayer->MaxVolume()) / 100);
		}
}

