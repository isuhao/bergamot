#include <e32std.h>

#include "SoundEngine.h"
#include "PodcastModel.h"
#include "PodcastData.h"
#include "ShowInfo.h"

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


void CSoundEngine::MapcPlayComplete(TInt aError) {
//	RDebug::Print(_L("Play Complete: %d"), aError);
//	User::InfoPrint(_L("Done!"));
	if (iPodcastModel.PlayingPodcast() != NULL) {
		iPodcastModel.PlayingPodcast()->playState = EPlayed;
	}

	iState = ESoundEngineStopped;
}

void CSoundEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	if (aError != KErrNone) {
		RDebug::Print(_L("MapcInitComplete error=%d"), aError);
		/*if (iPodcastModel.PlayingPodcast() != NULL) {			
			iPodcastModel.PlayingPodcast()->playState = EStateless;
		}*/	

		iState = ESoundEngineNotInitialized;

		return;
	}
	iState = ESoundEngineStopped;

	if(iVolume == 0)
	{
		iVolume = iPlayer->MaxVolume();
	}

	iPlayer->SetVolume(iVolume);

	if (iPodcastModel.PlayingPodcast() != NULL) {
		RDebug::Print(_L("Resuming from position: %ld"), iPodcastModel.PlayingPodcast()->position.Int64());
		iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->position);
		
		iPodcastModel.PlayingPodcast()->playState = EPlaying;
	}

	iState = ESoundEnginePlaying;

	iPlayer->Play();
}

void CSoundEngine::OpenFileL(TDesC& aFileName)
{
	iState = ESoundEngineNotInitialized;

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
		iPlayer->SetPosition(pos);
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
	}
}

TUint CSoundEngine::Volume()
{
	return iVolume;
}

void CSoundEngine::SetVolume(TUint aVolume)
{
	iVolume = aVolume;

	if(iState > ESoundEngineNotInitialized)
	{
		iPlayer->SetVolume(aVolume);
	}
}

TSoundEngineState CSoundEngine::State()
{
	return iState;
}


