#include <badesca.h>
#include <eikchlst.h>
#include <qikscrollablecontainer.h>
#include <qikrowlayoutmanager.h>
#include <qikgridlayoutmanager.h>
#include <qikbuildingblock.h>
#include <qikcommand.h>
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include <PodcastClient.rsg>
#include <qikgridlayoutmanager.h>
#include "e32debug.h"
#include <eiklabel.h>
#include <eikcmbut.h>
#include "HttpClient.h"
#include "bautils.h"
#include <QikListBoxModel.h>
#include <QikListBox.h>
#include <QikListBoxData.h>
#include <eikdialg.h>


/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientView* CPodcastClientView::NewLC(CQikAppUi& aAppUi)
	{
	CPodcastClientView* self = new (ELeave) CPodcastClientView(aAppUi);
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
CPodcastClientView::CPodcastClientView(CQikAppUi& aAppUi) 
	: CQikViewBase(aAppUi, KNullViewId)
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
	
   	//httpCli->StartClientL();
	//CActiveScheduler::Start();

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
    iPlayer = CMdaAudioPlayerUtility::NewL(*this);
    ViewConstructFromResourceL(R_LISTBOX_LISTVIEW_UI_CONFIGURATIONS);
    iMenuState = EMenuMain;
    iPlayingPodcast = -1;
    iDownloading = EFalse;
    iFeedEngine.LoadSettings();
    iFeedEngine.LoadFeeds();

    CreateMenu();
    }

void CPodcastClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
		iMenuState = EMenuMain;
		CreateMenu();
	}

void CPodcastClientView::PlayPausePodcast(TPodcastId *podcast) 
	{
	if (iPlayingPodcast == podcast->iId) {
		if (podcast->iPlaying) {
			User::InfoPrint(_L("Pausing"));
			iPlayer->Pause();
			iPlayer->GetPosition(podcast->iPosition);
			podcast->iPlaying = EFalse;
		} else {
			User::InfoPrint(_L("Resuming"));
			iPlayer->Play();
			podcast->iPlaying = ETrue;
		}
	} else {
		iPlayer->Stop();
		User::InfoPrint(_L("Playing"));
		RDebug::Print(_L("Starting: %S"), &(podcast->iFileName));
		TRAPD(error, iPlayer->OpenFileL(podcast->iFileName));
	    if (error != KErrNone) {
	    	RDebug::Print(_L("Error: %d"), error);
	    }
	    podcast->iPlaying = ETrue;
		iPlayingPodcast = podcast->iId;

	}
}

void CPodcastClientView::HandleControlEventL(CCoeControl */*aControl*/, TCoeEvent aEventType)
	{
	RDebug::Print(_L("ControlEvent"));
	switch(aEventType)
    {
    case EEventStateChanged:
    	/*if (aControl->UniqueHandle() == EMyWebCommand) {
    		TFeedInfo feedInfo;
    		feedInfo.iUrl.Copy(_L("http://www.podshow.com/feeds/tech5.xml"));
    		iFeedEngine.GetFeed(feedInfo);
    	} else {
    	int id = aControl->UniqueHandle() - EMyViewCommandButton;
    	
    	if (id < podcasts.Count()) {
    		TPodcastId *podcast = podcasts[id];
    		RDebug::Print(_L("EEventStateChanged"));
    		RDebug::Print(_L("HandleControlEventL: %S, %S"), &(podcast->iTitle), &(podcast->iFileName));
    		PlayPausePodcast(podcast);
    	}
    	}*/
        // The internal state of the Command Button was changed.
        break;
                                
    case EEventRequestExit:
    	//RDebug::Print(_L("EEventRequestExit"));
		break;
                                
    case EEventRequestCancel:
    	//RDebug::Print(_L("EEventRequestCancel"));
        break;
                                
    case EEventRequestFocus:
    	//RDebug::Print(_L("EEventRequestFocus"));
        // The Command Button received a pointer down event
        break;                      
    case EEventPrepareFocusTransition:
    	//RDebug::Print(_L("EEventPrepareFocusTransition"));
        // A focus change is about to appear
        break;
                            
    case EEventInteractionRefused:
    	//RDebug::Print(_L("EEventInteractionRefused"));
        // The Command Button is dimmed and received a pointer down event.
        break;                
    default:
        break;
    }

	}


void CPodcastClientView::MapcPlayComplete(TInt aError) {
	RDebug::Print(_L("Play Complete: %d"), aError);
	User::InfoPrint(_L("Done!"));
}

void CPodcastClientView::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	RDebug::Print(_L("Init Complete: %d"), aError);

	int numEntries = 0;
	int error = iPlayer->GetNumberOfMetaDataEntries(numEntries);
	RDebug::Print(_L("Found %d meta data entries"), numEntries);
	
	for (int i=0;i<numEntries;i++) {
		CMMFMetaDataEntry* entry = iPlayer->GetMetaDataEntryL(i);
		RDebug::Print(_L("Entry: Name: %S, Value: %S"), &entry->Name(), &entry->Value());
	}
	
	if (aError == KErrNone) {
	  iPlayer->Play();
	}
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
			TPodcastIdArray &files = iFeedEngine.GetPodcasts();
			RDebug::Print(_L("File: %S"), &(files[aItemIndex]->iFileName));
			PlayPausePodcast(files[aItemIndex]);
			break;
		case EMenuFeeds:
/*			if (aItemIndex == 0) {

			} else {*/
				if (iDownloading) {
					User::InfoPrint(_L("Cancel not implemented"));
					iDownloading = EFalse;
				} else {
					RArray <TFeedInfo*>& feeds = iFeedEngine.GetFeeds();
					RDebug::Print(_L("URL: %S"), &(feeds[aItemIndex]->iUrl));
					User::InfoPrint(_L("Getting feed..."));
					iDownloading = ETrue;
					iFeedEngine.GetFeed(*feeds[aItemIndex]);
					iMenuState = EMenuEpisodes;
					CreateMenu();
					iDownloading = EFalse;
				}
//			}
			break;
		case EMenuEpisodes:
			RArray <TPodcastInfo*>& fItems = iFeedEngine.GetItems();
			RDebug::Print(_L("Get podcast URL: %S"), &(fItems[aItemIndex]->iUrl));
			iFeedEngine.GetPodcast(*fItems[aItemIndex]);
			User::InfoPrint(_L("Done!"));
		}
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
	
		/*listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
		CleanupClosePushL(*listBoxData);
		itemName.Copy(_L("Downloads"));
		listBoxData->AddTextL(itemName, EQikListBoxSlotText1);*/
	
		CleanupStack::PopAndDestroy(2);
		}
		break;
	case EMenuFeeds:
		{
		RArray <TFeedInfo*>& feeds = iFeedEngine.GetFeeds();
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
			listBoxData->AddTextL(fi->iTitle, EQikListBoxSlotText1);
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
		iFeedEngine.ListAllPodcasts();
		TPodcastIdArray & files = iFeedEngine.GetPodcasts();
		
		for (int i=0;i<files.Count();i++) {
			MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TPodcastId *item = files[i];
			listBoxData->AddTextL(item->iTitle, EQikListBoxSlotText1);
			CleanupStack::PopAndDestroy();	
		}
		break;
		}
	break;
	case EMenuEpisodes:
		{
		RArray <TPodcastInfo*>& fItems = iFeedEngine.GetItems();
		int len = fItems.Count();
		
		if (len > 0) {
			for (int i=0;i<len;i++) {
			MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
			CleanupClosePushL(*listBoxData);
			TPodcastInfo *fi = fItems[i];
			listBoxData->AddTextL(fi->iTitle, EQikListBoxSlotText1);
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
