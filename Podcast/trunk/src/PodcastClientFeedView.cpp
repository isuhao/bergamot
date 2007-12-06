#include <eikdialg.h>
#include <qikcommand.h>
#include <qikcontent.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>

#include "PodcastClientAddFeedDlg.h"
#include "PodcastClientFeedView.h"
#include "PodcastModel.h"
#include "ShowEngine.h"
const TInt KMaxFeedNameLength = 100;
const TInt KADayInHours = 24;
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
	case EPodcastUpdateFeed:
	case EPodcastViewFeeds:
	case EPodcastPurgeFeed:
	case EPodcastPurgeShow:
	case EPodcastDeleteAllShows:
	case EPodcastDownloadShow:
	case EPodcastRemoveDownload:
	case EPodcastStopDownloads:
	case EPodcastResumeDownloads:
	case EPodcastMarkAllPlayed:
	case EPodcastShowUnplayedOnly:
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

void CPodcastClientFeedView::UpdateFeedInfoDataL(CFeedInfo* aFeedInfo,  MQikListBoxData* aListboxData)
{
	TBuf<KMaxShortDateFormatSpec*2> updatedDate;
	aListboxData->SetTextL(aFeedInfo->Title(), EQikListBoxSlotText1);
	//aListboxData->SetTextL(aFeedInfo->Description(), EQikListBoxSlotText2);
	
	TUint unplayedCount = 0;
	TUint showCount = 0;
	TBuf<100> unplayedShows;

	iPodcastModel.ShowEngine().GetStatsByFeed(aFeedInfo->Uid(), showCount, unplayedCount);
	unplayedShows.Format(_L("%d/%d shows"), unplayedCount, showCount);
	aListboxData->SetEmphasis(unplayedCount > 0);					
	aListboxData->SetTextL(unplayedShows, EQikListBoxSlotText2);
	
	if (aFeedInfo->LastUpdated().Int64() == 0) {
		updatedDate.Zero();
	}else {
		TTime now;
		TTimeIntervalHours interval;
		now.UniversalTime();
		now.HoursFrom(aFeedInfo->LastUpdated(), interval);
		if (interval.Int() < KADayInHours) {
			aFeedInfo->LastUpdated().FormatL(updatedDate, KTimeFormat());
		}else {
			aFeedInfo->LastUpdated().FormatL(updatedDate, KDateFormat());
		}
	}

	aListboxData->SetTextL(updatedDate, EQikListBoxSlotText3);
}

void CPodcastClientFeedView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
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

	RFeedInfoArray feeds;
	CleanupClosePushL(feeds);
	iPodcastModel.FeedEngine().GetFeeds(feeds);

	TInt index = feeds.Find(aFeedInfo);

	if(index != KErrNotFound)
	{
		MQikListBoxModel& model(iListbox->Model());
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	

		if(data != NULL)
		{
			CleanupClosePushL(*data);
			UpdateFeedInfoDataL(aFeedInfo, data);
			CleanupStack::PopAndDestroy(); // close data
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
		RFeedInfoArray feeds;
		CleanupClosePushL(feeds);
		iPodcastModel.FeedEngine().GetFeeds(feeds);
		int len = feeds.Count();
		TBool allUidsMatch = EFalse;
		MQikListBoxModel& model(iListbox->Model());

		if(len == iListbox->Model().Count())
		{
			TInt itemId = 0;
			for(TInt loop = 0;loop< len ;loop++)
			{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				itemId = data->ItemId();
				data->Close();
				if(feeds[loop]->Uid() != itemId)
				{						
					break;
				}
				
				allUidsMatch = ETrue;
			}
		}

		if(allUidsMatch)
		{
			model.ModelBeginUpdateLC();

			for(TInt loop = 0;loop< len ;loop++)
			{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				CleanupClosePushL(*data);
				UpdateFeedInfoDataL(feeds[loop], data);
				CleanupStack::PopAndDestroy();// close data;
				model.DataUpdatedL(loop);
			}

			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
		}
		else
		{
			iListbox->RemoveAllItemsL();
			
			// Informs the list box model that there will be an update of the data.
			// Notify the list box model that changes will be made after this point.
			// Observe that a cleanupitem has been put on the cleanupstack and 
			// will be removed by ModelEndUpdateL. This means that you have to 
			// balance the cleanupstack.
			// When you act directly on the model you always need to encapsulate 
			// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
			model.ModelBeginUpdateLC();
			
			TBuf<KMaxFeedNameLength> itemName;
			
			MQikListBoxData* listBoxData;
			TBuf<KMaxShortDateFormatSpec*2> updatedDate;
			TBuf<100> unplayedShows;
			
			if (len > 0) {
				for (int i=0;i<len;i++) {
					TInt bitmap = EMbmPodcastclientFeeds_40x40;
					TInt mask = EMbmPodcastclientFeeds_40x40m;
					
					listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					CFeedInfo *fi = feeds[i];
					listBoxData->SetItemId(fi->Uid());
					listBoxData->AddTextL(fi->Title(), EQikListBoxSlotText1);
					//listBoxData->AddTextL(fi->Description(), EQikListBoxSlotText2);
					TUint unplayedCount = 0;
					TUint showCount = 0;
					iPodcastModel.ShowEngine().GetStatsByFeed(fi->Uid(), showCount, unplayedCount);
					unplayedShows.Format(_L("%d/%d shows"), unplayedCount, showCount);
					listBoxData->SetEmphasis(unplayedCount > 0);					
					
					listBoxData->AddTextL(unplayedShows, EQikListBoxSlotText2);

					if (fi->LastUpdated().Int64() == 0) {
						updatedDate.Zero();
					}else {
						TTime now;
						TTimeIntervalHours interval;
						now.UniversalTime();
						now.HoursFrom(fi->LastUpdated(), interval);
						if (interval.Int() < 24) {
							fi->LastUpdated().FormatL(updatedDate, KTimeFormat());
						}else {
							fi->LastUpdated().FormatL(updatedDate, KDateFormat());
						}
					}
					listBoxData->AddTextL(updatedDate, EQikListBoxSlotText3);
					
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
		}
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
			RFeedInfoArray feeds;
			CleanupClosePushL(feeds);
			iPodcastModel.FeedEngine().GetFeeds(feeds);

			if(aItemIndex >= 0 && aItemIndex < feeds.Count())
			{
				iPodcastModel.ActiveShowList().Reset();
				iPodcastModel.SetActiveFeedInfo(feeds[aItemIndex]);
				TVwsViewId showsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
				iQikAppUi.ActivateViewL(showsView,  TUid::Uid(EShowFeedShows), KNullDesC8());
			}
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
			if(iListbox != NULL)
			{
				TInt index = iListbox->CurrentItemIndex();
			}
		}break;

	case EPodcastDeleteFeed:
		{
			if(iListbox != NULL)
			{
				TInt index = iListbox->CurrentItemIndex();
				MQikListBoxModel& model(iListbox->Model());
				MQikListBoxData* data = model.RetrieveDataL(index);	
				if(data != NULL)
				{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_FEED_TITLE, R_PODCAST_REMOVE_FEED_PROMPT))
					{
						iPodcastModel.FeedEngine().RemoveFeed(data->ItemId());
						iListbox->RemoveItemL(index);
					}
					data->Close();
				}	
			}
		}break;
	case EPodcastPurgeFeed:
		{
			if(iListbox != NULL)
			{
				TInt index = iListbox->CurrentItemIndex();
			}
		}break;
	default:
		CPodcastClientView::HandleCommandL(aCommand);
		break;
	}
}