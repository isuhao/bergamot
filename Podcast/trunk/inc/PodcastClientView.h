#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <qikviewbase.h>
#include <qikscrollablecontainer.h>
#include "PodcastClient.hrh"
#include "s32file.h"
#include "e32debug.h"
#include <e32cmn.h>
#include "RPodcastServerSession.h"
#include "PodcastClientGlobals.h"
#include <mdaaudiosampleplayer.h>
#include <coecobs.h>
#include "HttpClient.h"
#include "FeedParserCallbacks.h"

class TPodcastId {
public:
	TPodcastId(int aId, const TDesC &aFileName, const TDesC &aTitle) {
		iId =aId;
		iFileName.Copy(aFileName);
		iTitle.Copy(aTitle);
		iPlaying = EFalse;
	}
	
	int iId;
	TBuf<256> iFileName;
	TBuf<256> iTitle;
	TTimeIntervalMicroSeconds iPosition;
	TBool iPlaying;
};

class CPodcastClientView : public CQikViewBase, public MRssParserCallbacks, public MResultObs, public MMdaAudioPlayerCallback
	{
public:
	static CPodcastClientView* NewLC(CQikAppUi& aAppUi);
	~CPodcastClientView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
    void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void PlayPausePodcast(TPodcastId *podcast);
protected: 
	void ViewConstructL();
	
private:
	CPodcastClientView(CQikAppUi& aAppUi);
	void ConstructL();
	void ListAllPodcastsL();
	void ListDirL(RFs &rfs, TDesC &folder);
	void CreatePodcastListItem(TPodcastId *pid);
	
	virtual void AddResult(const TDesC& aText);
	virtual void SetConnected();
    virtual void SetDisconnected();
    
	void Item(TPodcastItem *item);

private:
	RArray<TPodcastId*> podcasts;
	RPodcastServerSession serverSession;
	CQikScrollableContainer* iContainer;
    CMdaAudioPlayerUtility *iPlayer;
    int iPlayingPodcast;
    CHttpClient* iClient;
	};
#endif
