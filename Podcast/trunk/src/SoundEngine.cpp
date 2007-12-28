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
//	RDebug::Print(_L("Play Complete: %d"), aError);
	if (iPodcastModel.PlayingPodcast() != NULL) {
		iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
	}

	iState = ESoundEngineStopped;
	if(iObserver != NULL)
	{
		TRAPD(err, iObserver->PlaybackStoppedL());
	}
}

void CSoundEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	if (aError != KErrNone) {
		RDebug::Print(_L("MapcInitComplete error=%d"), aError);	

		iState = ESoundEngineNotInitialized;

		return;
	}

	iState = ESoundEngineStopped;

	iPlayer->SetVolume((iPodcastModel.SettingsEngine().Volume() * iPlayer->MaxVolume()) / 100);

	if (iPodcastModel.PlayingPodcast() != NULL) {
		RDebug::Print(_L("Resuming from position: %ld"), iPodcastModel.PlayingPodcast()->Position().Int64());
		iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->Position());			
	}

	if(iObserver != NULL)
	{
		TRAPD(err, iObserver->PlaybackInitializedL());
	}
}

void CSoundEngine::OpenFileL(TDesC& aFileName)
{
	iState = ESoundEngineNotInitialized;
	iPlayer->Stop();
	iPlayer->OpenFileL(aFileName);
}

TTimeIntervalMicroSeconds CSoundEngine::Position()
{
	TTimeIntervalMicroSeconds pos = 0;
	
	if(iState > ESoundEngineNotInitialized)
	{
		iPlayer->GetPosition(pos);
	}

	return pos;
}

void CSoundEngine::SetPosition(TUint aPos)
{
	if(iState > ESoundEngineNotInitialized)
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
	if(iState > ESoundEngineNotInitialized)
	{
		return iPlayer->Duration().Int64()/1000000;
	}

	return 0;
}


void CSoundEngine::Play()
{
	if(iState > ESoundEngineNotInitialized)
	{
		iPlayer->Play();
		iState = ESoundEnginePlaying;

		if (iPodcastModel.PlayingPodcast() != NULL) {
			iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
		}

		if(iObserver != NULL)
		{
			TRAPD(err, iObserver->PlaybackStartedL());
		}
	}
}

void CSoundEngine::Stop()
{
	if(iState > ESoundEngineNotInitialized)
	{
		iState = ESoundEngineStopped;
		SetPosition(0);
		iPlayer->Stop();
	}
}

void CSoundEngine::Pause()
{
	if(iState > ESoundEngineNotInitialized)
	{
		iState = ESoundEnginePaused;
		iPlayer->Pause();
		TTimeIntervalMicroSeconds pos = 0;
		iPlayer->GetPosition(pos);
		// had a crash here, so we check for NULL first
		if (iPodcastModel.PlayingPodcast() != NULL) {
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
	if(iState > ESoundEngineNotInitialized)
		{
			iPlayer->SetVolume((aVolume*iPlayer->MaxVolume()) / 100);
		}
}
