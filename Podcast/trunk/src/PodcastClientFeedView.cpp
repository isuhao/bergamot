#include <PodcastClient.rsg>
#include "PodcastClientFeedView.h"
#include "PodcastModel.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientFeedView* CPodcastClientFeedView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientFeedView* self = new (ELeave) CPodcastClientFeedView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastClientFeedView::CPodcastClientFeedView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
}

CPodcastClientFeedView::~CPodcastClientFeedView()
{
}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientFeedView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID);
	}

void CPodcastClientFeedView::ViewConstructL()
{
	CPodcastClientView::ViewConstructL();
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_TITLE);
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *iViewLabel);
}


void CPodcastClientFeedView::UpdateListboxItemsL()
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
	
	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();
}


void CPodcastClientFeedView::HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			if (iDownloading) {
				User::InfoPrint(_L("Cancel not implemented"));
				iDownloading = EFalse;
			} else {
				TFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetFeeds();
				RDebug::Print(_L("URL: %S"), &(feeds[aItemIndex]->url));
				User::InfoPrint(_L("Getting feed..."));
				iDownloading = ETrue;
				iPodcastModel.FeedEngine().GetFeed(feeds[aItemIndex]);
				iPodcastModel.iActiveShowList = iPodcastModel.FeedEngine().GetShowsByFeed(feeds[aItemIndex]->title);
				iPodcastModel.iActiveFeed = feeds[aItemIndex];
				TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastPodcastsViewID);
				iQikAppUi.ActivateViewL(podcastsView);
			}
		}
		break;
	default:
		break;
	}
}
