#include <PodcastClient.rsg>

#include "PodcastClientPodcastsView.h"
#include "PodcastModel.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientPodcastsView* CPodcastClientPodcastsView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientPodcastsView* self = new (ELeave) CPodcastClientPodcastsView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastClientPodcastsView::CPodcastClientPodcastsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
}

CPodcastClientPodcastsView::~CPodcastClientPodcastsView()
{
}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientPodcastsView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastPodcastsViewID);
	}

void CPodcastClientPodcastsView::ViewConstructL()
{
	CPodcastClientView::ViewConstructL();
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_PODCASTS_TITLE);
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *iViewLabel);
}




void CPodcastClientPodcastsView::UpdateListboxItemsL()
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
	if (iPodcastModel.iActiveShowList == NULL) {
		iPodcastModel.iActiveShowList = iPodcastModel.FeedEngine().GetAllShows();
	}
	TShowInfoArray &fItems = *iPodcastModel.iActiveShowList;
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
	
	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();
}


void CPodcastClientPodcastsView::HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			TShowInfoArray &fItems = *iPodcastModel.iActiveShowList;

			if(!fItems[aItemIndex]->iShowDownloaded)
			{
				iPodcastModel.FeedEngine().AddDownload(fItems[aItemIndex]);
			}
			// play the podcast if downloaded and its not currently downloading
			else if(fItems[aItemIndex]->state !=  EDownloading)
			{
				iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);
			}
		}
		break;
	default:
		break;
	}
}
