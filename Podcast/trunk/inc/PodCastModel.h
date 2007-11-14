#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H
#include <e32base.h>

class CFeedEngine;
class CSoundEngine;
class TShowInfo;

/**
 * This class handles application storage needs and ownership of audioplayer, resource lists etc.
 */
class CPodcastModel:public CBase
{
public:
	static CPodcastModel* NewL();
	~CPodcastModel();
	CFeedEngine& FeedEngine();
	CSoundEngine& SoundEngine();
	TShowInfo* PlayingPodcast();
	void SetPlayingPodcast(TShowInfo* aPodcast);
protected:
	CPodcastModel();
	void ConstructL();
private:	
   TShowInfo* iPlayingPodcast;
   CFeedEngine* iFeedEngine;
   CSoundEngine* iSoundEngine;

};

#endif // PODCASTMODEL_H