#include <PodcastClient.rsg>
#include <QikCategoryModel.h>
#include <qikcommand.h>
#include <qikcontent.h>
#include <QikListBoxLayoutPair.h>
#include <QikListBoxLayoutElement.h>
#include <QikListBoxLayout.h>
#include <QikListBoxLayoutProperties.h>
#include <podcastclient.mbg>
#include "PodcastClientShowsView.h"
#include "PodcastModel.h"
#include "ShowEngine.h"


_LIT(KShowsTitleFormat, "%d / %d");
_LIT(KShowsSizeFormatKb,"%dkB");
_LIT(KShowsSizeFormatMb,"%dMB");
_LIT(KSizeDownloadingOf, "%S/%S");

const TInt KSizeKb = 1024;
const TInt KSizeMb = 1024000;

const TInt KSizeBufLen = 16;

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
	return self;
	}

CPodcastClientShowsView::CPodcastClientShowsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
	iCurrentCategory = EShowAllShows;
}

void CPodcastClientShowsView::ConstructL()
{
	CPodcastClientView::ConstructL();
	iPodcastModel.FeedEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);
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
	iViewLabel = HBufC::NewL(1);

    ViewConstructFromResourceL(R_PODCAST_SHOWSVIEW_UI_CONFIGURATIONS);

	CPodcastClientView::ViewConstructL();

	SetCategoryModel(NULL);
	delete iCategories; 
	iCategories = NULL;
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_SHOWS_CATEGORIES);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
}

void CPodcastClientShowsView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
{
	switch(aCustomMessageId.iUid)
	{
	case EShowNewShows:
	case EShowAllShows:
	case EShowDownloadedShows:
	case EShowPendingShows:
		iCurrentCategory = (TPodcastClientShowCategory) aCustomMessageId.iUid;
		break;
	case EShowFeedShows:
		iCurrentCategory = EShowFeedShows;
		break;
	}	

	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);

	if(aPrevViewId == TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID))
	{
		SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID));
	}

}


void CPodcastClientShowsView::HandleCommandL(CQikCommand& aCommand)
{
	if(aCommand.Type() == EQikCommandTypeCategory)
	{
		iCurrentCategory = (TPodcastClientShowCategory) CategoryHandleForCommandId(aCommand.Id());
		UpdateListboxItemsL();
	}
	else
	{	
		switch(aCommand.Id())
		{
			
		case EPodcastPurgeShow:
			{				
				if(iEikonEnv->QueryWinL(R_PODCAST_PURGE_SHOW_TITLE, R_PODCAST_PURGE_SHOW_PROMPT))				
				{
					TInt index = iListbox->CurrentItemIndex();
					if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
					{
						iPodcastModel.ShowEngine().PurgeShow(iPodcastModel.ActiveShowList()[index]->Uid());
						UpdateShowItemL(iPodcastModel.ActiveShowList()[index]);
					}
				}
			}break;
				
		case EPodcastPurgeFeed:
			{
				if(iEikonEnv->QueryWinL(R_PODCAST_PURGE_FEED_TITLE, R_PODCAST_PURGE_FEED_PROMPT))				
				{
					if (iPodcastModel.ActiveFeedInfo()->Url().Length()>0) {
						iPodcastModel.ShowEngine().PurgeShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
						UpdateListboxItemsL();
					} 
				}
			}break;
		case EPodcastUpdateFeed:
			{
				if (iPodcastModel.ActiveFeedInfo()->Url().Length()>0) {
					User::InfoPrint(_L("Updating feed..."));
					iPodcastModel.FeedEngine().UpdateFeed(iPodcastModel.ActiveFeedInfo()->Uid());
				} 
			}
			break;
		case EPodcastRemoveDownload:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					iPodcastModel.ShowEngine().RemoveDownload(iPodcastModel.ActiveShowList()[index]->Uid());
					UpdateShowItemL(iPodcastModel.ActiveShowList()[index]);
				}
			}break;
		case EPodcastStopDownloads:
			{
				iPodcastModel.ShowEngine().StopDownloads();
			}break;
		case EPodcastResumeDownloads:
			{
				iPodcastModel.ShowEngine().ResumeDownloads();
			}break;
		default:
			CPodcastClientView::HandleCommandL(aCommand);
			break;
		}
	}
	
}

// Engine callback when new shows are available
void CPodcastClientShowsView::ShowListUpdated()
{
	UpdateListboxItemsL();
}

void CPodcastClientShowsView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
{
	if(iPodcastModel.ActiveFeedInfo() != NULL && aFeedInfo->Uid() == iPodcastModel.ActiveFeedInfo()->Uid())
	{
		iPodcastModel.SetActiveFeedInfo(aFeedInfo);
		// Title might have changed
		UpdateListboxItemsL();
	}
}

void CPodcastClientShowsView::ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt /*aBytesTotal*/)
{
	if(ViewContext() != NULL)
	{
		if(aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload < KOneHundredPercent)
		{
			if(!iProgressAdded)
			{
				ViewContext()->AddProgressInfoL(EEikProgressTextPercentage, KOneHundredPercent);

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

		if(aPercentOfCurrentDownload == KOneHundredPercent)
		{
			// To update icon list status and commands
			UpdateCommandsL();
			if(iCurrentCategory == EShowPendingShows && iPodcastModel.ShowEngine().ShowDownloading() != NULL)
			{
				// First find the item, to remove it if we are in the pending show list
				TInt index = iPodcastModel.ActiveShowList().Find(iPodcastModel.ShowEngine().ShowDownloading());
				
				if(index != KErrNotFound)
				{
					iListbox->RemoveItemL(index);
					iPodcastModel.ActiveShowList().Remove(index);
				}
			}
		}

		if(iPodcastModel.ShowEngine().ShowDownloading() != NULL)
		{
			UpdateShowItemL(iPodcastModel.ShowEngine().ShowDownloading(), aBytesOfCurrentDownload);
		}
	}
}

CQikCommand* CPodcastClientShowsView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
{
	if(aCommand->Type() == EQikCommandTypeCategory && CategoryHandleForCommandId(aCommand->Id()) == EShowFeedShows)
		 return NULL;

	switch(aCommand->Id())
	{
	case EPodcastAddFeed:
	case EPodcastEditFeed:
	case EPodcastDeleteFeed:
	case EPodcastUpdateAllFeeds:
		aCommand = NULL;
		break;
	case EQikListBoxCmdSelect:
		{
			if(iPodcastModel.ActiveShowList().Count() == 0)
			{			
				aCommand->SetInvisible(ETrue);
			}
			else
			{
				TInt index = iListbox->CurrentItemIndex();
				
				if(index>= 0 && index < iPodcastModel.ActiveShowList().Count())
				{					
					aCommand->SetInvisible(EFalse);	
					aCommand->SetTextL(R_PODCAST_VIEW_CMD);
					aCommand->SetShortTextL(R_PODCAST_VIEW_CMD);
				}
				else
				{
					aCommand->SetInvisible(ETrue);
				}
			}						
		}
		break;
	default:
		break;
	}


	return aCommand;
}

void CPodcastClientShowsView::GetShowIcons(CShowInfo* aShowInfo, TInt& aImageId, TInt& aMaskId)
{	
	aImageId = EMbmPodcastclientShow_40x40;
	aMaskId = EMbmPodcastclientShow_40x40m;

	if(aShowInfo->DownloadState() == EDownloaded)
	{
		if(	aShowInfo->PlayState() == EPlaying)
		{
			aImageId = EMbmPodcastclientShow_playing_40x40;
			aMaskId = EMbmPodcastclientShow_playing_40x40m;
		}
		else if(aShowInfo->PlayState() == ENeverPlayed)
		{
			aImageId = EMbmPodcastclientNew_40x40;
			aMaskId = EMbmPodcastclientNew_40x40m;
		}
		
	}
	else
	{
		switch(aShowInfo->DownloadState())
		{
		case ENotDownloaded:
			aImageId = EMbmPodcastclientNew_40x40;
			aMaskId = EMbmPodcastclientNew_40x40m;
			break;
		case EQueued:
			aImageId = EMbmPodcastclientQueued_40x40;
			aMaskId = EMbmPodcastclientQueued_40x40m;
			break;
		case EDownloading:
			aImageId = EMbmPodcastclientDownloading_40x40;
			aMaskId = EMbmPodcastclientDownloading_40x40m;
			break;
		}
	}
}

void CPodcastClientShowsView::UpdateShowItemL(CShowInfo* aShowInfo, TInt aSizeDownloaded)
{
	// First find the item
	TInt index = iPodcastModel.ActiveShowList().Find(aShowInfo);

	if(index != KErrNotFound)
	{
		TBuf<32> infoSize;

		MQikListBoxModel& model(iListbox->Model());
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	

		TInt bitmap = EMbmPodcastclientShow_40x40;
		TInt mask = EMbmPodcastclientShow_40x40m;

		GetShowIcons(aShowInfo, bitmap, mask);
		CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
		CleanupStack::PushL(content);
		data->SetIconL(content, EQikListBoxSlotLeftMediumIcon1);
		CleanupStack::Pop(content);

		if(aSizeDownloaded > 0)
		{
			TBuf<KSizeBufLen> dlSize;
			TBuf<KSizeBufLen> totSize;

			if(aShowInfo->ShowSize() < KSizeMb)
			{
				totSize.Format(KShowsSizeFormatKb(), aShowInfo->ShowSize() / KSizeKb);
			}
			else
			{
				totSize.Format(KShowsSizeFormatMb(), aShowInfo->ShowSize() / KSizeMb);
			}

			if(aSizeDownloaded < KSizeMb)
			{
				dlSize.Format(KShowsSizeFormatKb(), aSizeDownloaded / KSizeKb);
			}
			else
			{
				dlSize.Format(KShowsSizeFormatMb(), aSizeDownloaded / KSizeMb);
			}
			infoSize.Format(KSizeDownloadingOf(), &dlSize, &totSize);

		}
		else
		{
			if(aShowInfo->ShowSize() < KSizeMb)
			{
				infoSize.Format(KShowsSizeFormatKb(), aShowInfo->ShowSize() / KSizeKb);
			}
			else
			{
				infoSize.Format(KShowsSizeFormatMb(), aShowInfo->ShowSize() / KSizeMb);
			}
			
		}

		data->SetTextL(infoSize, EQikListBoxSlotText4);
	
		data->Close();
		model.DataUpdatedL(index);
		model.ModelEndUpdateL();
		
	}
}

void CPodcastClientShowsView::UpdateListboxItemsL()
{
	if(IsVisible())
	{
		TBuf<KSizeBufLen> showSize;
		TBuf<KMaxShortDateFormatSpec*2> showDate;

		TInt len = 0;
		TUint unplayed = 0;
		SetAppTitleNameL(KNullDesC());
		if (iListbox != NULL)
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
					
			switch(iCurrentCategory)
			{
			case EShowAllShows:
				SelectCategoryL(EShowAllShows);
				iPodcastModel.ShowEngine().SelectAllShows();
				break;
			case EShowNewShows:
				SelectCategoryL(EShowNewShows);
				iPodcastModel.ShowEngine().SelectNewShows();
				break;		
			case EShowDownloadedShows:
				SelectCategoryL(EShowDownloadedShows);
				iPodcastModel.ShowEngine().SelectShowsByDownloadState(EDownloaded);
				break;
			case EShowPendingShows:
				SelectCategoryL(EShowPendingShows);
				iPodcastModel.ShowEngine().SelectShowsDownloading();
				break;
			default:

				iCategories->RenameCategoryL(EShowFeedShows, iPodcastModel.ActiveFeedInfo()->Title());
				
				iPodcastModel.ShowEngine().SelectShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
				SelectCategoryL(EShowFeedShows);
				break;
			}
			
			iPodcastModel.SetActiveShowList(iPodcastModel.ShowEngine().GetSelectedShows());

			CShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			len = fItems.Count();

			if (len > 0) {
				for (TInt i=0;i<len;i++) {
					MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					CShowInfo *si = fItems[i];
					listBoxData->AddTextL(si->Title(), EQikListBoxSlotText1);
					//listBoxData->AddTextL(si->iDescription, EQikListBoxSlotText2);
					if(si->ShowSize() < KSizeMb)
					{
						showSize.Format(KShowsSizeFormatKb(), si->ShowSize() / KSizeKb);
					}
					else
					{
						showSize.Format(KShowsSizeFormatMb(), si->ShowSize() / KSizeMb);
					}
					listBoxData->AddTextL(showSize, EQikListBoxSlotText4);
					
					si->PubDate().FormatL(showDate, TShortDateFormatSpec());
					listBoxData->AddTextL(showDate, EQikListBoxSlotText3);

					listBoxData->SetEmphasis(si->PlayState() == ENeverPlayed);					
					unplayed+=(si->PlayState() == ENeverPlayed);
					CleanupStack::PopAndDestroy();	
					
					TInt bitmap = EMbmPodcastclientShow_40x40;
					TInt mask = EMbmPodcastclientShow_40x40m;

					GetShowIcons(si, bitmap, mask);
									
					CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
					CleanupStack::PushL(content);
					listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
					CleanupStack::Pop(content);
				}
			} else {		
				MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
				CleanupClosePushL(*listBoxData);
				
				listBoxData->AddTextL(_L("No items"), EQikListBoxSlotText1);
				CleanupStack::PopAndDestroy();
			}
			
			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
			
		}

		if(len == 0)
		{
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, KNullDesC());
		}
		else
		{
			HBufC* titleBuffer = HBufC::NewLC(KShowsTitleFormat().Length()+8);
			titleBuffer->Des().Format(KShowsTitleFormat, unplayed, len);
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
			CleanupStack::PopAndDestroy(titleBuffer);

		}

		UpdateCommandsL();

	}
}

void CPodcastClientShowsView::UpdateCommandsL()
{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	CShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TBool removeDownloadCmd = EFalse;
	TBool removePurgeShowCmd = ETrue;
	if(iListbox != NULL)
	{
		TInt index = iListbox->CurrentItemIndex();

		if(index>= 0 && index < fItems.Count())
		{		
			comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
			comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
			comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
			if(fItems[index]->DownloadState() != EQueued &&  fItems[index]->DownloadState() != EDownloading)
			{			
				removeDownloadCmd = ETrue;
			}
			removePurgeShowCmd = fItems[index]->DownloadState() != EDownloaded;
		
		}
		else
		{
			comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);		
			removeDownloadCmd = ETrue; 		
		}
	}
	else
	{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
		removeDownloadCmd = ETrue;
	}

	comMan.SetInvisible(*this, EPodcastUpdateFeed, iCurrentCategory != EShowFeedShows);

	TBool notshowDownloadCommands = (iCurrentCategory != EShowPendingShows);
	comMan.SetInvisible(*this, EPodcastRemoveDownload, (notshowDownloadCommands||removeDownloadCmd));
	comMan.SetInvisible(*this, EPodcastStopDownloads, notshowDownloadCommands);
	comMan.SetInvisible(*this, EPodcastResumeDownloads, notshowDownloadCommands);
	comMan.SetInvisible(*this, EPodcastPurgeShow, removePurgeShowCmd);

}


void CPodcastClientShowsView::HandleListBoxEventL(CQikListBox * /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		{
			UpdateCommandsL();
		}
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			CShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			if(aItemIndex>=0 && aItemIndex< fItems.Count())
			{
				RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->Title()), fItems[aItemIndex]->DownloadState());
				iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
				iQikAppUi.ActivateViewL(viewId);		

				UpdateCommandsL();
			}
		}
		break;
	default:
		break;
	}
}
