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
