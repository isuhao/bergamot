#ifndef PODCASTCLIENTBASEVIEW_H
#define PODCASTCLIENTBASEVIEW_H
#include "PodcastClientView.h"

enum TBaseViewSelections
{
	EBaseViewPlayer, 
	EBaseViewPodcasts,
	EBaseViewFeeds
};

class CPodcastClientBaseView : public CPodcastClientView
{
public:
	static CPodcastClientBaseView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientBaseView();
protected:
	CPodcastClientBaseView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
};
#endif // baseview


