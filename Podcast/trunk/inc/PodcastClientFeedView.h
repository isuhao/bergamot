#ifndef PODCASTCLIENTFEEDVIEW_H
#define PODCASTCLIENTFEEDVIEW_H
#include "PodcastClientView.h"
class CPodcastClientFeedView : public CPodcastClientView, public MFeedEngineObserver
{
public:
	static CPodcastClientFeedView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientFeedView();
protected:
	CPodcastClientFeedView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();
	void ConstructL();

	void ShowListUpdated(){}
    void FeedInfoUpdated(const TFeedInfo& aFeedInfo);
	void FeedDownloadUpdatedL(TInt aPercentOfCurrentDownload);
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal) {};
	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void HandleCommandL(CQikCommand& aCommand);

private:
    TBool iProgressAdded;
};


#endif
