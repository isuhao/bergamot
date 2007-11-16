#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include "PodcastClientView.h"

class CPodcastClientShowsView : public CPodcastClientView
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

};


#endif // Podcasts view


