#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H
#include <e32base.h>
#include "FeedInfo.h"
#include "ShowInfo.h"
class CFeedEngine;
class CSoundEngine;
//class TShowInfo;

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
	void PlayPausePodcastL(TShowInfo * aPodcast);
	TFeedInfo& ActiveFeedInfo();
	void SetActiveFeedInfo(const TFeedInfo& aFeedInfo);

	TShowInfoArray& ActiveShowList();
	void SetActiveShowList(TShowInfoArray& aShowArray);

	TBool SetZoomState(TInt aZoomState);
	TInt ZoomState();
protected:
	CPodcastModel();
	void ConstructL();
private:	
   TShowInfo* iPlayingPodcast;
   CFeedEngine* iFeedEngine;
   CSoundEngine* iSoundEngine;
   TShowInfoArray iActiveShowList;
   TFeedInfo iActiveFeed;
   TInt iZoomState;

};

#endif // PODCASTMODEL_H

