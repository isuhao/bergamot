#include <PodcastClient.rsg>
#include <qikcategorymodel.h>
#include <qikcommand.h>

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

void CPodcastClientShowsView::ConstructL()
{
	CPodcastClientView::ConstructL();
	iFeedsCategories = CQikCategoryModel::NewL();
	SetCategoryModel(iFeedsCategories);
	SetCategoryModelAsCommandsL();
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
	return TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
	}

void CPodcastClientShowsView::ViewConstructL()
{
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_SHOWS_TITLE);
	CPodcastClientView::ViewConstructL();
	
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *iViewLabel);
}

void CPodcastClientShowsView::HandleCommandL(CQikCommand& aCommand)
{
	if(aCommand.Type() == EQikCommandTypeCategory)
	{
		RArray <TFeedInfo*>& feeds = iPodcastModel.FeedEngine().GetFeeds();
		TInt len = feeds.Count();
		for (TInt i=0;i<len;i++) {
			if(feeds[i]->iCategoryHandle == CategoryHandleForCommandId(aCommand.Id()))
			{
				iPodcastModel.SetActiveFeedInfo(*feeds[i]);
				iPodcastModel.SetActiveShowList(*iPodcastModel.FeedEngine().GetShowsByFeed(feeds[i]));
				UpdateListboxItemsL();
				return;
			}
		}
	}

	switch(aCommand.Id())
	{
		default:
			CPodcastClientView::HandleCommandL(aCommand);
			break;
	}

}

// Engine callback when new shows are available
void CPodcastClientShowsView::ShowListUpdated()
	{
	UpdateListboxItemsL();
	}

void CPodcastClientShowsView::FeedInfoUpdated(const TFeedInfo& aFeedInfo)
{
	if(aFeedInfo.url == iPodcastModel.ActiveFeedInfo().url)
	{
		iPodcastModel.SetActiveFeedInfo(aFeedInfo);
	}
}


CQikCommand* CPodcastClientShowsView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands)
{
	if(aCommand->Id() == EQikSoftkeyCmdSelectCategory)
	{
		aCommand->SetTextL(R_PODCAST_FEEDS_CATEGORY);
	}

	return aCommand;
}



_LIT(KShowsTitleFormat, "%S - %S");
void CPodcastClientShowsView::UpdateListboxItemsL()
{
	iListbox->RemoveAllItemsL();
	

	HBufC* titleBuffer = HBufC::NewLC(iViewLabel->Length()+KShowsTitleFormat().Length()+iPodcastModel.ActiveFeedInfo().title.Length());
	titleBuffer->Des().Format(KShowsTitleFormat, iViewLabel, &iPodcastModel.ActiveFeedInfo().title);
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
	CleanupStack::PopAndDestroy(titleBuffer);

	MQikListBoxModel& model(iListbox->Model());
	
	// Informs the list box model that there will be an update of the data.
	// Notify the list box model that changes will be made after this point.
	// Observe that a cleanupitem has been put on the cleanupstack and 
	// will be removed by ModelEndUpdateL. This means that you have to 
	// balance the cleanupstack.
	// When you act directly on the model you always need to encapsulate 
	// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
	model.ModelBeginUpdateLC();
	
	if (iPodcastModel.ActiveShowList().Count() == 0) {
		iPodcastModel.SetActiveShowList(*iPodcastModel.FeedEngine().GetShowsByFeed(&iPodcastModel.ActiveFeedInfo()));
	}
	TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TInt len = fItems.Count();
	
	if (len > 0) {
		for (TInt i=0;i<len;i++) {
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
					
			listBoxData->AddTextL(_L("No items"), EQikListBoxSlotText1);
			CleanupStack::PopAndDestroy();
		}
	}
	
	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();

		
	RArray <TFeedInfo*>& feeds = iPodcastModel.FeedEngine().GetFeeds();
	len = feeds.Count();
	for (TInt i=0;i<len;i++) {
		if(feeds[i]->iCategoryHandle == KErrNotFound) 
		{
			feeds[i]->iCategoryHandle = iFeedsCategories->AddCategoryL(feeds[i]->title);
		}
		else
		{
			iFeedsCategories->RenameCategoryL(feeds[i]->iCategoryHandle, feeds[i]->title);
		}
	}
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
		TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->title), fItems[aItemIndex]->downloadState);
		if(fItems[aItemIndex]->downloadState == ENotDownloaded)
		{
			TShowInfoArray &fItems = iPodcastModel.ActiveShowList();

			if(!fItems[aItemIndex]->downloadState == EDownloaded)
			{
				iPodcastModel.FeedEngine().AddDownload(fItems[aItemIndex]);
			}
			// play the podcast if downloaded and its not currently downloading
			else if(fItems[aItemIndex]->downloadState !=  EDownloading)
			{
				iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);
			}
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