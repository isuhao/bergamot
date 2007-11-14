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
#include "HttpClient.h"
#include "FeedEngine.h"

#include <coecobs.h>
#include <mqiklistboxobserver.h>

enum TMenus {
	EMenuMain,
	EMenuFiles,
	EMenuFeeds,
	EMenuDownloads,
	EMenuEpisodes,
};
class CPodcastModel;

class CPodcastClientView : public CQikViewBase, public MQikListBoxObserver
	{
public:
	static CPodcastClientView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
  
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void PlayPausePodcast(TShowInfo *podcast);
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	
private:
	CPodcastClientView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void ConstructL();
	void CreateMenu();
private:
	//RPodcastServerSession serverSession;
	CQikScrollableContainer* iContainer;
    TMenus iMenuState;
    TBool iDownloading;
	CPodcastModel& iPodcastModel;
	CQikListBox* iListbox;
	};
#endif
