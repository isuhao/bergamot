#ifndef PODCASTCLIENTBASEVIEW_H
#define PODCASTCLIENTBASEVIEW_H
#include "PodcastClientView.h"

enum TBaseViewSelections
{
	EBaseViewPlayer = 0x100, 
	EBaseViewNewShows = 0x101,
	EBaseViewDownloadedShows = 0x102,
	EBaseViewPendingShows = 0x103,
	EBaseViewFeeds = 0x104
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
	void ViewConstructL();

	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);

private:
	TBool iCheckForQuedDownloads;

};
#endif // baseview


