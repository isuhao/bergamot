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
		return;
	}

	
	if (iPodcastModel.PlayingPodcast() != NULL) {
		RDebug::Print(_L("Resuming from position: %ld"), iPodcastModel.PlayingPodcast()->position.Int64());
		iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->position);
		
		iPodcastModel.PlayingPodcast()->playing = ETrue;
	}

	iPlayer->Play();
}

void CSoundEngine::OpenFileL(TDesC& aFileName)
{
}

TTimeIntervalMicroSeconds CSoundEngine::Position()
{
	TTimeIntervalMicroSeconds pos;
    iPlayer->GetPosition(pos);
	return pos;
}


void CSoundEngine::Play()
{
	iPlayer->Play();
}

void CSoundEngine::Stop()
{
	iPlayer->Stop();
}

void CSoundEngine::Pause()
{
	iPlayer->Pause();
}
