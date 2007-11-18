#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include "PodcastClientView.h"
class CQikCategoryModel;


class CPodcastClientShowsView : public CPodcastClientView, public MFeedEngineObserver
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
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload);

	void HandleCommandL(CQikCommand& aCommand);
	void UpdateShowItemL(TShowInfo* aShowInfo);
	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);

private:
	TPodcastClientShowCategory iCurrentCategory;
	TBool iProgressAdded;
};


#endif // Shows view


