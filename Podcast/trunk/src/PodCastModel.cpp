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
	if (iPlayingPodcast == aPodcast) {
		if (aPodcast->state == EPlaying) {
			User::InfoPrint(_L("Pausing"));
			SoundEngine().Pause();
			aPodcast->position = iSoundEngine->Position();
			aPodcast->state = EStateless;
		} else {
			User::InfoPrint(_L("Resuming"));
			iSoundEngine->Play();
		}
	} else {
		// switching file, so save position
		iSoundEngine->Pause();
		if (iPlayingPodcast != NULL) {
			iPlayingPodcast->position = iSoundEngine->Position();
			iPlayingPodcast->state  = EStateless;
		}
		
		iSoundEngine->Stop();

		User::InfoPrint(_L("Playing"));
		RDebug::Print(_L("Starting: %S"), &(aPodcast->fileName));
		TRAPD(error, iSoundEngine->OpenFileL(aPodcast->fileName));
	    if (error != KErrNone) {
	    	RDebug::Print(_L("Error: %d"), error);
	    }

		iPlayingPodcast = aPodcast;
	}
}

