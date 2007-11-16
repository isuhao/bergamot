#ifndef PODCASTCLIENTFEEDVIEW_H
#define PODCASTCLIENTFEEDVIEW_H
#include "PodcastClientView.h"
class CPodcastClientFeedView : public CPodcastClientView
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

private:
    TBool iDownloading;
};


#endif
