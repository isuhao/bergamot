#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include "PodcastClientView.h"

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
	void ShowListUpdated();

};


#endif // Podcasts view


