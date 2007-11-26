#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H
#include <e32base.h>
#include <eikenv.h>
#include "FeedInfo.h"
#include "ShowInfo.h"

class CFeedEngine;
class CSoundEngine;
class CShowEngine;
class CSettingsEngine;

/**
 * This class handles application storage needs and ownership of audioplayer, resource lists etc.
 */
class CPodcastModel:public CBase
{
public:
	static CPodcastModel* NewL();
	~CPodcastModel();
	CFeedEngine& FeedEngine();
	CShowEngine& ShowEngine();
	CSoundEngine& SoundEngine();
	CSettingsEngine& SettingsEngine();
	CShowInfo* PlayingPodcast();
	void SetPlayingPodcast(CShowInfo* aPodcast);
	void PlayPausePodcastL(CShowInfo * aPodcast);
	TFeedInfo& ActiveFeedInfo();
	void SetActiveFeedInfo(const TFeedInfo& aFeedInfo);
	CEikonEnv* EikonEnv();
	CShowInfoArray& ActiveShowList();
	void SetActiveShowList(CShowInfoArray& aShowArray);

	TBool SetZoomState(TInt aZoomState);
	TInt ZoomState();
protected:
	CPodcastModel();
	void ConstructL();
private:	
   CShowInfo* iPlayingPodcast;
   
   CFeedEngine* iFeedEngine;
   CShowEngine* iShowEngine;
   CSoundEngine* iSoundEngine;
   CSettingsEngine *iSettingsEngine;
   
   CShowInfoArray iActiveShowList;
   TFeedInfo iActiveFeed;
   TInt iZoomState;
   
   CEikonEnv* iEnv;
};

#endif // PODCASTMODEL_H

