#include <badesca.h>
#include <qikscrollablecontainer.h>
#include <qikrowlayoutmanager.h>
#include <qikgridlayoutmanager.h>
#include <qikbuildingblock.h>
#include <qikcommand.h>
#include <PodcastClient.rsg>
#include <qikgridlayoutmanager.h>

#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"

/**
Constructor for the view.
Passes the application UI reference to the construction of the super class.

KNullViewId should normally be passed as parent view for the applications 
default view. The parent view is the logical view that is normally activated 
when a go back command is issued. KNullViewId will activate the system 
default view. 

@param aAppUi Reference to the application UI
*/
CPodcastClientView::CPodcastClientView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel) 
	: CQikViewBase(aAppUi, KNullViewId), iPodcastModel(aPodcastModel)
	{
	}

/**
Destructor for the view
*/
CPodcastClientView::~CPodcastClientView()
	{
	delete iViewLabel;
	}

/**
2nd stage construction of the view.
*/
void CPodcastClientView::ConstructL()
	{
	//RDebug::Print(_L("ConstructL"));
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CPodcastClientView::HandleCommandL(CQikCommand& aCommand)
{
	switch(aCommand.Id())
	{
	case EPodcastViewMain:
		{			
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientBaseViewID);
			iQikAppUi.ActivateViewL(playView);
		}break;
	case EPodcastViewPlayer:
		{
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientPlayViewID);
			iQikAppUi.ActivateViewL(playView);
		}
		break;		
	case EPodcastViewPodcasts:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientPodcastsViewID);
			iQikAppUi.ActivateViewL(podcastsView);
		}break;
	case EPodcastViewFeeds:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientFeedViewID);
			iQikAppUi.ActivateViewL(podcastsView);
		}break;		
		// Just issue simple info messages to show that
		// the commands have been selected
	default:
		CQikViewBase::HandleCommandL(aCommand);
		break;
	}
}


void CPodcastClientView::ViewConstructL()
{
    //RDebug::Print(_L("ViewConstructL"));
    ViewConstructFromResourceL(R_PODCAST_LISTVIEW_UI_CONFIGURATIONS);

	ViewContext()->AddTextL(EPodcastListViewContextLabel, KNullDesC(), EHCenterVCenter);

	// Get the list box and the list box model
	iListbox = LocateControlByUniqueHandle<CQikListBox>(EPodcastListViewListCtrl);
	iListbox->SetListBoxObserver(this);
    UpdateListboxItemsL();
    }

void CPodcastClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
		UpdateListboxItemsL();
	}


void CPodcastClientView::HandleControlEventL(CCoeControl */*aControl*/, TCoeEvent aEventType)
	{
	RDebug::Print(_L("HandleControlEvent"));
	}




/*
void CPodcastClientView::CreateMenu()
{
	iListbox->RemoveAllItemsL();

	MQikListBoxModel& model(iListbox->Model());
	// Informs the list box model that there will be an update of the data.
	// Notify the list box model that changes will be made after this point.
	// Observe that a cleanupitem has been put on the cleanupstack and 
	// will be removed by ModelEndUpdateL. This means that you have to 
	// balance the cleanupstack.
	// When you act directly on the model you always need to encapsulate 
	// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
	model.ModelBeginUpdateLC();
	
	TBuf<100> itemName;
	RDebug::Print(_L("CreateMenu, iMenuState: %d"), (int)iMenuState);
	switch (iMenuState) {



	case EMenuFiles:
		{	RFs rfs;
		rfs.Connect();
		iPodcastModel.FeedEngine().ListAllPodcasts();
		TShowInfoArray& files = iPodcastModel.FeedEngine().GetPodcasts();
		RDebug::Print(_L("Showing menu files..."));
		for (int i=0;i<files.Count();i++) {
			MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TShowInfo *item = files[i];
			listBoxData->AddTextL(item->title, EQikListBoxSlotText1);
			if (item->playing) {
				listBoxData->AddTextL(_L("Playing"), EQikListBoxSlotText2);
			} else if (item->position.Int64() != 0) {
				TTime time(item->position.Int64());
				TTime ref(0);
				TTimeIntervalSeconds seconds;
				TTimeIntervalHours hours;
				TTimeIntervalMinutes minutes;
				
				time.HoursFrom(ref, hours);
				time.MinutesFrom(ref, minutes);
				time.SecondsFrom(ref,seconds);
				TBuf<100> buf;
				buf.Format(_L("Paused at %d h %d m %d s"), hours.Int(), minutes.Int(), seconds.Int());
				listBoxData->AddTextL(buf, EQikListBoxSlotText2);
			}
			
			CleanupStack::PopAndDestroy();	
		}
		break;
		}
	break;

	}

	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();
}
*/