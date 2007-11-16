#include <PodcastClient.rsg>

#include "PodcastClientShowsView.h"
#include "PodcastModel.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientShowsView* CPodcastClientShowsView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientShowsView* self = new (ELeave) CPodcastClientShowsView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	aPodcastModel.FeedEngine().AddObserver(self);
	return self;
	}

CPodcastClientShowsView::CPodcastClientShowsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
}

CPodcastClientShowsView::~CPodcastClientShowsView()
{
}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientShowsView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastPodcastsViewID);
	}

void CPodcastClientShowsView::ViewConstructL()
{
	CPodcastClientView::ViewConstructL();
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_PODCASTS_TITLE);
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *iViewLabel);
}


// Engine callback when new shows are available
void CPodcastClientShowsView::ShowListUpdated()
	{
	UpdateListboxItemsL();
	}

void CPodcastClientShowsView::UpdateListboxItemsL()
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


void CPodcastClientShowsView::HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
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
		RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->title), fItems[aItemIndex]->downloadState);
		if(fItems[aItemIndex]->downloadState == ENotDownloaded)
		{
			iPodcastModel.FeedEngine().AddDownload(fItems[aItemIndex]);
		}
		// play the podcast if downloaded
		else if(fItems[aItemIndex]->downloadState == EDownloaded)
		{
			iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);
		}
	}
	break;
default:
	break;
}
}