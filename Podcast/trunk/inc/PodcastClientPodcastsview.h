#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include "PodCastClientView.h"

class CPodcastClientPodcastsView : public CPodcastClientView
{
public:
	static CPodcastClientPodcastsView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientPodcastsView();
protected:
	CPodcastClientPodcastsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();

};


#endif // Podcasts view


