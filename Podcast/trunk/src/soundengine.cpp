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
		iPodcastModel.PlayingPodcast()->playing = EFalse;
	}
}

void CSoundEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	if (aError != KErrNone) {
		RDebug::Print(_L("MapcInitComplete error=%d"), aError);
		if (iPodcastModel.PlayingPodcast() != NULL) {			
			iPodcastModel.PlayingPodcast()->playing = EFalse;
		}
		iPlayerInit = EFalse;
		return;
	}
	iPlayerInit = ETrue;
	if(iVolume == 0)
	{
		iVolume = iPlayer->MaxVolume();
	}

	iPlayer->SetVolume(iVolume);

	if (iPodcastModel.PlayingPodcast() != NULL) {
		RDebug::Print(_L("Resuming from position: %ld"), iPodcastModel.PlayingPodcast()->position.Int64());
		iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->position);
		
		iPodcastModel.PlayingPodcast()->playing = ETrue;
	}

	iPlayer->Play();
}

void CSoundEngine::OpenFileL(TDesC& aFileName)
{
	iPlayer->OpenFileL(aFileName);
}

TTimeIntervalMicroSeconds CSoundEngine::Position()
{
	TTimeIntervalMicroSeconds pos = 0;
	
	if(iPlayerInit)
	{
		iPlayer->GetPosition(pos);
	}

	return pos;
}

void CSoundEngine::SetPosition(TUint aPos)
{
	if(iPlayerInit)
	{
		TTimeIntervalMicroSeconds pos = aPos*1000000;
		iPlayer->SetPosition(pos);
	}
}


TUint CSoundEngine::PlayTime()
{
	if(iPlayerInit)
	{
		return iPlayer->Duration().Int64()/1000000;
	}

	return 0;
}


void CSoundEngine::Play()
{
	if(iPlayerInit)
	{
		iPlayer->Play();
	}
}

void CSoundEngine::Stop()
{
	if(iPlayerInit)
	{
		iPlayer->Stop();
	}
}

void CSoundEngine::Pause()
{
	if(iPlayerInit)
	{
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

	if(iPlayerInit)
	{
		iPlayer->SetVolume(aVolume);
	}
}

