#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <qikviewbase.h>
#include <qikscrollablecontainer.h>
#include "PodcastClient.hrh"
#include "s32file.h"
#include "e32debug.h"
#include <e32cmn.h>
//#include "RPodcastServerSession.h"
#include "PodcastClientGlobals.h"
#include <mdaaudiosampleplayer.h>
#include <coecobs.h>
#include "HttpClient.h"
#include "FeedEngine.h"
#include <MQikListBoxObserver.h>

enum TMenus {
	EMenuMain,
	EMenuFiles,
	EMenuFeeds,
	EMenuDownloads,
	EMenuEpisodes,
};

class CPodcastClientView : public CQikViewBase, public MMdaAudioPlayerCallback, public MQikListBoxObserver
	{
public:
	static CPodcastClientView* NewLC(CQikAppUi& aAppUi);
	~CPodcastClientView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
    void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void PlayPausePodcast(TShowInfo *podcast);
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	
private:
	CPodcastClientView(CQikAppUi& aAppUi);
	void ConstructL();
	void CreateMenu();
	
private:
	//RPodcastServerSession serverSession;
	CQikScrollableContainer* iContainer;
    CMdaAudioPlayerUtility *iPlayer;
    TShowInfo* iPlayingPodcast;
    TMenus iMenuState;
    CFeedEngine iFeedEngine;
    TBool iDownloading;
	};
#endif
