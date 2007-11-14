#include <badesca.h>
#include <eikchlst.h>
#include <qikscrollablecontainer.h>
#include <qikrowlayoutmanager.h>
#include <qikgridlayoutmanager.h>
#include <qikbuildingblock.h>
#include <qikcommand.h>
#include <PodcastClient.rsg>
#include <qikgridlayoutmanager.h>
#include <e32debug.h>
#include <eiklabel.h>
#include <eikcmbut.h>
#include <bautils.h>
#include <qiklistboxmodel.h>
#include <qiklistbox.h>
#include <qiklistboxdata.h>
#include <eikdialg.h>

#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientView* CPodcastClientView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientView* self = new (ELeave) CPodcastClientView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

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
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidListBoxListView);
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
    ViewConstructFromResourceL(R_LISTBOX_LISTVIEW_UI_CONFIGURATIONS);
    iMenuState = EMenuMain;
    iDownloading = EFalse;

    CreateMenu();
    }

void CPodcastClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
		iMenuState = EMenuMain;
		CreateMenu();
	}

void CPodcastClientView::PlayPausePodcast(TShowInfo* aPodcast) 
	{
	
	// special treatment if this podcast is already active
	if (iPodcastModel.PlayingPodcast() == aPodcast) {
		if (aPodcast->playing) {
			User::InfoPrint(_L("Pausing"));
			iPodcastModel.SoundEngine().Pause();
			aPodcast->position = iPodcastModel.SoundEngine().Position();
			aPodcast->playing = EFalse;
		} else {
			User::InfoPrint(_L("Resuming"));
			iPodcastModel.SoundEngine().Play();
			aPodcast->playing = ETrue;
		}
	} else {
		// switching file, so save position
		iPodcastModel.SoundEngine().Pause();
		if (iPodcastModel.PlayingPodcast() != NULL) {
			iPodcastModel.PlayingPodcast()->position = iPodcastModel.SoundEngine().Position();
			iPodcastModel.PlayingPodcast()->playing = EFalse;
		}
		
		iPodcastModel.SoundEngine().Stop();

		User::InfoPrint(_L("Playing"));
		RDebug::Print(_L("Starting: %S"), &(aPodcast->fileName));
		TRAPD(error, iPodcastModel.SoundEngine().OpenFileL(aPodcast->fileName));
	    if (error != KErrNone) {
	    	RDebug::Print(_L("Error: %d"), error);
	    }
		iPodcastModel.SetPlayingPodcast(aPodcast);
	}
}

void CPodcastClientView::HandleControlEventL(CCoeControl */*aControl*/, TCoeEvent aEventType)
	{
	RDebug::Print(_L("HandleControlEvent"));
	}



void CPodcastClientView::HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
	{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		switch (iMenuState)
		{
		case EMenuMain:
			iMenuState = (TMenus) (aItemIndex + 1);
			CreateMenu();
			break;
		case EMenuFiles:
			{
			TShowInfoArray &files = iPodcastModel.FeedEngine().GetPodcasts();
			RDebug::Print(_L("File: %S"), &(files[aItemIndex]->fileName));
			PlayPausePodcast(files[aItemIndex]);
			}
			break;
		case EMenuFeeds:
			if (iDownloading) {
				User::InfoPrint(_L("Cancel not implemented"));
				iDownloading = EFalse;
			} else {
				TFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetFeeds();
				RDebug::Print(_L("URL: %S"), &(feeds[aItemIndex]->url));
				User::InfoPrint(_L("Getting feed..."));
				iDownloading = ETrue;
				iPodcastModel.FeedEngine().GetFeed(feeds[aItemIndex]);
				iMenuState = EMenuEpisodes;
				CreateMenu();
				iDownloading = EFalse;
			}
			break;
		case EMenuEpisodes:
			{
			TShowInfoArray& fItems = iPodcastModel.FeedEngine().GetItems();
			RDebug::Print(_L("Get podcast URL: %S"), &(fItems[aItemIndex]->url));
			iPodcastModel.FeedEngine().AddDownload(fItems[aItemIndex]);
			}
			break;
		case EMenuDownloads:
			break;
		}
		break;
	default:
		break;
	}
	}

void CPodcastClientView::CreateMenu()
{
	// Get the list box and the list box model
	CQikListBox* listBox = (CQikListBox*) LocateControlByUniqueHandle<const CQikListBox>(EListBoxListViewListCtrl);
	listBox->RemoveAllItemsL();
	MQikListBoxModel& model(listBox->Model());
	listBox->SetListBoxObserver(this);
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
	case EMenuMain:
		{
		MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
		CleanupClosePushL(*listBoxData);
		itemName.Copy(_L("Episodes"));
		listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
	
		listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
		CleanupClosePushL(*listBoxData);
		itemName.Copy(_L("Subscriptions"));
		listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
	
		listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
		CleanupClosePushL(*listBoxData);
		itemName.Copy(_L("Downloads"));
		listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
	
		CleanupStack::PopAndDestroy(3);
		}
		break;
	case EMenuDownloads:
		{
		TShowInfoArray& downloads = iPodcastModel.FeedEngine().GetDownloads();
		int len = downloads .Count();
		MQikListBoxData* listBoxData;
		
		if (len > 0) {
			for (int i=0;i<len;i++) {
			listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TShowInfo *show = downloads[i];
			listBoxData->AddTextL(show->title, EQikListBoxSlotText1);
			if (show->state == EDownloading) {
				listBoxData->AddTextL(_L("Downloading..."), EQikListBoxSlotText2);
			} else {
				listBoxData->AddTextL(_L("Queued"), EQikListBoxSlotText2);
			}
			CleanupStack::PopAndDestroy();	
			}
		}
		}
		break;
	case EMenuFeeds:
		{
		RArray <TFeedInfo*>& feeds = iPodcastModel.FeedEngine().GetFeeds();
		int len = feeds.Count();
		MQikListBoxData* listBoxData;// = model.NewDataL(MQikListBoxModel::EDataNormal);
		//CleanupClosePushL(*listBoxData);
	
		//itemName.Copy(_L("Add feed..."));
		//listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
		//CleanupStack::PopAndDestroy();
		
		if (len > 0) {
			for (int i=0;i<len;i++) {
			listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TFeedInfo *fi = feeds[i];
			listBoxData->AddTextL(fi->title, EQikListBoxSlotText1);
			listBoxData->AddTextL(fi->description, EQikListBoxSlotText2);
			CleanupStack::PopAndDestroy();	
			}
		} else {
			{
			listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
		
			itemName.Copy(_L("No feeds"));
			listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
			CleanupStack::PopAndDestroy();
			}
		}
		
		}

	break;
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
	case EMenuEpisodes:
		{
		TShowInfoArray& fItems = iPodcastModel.FeedEngine().GetItems();
		int len = fItems.Count();
		
		if (len > 0) {
			for (int i=0;i<len;i++) {
			MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TShowInfo *fi = fItems[i];
			listBoxData->AddTextL(fi->title, EQikListBoxSlotText1);
			listBoxData->AddTextL(fi->description, EQikListBoxSlotText2);
			CleanupStack::PopAndDestroy();	
			}
		} else {
			{
			MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
		
			itemName.Copy(_L("No items"));
			listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
			CleanupStack::PopAndDestroy();
			}
		}
		
		}

	break;
	}

		if (iMenuState == EMenuMain) {
		    SetParentView(KNullViewId);
		} else {
		    SetParentView(ViewId());
		}
	
	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();
}
