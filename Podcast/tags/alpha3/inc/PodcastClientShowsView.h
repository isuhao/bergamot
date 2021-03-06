#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include "PodcastClientView.h"
#include "ShowEngineObserver.h"

class CQikCategoryModel;


class CPodcastClientShowsView : public CPodcastClientView, public MFeedEngineObserver, public MShowEngineObserver
{
public:
	static CPodcastClientShowsView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientShowsView();
protected:
	CPodcastClientShowsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();
	void ConstructL();
	void ShowListUpdated();
    void FeedInfoUpdated(const TFeedInfo& aFeedInfo);
	void FeedDownloadUpdatedL(TInt aPercentOfCurrentDownload){};
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
	void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows) {};
	void HandleCommandL(CQikCommand& aCommand);
	void UpdateShowItemL(TShowInfo* aShowInfo, TInt aSizeDownloaded = KErrNotFound);
	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	void UpdateCommandsL();
	void UpdateSelectCommandL();
	void GetShowIcons(TShowInfo* aShowInfo, TInt& aImageId, TInt& aMaskId);
private:
	TPodcastClientShowCategory iCurrentCategory;
	TBool iProgressAdded;
};


#endif // Shows view


