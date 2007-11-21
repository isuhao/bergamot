#include <eikdialg.h>
#include <qikcommand.h>
#include <qikcontent.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>

#include "PodcastClientAddFeedDlg.h"
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

void CPodcastClientFeedView::ConstructL()
{
	CPodcastClientView::ConstructL();
	iPodcastModel.FeedEngine().AddObserver(this);
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
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_TITLE);

    ViewConstructFromResourceL(R_PODCAST_FEEDVIEW_UI_CONFIGURATIONS);

	CPodcastClientView::ViewConstructL();
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *iViewLabel);
}


void CPodcastClientFeedView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
{
	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);


	SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID) );
}


CQikCommand* CPodcastClientFeedView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands)
{
	switch(aCommand->Id())
	{
	case EPodcastViewFeeds:
		aCommand = NULL;
		break;
	case EQikListBoxCmdSelect:
		{
			aCommand->SetTextL(R_PODCAST_SHOW_CMD);
		}
		break;
	case EQikSoftkeyCmdSelectCategory:
		{	
			aCommand->SetTextL(R_PODCAST_FEEDS_CATEGORY);
		}
		break;
	default:
		break;
	}

	return aCommand;
}

void CPodcastClientFeedView::FeedInfoUpdated(const TFeedInfo& aFeedInfo)
{

	if (ViewContext() == NULL) {
		return;
	}
	
	// Since a title might have been updated
	if(iProgressAdded)
	{
		ViewContext()->RemoveAndDestroyProgressInfo();
		iProgressAdded = EFalse;
	}

	TFeedInfoArray feeds;
	CleanupClosePushL(feeds);
	iPodcastModel.FeedEngine().GetFeeds(feeds);

	TInt index = feeds.Find(&aFeedInfo);

	if(index != KErrNotFound)
	{
		MQikListBoxModel& model(iListbox->Model());
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	
		if(data != NULL)
		{
			data->SetTextL(aFeedInfo.iTitle, EQikListBoxSlotText1);
			data->SetTextL(aFeedInfo.iDescription, EQikListBoxSlotText2);
			data->Close();
			model.DataUpdatedL(index);
		}
		model.ModelEndUpdateL();
	}

	CleanupStack::PopAndDestroy();// close feeds
}

void CPodcastClientFeedView::FeedDownloadUpdatedL(TInt aPercentOfCurrentDownload)
{
	if (ViewContext() == NULL) {
		return;
	}
	if(aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload<100)
	{
		if(!iProgressAdded)
		{
			ViewContext()->AddProgressInfoL(EEikProgressTextPercentage, 100);

			iProgressAdded = ETrue;
		}

		ViewContext()->SetAndDrawProgressInfo(aPercentOfCurrentDownload);
	}
	else if(iProgressAdded)
	{
		ViewContext()->RemoveAndDestroyProgressInfo();
		ViewContext()->DrawNow();
		iProgressAdded = EFalse;
	}
}

void CPodcastClientFeedView::UpdateListboxItemsL()
{
	if(IsVisible())
	{		
		TFeedInfoArray feeds;
		CleanupClosePushL(feeds);
		
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
		
		iPodcastModel.FeedEngine().GetFeeds(feeds);
		int len = feeds.Count();
		MQikListBoxData* listBoxData;
		
		if (len > 0) {
			for (int i=0;i<len;i++) {
				TInt bitmap = EMbmPodcastclientFeeds_40x40;
				TInt mask = EMbmPodcastclientFeeds_40x40m;
				
				listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
				CleanupClosePushL(*listBoxData);
				TFeedInfo *fi = feeds[i];
				listBoxData->AddTextL(fi->iTitle, EQikListBoxSlotText1);
				listBoxData->AddTextL(fi->iDescription, EQikListBoxSlotText2);
				CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
				CleanupStack::PushL(content);
				listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
				CleanupStack::Pop(content);
				
				CleanupStack::PopAndDestroy(); // close listbox data
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
		CleanupStack::PopAndDestroy();// close feeds
	}
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
			TFeedInfoArray feeds;
			CleanupClosePushL(feeds);
			iPodcastModel.FeedEngine().GetFeeds(feeds);
			iPodcastModel.ActiveShowList().Reset();
			iPodcastModel.SetActiveFeedInfo(*feeds[aItemIndex]);
			TVwsViewId showsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(showsView,  TUid::Uid(EShowFeedShows), KNullDesC8());
			CleanupStack::PopAndDestroy();// close feeds
		}
		break;
	default:
		break;
	}
}

void CPodcastClientFeedView::HandleCommandL(CQikCommand& aCommand)
{
	switch(aCommand.Id())
	{
	case EPodcastAddFeed:
		{
			CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel);
			if(dlg->ExecuteLD(R_PODCAST_ADD_FEED_DLG))
			{
				UpdateListboxItemsL();
			}
		}break;		
	case EPodcastEditFeed:
		{
		}break;

	case EPodcastDeleteFeed:
		{
		}break;
	case EPodcastUpdateFeed:
		{
			if(iListbox != NULL)
			{
				TInt index = iListbox->CurrentItemIndex();
				TFeedInfoArray feeds;
				CleanupClosePushL(feeds);
				iPodcastModel.FeedEngine().GetFeeds(feeds);
				if(index >= 0 && index <feeds.Count())
				{
					if (feeds[index]->iUrl.Length()>0) {
						TBuf<256> buf;
						buf.Format(_L("Getting %S"), &feeds[index]->iTitle);
						User::InfoPrint(buf);
						iPodcastModel.FeedEngine().UpdateFeed(feeds[index]->iUid);
					} 
				}
				CleanupStack::PopAndDestroy();// close feeds
			}
		}
		break;
	default:
		CPodcastClientView::HandleCommandL(aCommand);
		break;
	}
}