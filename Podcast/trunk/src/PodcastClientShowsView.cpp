#include <PodcastClient.rsg>
#include <QikCategoryModel.h>
#include <qikcommand.h>
#include <QikListBoxLayoutPair.h>
#include <QikListBoxLayoutElement.h>
#include <QikListBoxLayout.h>
#include <QikListBoxLayoutProperties.h>

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
	CPodcastClientView::ViewConstructL();
	SetCategoryModel(NULL);
	delete iCategories; 
	iCategories = NULL;
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_SHOWS_CATEGORIES);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
	SetCategoryModelAsCommandsL();
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
	default:
		iCurrentCategory = EShowFeedShows;
		break;
	}	

	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
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

void CPodcastClientShowsView::FeedInfoUpdated(const TFeedInfo& aFeedInfo)
{
	if(aFeedInfo.iUid == iPodcastModel.ActiveFeedInfo().iUid)
	{
		iPodcastModel.SetActiveFeedInfo(aFeedInfo);
		// Title might have changed
		UpdateListboxItemsL();
	}
}

void CPodcastClientShowsView::ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload)
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
	}

	if(iPodcastModel.FeedEngine().ShowDownloading() != NULL)
	{
		UpdateShowItemL(iPodcastModel.FeedEngine().ShowDownloading());
	}
}

CQikCommand* CPodcastClientShowsView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
{
	if(aCommand->Type() == EQikCommandTypeCategory && CategoryHandleForCommandId(aCommand->Id()) == EShowFeedShows)
		 return NULL;

	switch(aCommand->Id())
	{
	case EPodcastViewPendingShows:
	case EPodcastViewDownloadedShows:
	case EPodcastViewNewShows:
	case EPodcastViewAllShows:
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

					if(iPodcastModel.ActiveShowList()[index]->iDownloadState == EDownloaded)
					{
						aCommand->SetTextL(R_PODCAST_FEEDS_PLAY_CMD);
						aCommand->SetShortTextL(R_PODCAST_FEEDS_PLAY_CMD);
					}
					else 
					{
						aCommand->SetTextL(R_PODCAST_VIEW_CMD);
						aCommand->SetShortTextL(R_PODCAST_VIEW_CMD);
					}			
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

void CPodcastClientShowsView::UpdateShowItemL(TShowInfo* aShowInfo)
{
	// First find the item
	TInt index = iPodcastModel.ActiveShowList().Find(aShowInfo);

	if(index != KErrNotFound)
	{
		MQikListBoxModel& model(iListbox->Model());
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	
		// Informs that the update of the list box model has ended
		if(aShowInfo->iDownloadState == EDownloaded)
		{
			
		}
		else
		{
			switch(aShowInfo->iDownloadState)
			{
			case ENotDownloaded:
				{
				}
				break;
			case EQueued:
				{
				}break;
			case EDownloading:
				{
				}break;
			default:
				break;
			}
		}
		model.ModelEndUpdateL();
		
	}
}

_LIT(KShowsTitleFormat, "%d / %d");
void CPodcastClientShowsView::UpdateListboxItemsL()
{
	if(IsVisible())
	{
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
			//	SetAppTitleNameL(iCategories->CategoryNameByHandle(iCurrentCategory));
				
				iPodcastModel.FeedEngine().SelectAllShows();
				break;
			case EShowNewShows:
				SelectCategoryL(EShowNewShows);
			//	SetAppTitleNameL(iCategories->CategoryNameByHandle(iCurrentCategory));
				break;		
			case EShowDownloadedShows:
				SelectCategoryL(EShowDownloadedShows);
			//	SetAppTitleNameL(iCategories->CategoryNameByHandle(iCurrentCategory));
				iPodcastModel.FeedEngine().SelectShowsByDownloadState(EDownloaded);
				break;
			case EShowPendingShows:
				SelectCategoryL(EShowPendingShows);
			//	SetAppTitleNameL(iCategories->CategoryNameByHandle(iCurrentCategory));
				iPodcastModel.FeedEngine().SelectShowsByDownloadState(EDownloading);
				break;
			default:

				iCategories->RenameCategoryL(EShowFeedShows, iPodcastModel.ActiveFeedInfo().iTitle);
				
				iPodcastModel.FeedEngine().SelectShowsByFeed(iPodcastModel.ActiveFeedInfo().iUid);
				SelectCategoryL(EShowFeedShows);
				break;
			}
			
			iPodcastModel.SetActiveShowList(iPodcastModel.FeedEngine().GetSelectedShows());

			TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			len = fItems.Count();

			if (len > 0) {
				for (TInt i=0;i<len;i++) {
					MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					TShowInfo *si = fItems[i];
					listBoxData->AddTextL(si->iTitle, EQikListBoxSlotText1);
					listBoxData->AddTextL(si->iDescription, EQikListBoxSlotText2);
					listBoxData->SetEmphasis(si->iPlayState == ENeverPlayed);
					unplayed+=(si->iPlayState == ENeverPlayed);
					CleanupStack::PopAndDestroy();	
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
	TShowInfoArray &fItems = iPodcastModel.ActiveShowList();

	if(iListbox != NULL)
	{
		TInt index = iListbox->CurrentItemIndex();

		if(index>= 0 && index < fItems.Count())
		{
			if(fItems[index]->iDownloadState == EDownloaded)
			{
				comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
				comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_PLAY_CMD);
				comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_PLAY_CMD);
			}
			else 
			{
				comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
				comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
				comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
			}			
		}
		else
		{
			comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
		}
	}
	else
	{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
	}
	
	if(fItems.Count() > 0)
	{
		comMan.SetType(*this, EPodcastUpdateFeed, EQikCommandTypeScreen);
	}
	else
	{
		comMan.SetType(*this, EPodcastUpdateFeed, EQikCommandTypeItem);
	}

	comMan.SetInvisible(*this, EPodcastUpdateFeed, iCurrentCategory != EShowFeedShows);
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
			TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			if(aItemIndex>=0 && aItemIndex< fItems.Count())
			{
				RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->iTitle), fItems[aItemIndex]->iDownloadState);
				iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
				iQikAppUi.ActivateViewL(viewId);
				// add to downloads if not already downloading
			/*	if(fItems[aItemIndex]->iDownloadState == ENotDownloaded)
				{
					iPodcastModel.FeedEngine().AddDownload(fItems[aItemIndex]);
				}
				// play the podcast if downloaded
				else if(fItems[aItemIndex]->iDownloadState == EDownloaded)
				{

					iPodcastModel.PlayPausePodcastL(fItems[aItemIndex]);	
					MQikListBoxModel& model(iListbox->Model());

					model.ModelBeginUpdateLC();
					MQikListBoxData* data = model.RetrieveDataL(aItemIndex);	
					data->SetEmphasis(EFalse);				
					// Informs that the update of the list box model has ended
					model.ModelEndUpdateL();
				}*/

				UpdateCommandsL();
			}
		}
		break;
	default:
		break;
	}
}
