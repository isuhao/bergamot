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
		UpdateListboxItemsL();
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
				aCommand->SetInvisible(EFalse);
			}
		}
		break;
	default:
		break;
	}


	return aCommand;
}



_LIT(KShowsTitleFormat, "%d / %d");
void CPodcastClientShowsView::UpdateListboxItemsL()
{
	if(IsVisible())
	{
		CQikCommandManager& comMan = CQikCommandManager::Static();

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
				comMan.SetType(*this, EPodcastUpdateFeed, EQikCommandTypeScreen);
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
			comMan.SetType(*this, EPodcastUpdateFeed, EQikCommandTypeItem);
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, KNullDesC());
		}
		else
		{
			HBufC* titleBuffer = HBufC::NewLC(KShowsTitleFormat().Length()+8);
			titleBuffer->Des().Format(KShowsTitleFormat, unplayed, len);
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
			CleanupStack::PopAndDestroy(titleBuffer);

		}

		comMan.SetInvisible(*this, EPodcastUpdateFeed, iCurrentCategory != EShowFeedShows);
	}
}


void CPodcastClientShowsView::HandleListBoxEventL(CQikListBox * /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	CQikCommandManager& comMan = CQikCommandManager::Static();
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		{
		TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		if(aItemIndex>= 0 && aItemIndex < fItems.Count())
		{
			if(fItems[aItemIndex]->iDownloadState == EDownloaded)
			{
				comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
				comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_PLAY_CMD);
				comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_PLAY_CMD);
			}
			else if(fItems[aItemIndex]->iDownloadState == ENotDownloaded)
			{
				comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
				comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_DOWNLOAD_CMD);
				comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_FEEDS_DOWNLOAD_SHORT_CMD);
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
		}
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			TShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			if(aItemIndex>=0 && aItemIndex< fItems.Count())
			{
				RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->iTitle), fItems[aItemIndex]->iDownloadState);
				// add to downloads if not already downloading
				if(fItems[aItemIndex]->iDownloadState == ENotDownloaded)
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
				}
			}
		}
		break;
	default:
		break;
	}
}
