#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <qikviewbase.h>
#include "PodcastClient.hrh"
#include "s32file.h"
#include <e32cmn.h>
#include <coecobs.h>
#include <mqiklistboxobserver.h>
#include <e32debug.h>
#include <qiklistboxmodel.h>
#include <qiklistbox.h>
#include <qiklistboxdata.h>
#include <qikappui.h>

#include "e32debug.h"
#include "PodcastClientGlobals.h"
#include "HttpClient.h"
#include "FeedEngine.h"

class CPodcastModel;

class CPodcastClientView : public CQikViewBase, public MQikListBoxObserver
	{
public:
	~CPodcastClientView();
	void HandleCommandL(CQikCommand& aCommand);
  
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void PlayPausePodcast(TShowInfo *podcast);
protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	
protected:
	CPodcastClientView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	virtual void ConstructL();
	virtual void UpdateListboxItemsL() = 0;
protected:
	CPodcastModel& iPodcastModel;
	CQikListBox* iListbox;
	HBufC* iViewLabel;
	};
#endif
