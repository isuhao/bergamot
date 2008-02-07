#include <qikcommand.h>
#include <qikcontent.h>
#include <QikMediaFileFolderUtils.h>
#include <QikSelectFileDlg.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>

#include "PodcastClientAudioBookDlg.h"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastClientFeedView.h"
#include "PodcastModel.h"
#include "ShowEngine.h"

const TInt KMaxFeedNameLength = 100;
const TInt KMaxUnplayedFeedsLength =64;
const TInt KADayInHours = 24;
const TInt KDefaultGran = 5;
_LIT(KUnknownUpdateDateString, "?/?");
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
	iPodcastModel.FeedEngine().RemoveObserver(this);
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
	iAudioBooksLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_AUDIOBOOKS_TITLE);
    ViewConstructFromResourceL(R_PODCAST_FEEDVIEW_UI_CONFIGURATIONS);
	CPodcastClientView::ViewConstructL();
	}


void CPodcastClientFeedView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	switch(aCustomMessageId.iUid)
	{
	case EFeedsAudioBooksMode:
		{
		iCurrentViewMode = EFeedsAudioBooksMode;
		}
		break;
	default:
		{
		iCurrentViewMode = EFeedsNormalMode;
		}
		break;
	}	

	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID) );
	}


CQikCommand* CPodcastClientFeedView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
	{
	switch(aCommand->Id())
		{
		case EQikListBoxCmdSelect:
			{
			aCommand->SetTextL(R_PODCAST_SHOW_CMD);
			break;
			}
			
		case EQikSoftkeyCmdSelectCategory:
			{	
			aCommand->SetTextL(R_PODCAST_FEEDS_CATEGORY);
			break;
			}
		
		default:
			break;
		}
	
	return aCommand;
	}

void CPodcastClientFeedView::UpdateFeedInfoDataL(CFeedInfo* aFeedInfo,  MQikListBoxData* aListboxData, TBool aIsUpdating )
	{
	TBuf<KMaxShortDateFormatSpec*2> updatedDate;
	aListboxData->SetTextL(aFeedInfo->Title(), EQikListBoxSlotText1);
	//aListboxData->SetTextL(aFeedInfo->Description(), EQikListBoxSlotText2);
	
	TUint unplayedCount = 0;
	TUint showCount = 0;
	TBuf<KMaxUnplayedFeedsLength> unplayedShows;

	if(aIsUpdating)
		{
			iEikonEnv->ReadResourceL(updatedDate, R_PODCAST_FEEDS_IS_UPDATING);
			unplayedShows = KNullDesC();			
		}
	else
	{
		iPodcastModel.ShowEngine().GetStatsByFeed(aFeedInfo->Uid(), showCount, unplayedCount);
		
		HBufC* templateStr = CEikonEnv::Static()->AllocReadResourceAsDes16LC(R_PODCAST_FEEDS_STATUS_FORMAT);
		unplayedShows.Format(*templateStr, unplayedCount, showCount);
		CleanupStack::PopAndDestroy(templateStr);
		
		aListboxData->SetEmphasis(unplayedCount > 0);					
				
		if (aFeedInfo->LastUpdated().Int64() == 0) 
		{
			updatedDate.Zero();
		}
		else 
		{
			TTime now;
			TTimeIntervalHours interval;
			now.HomeTime();
			now.HoursFrom(aFeedInfo->LastUpdated(), interval);
			if (interval.Int() < KADayInHours) 
			{
				aFeedInfo->LastUpdated().FormatL(updatedDate, KTimeFormat());
			}
			else 
			{
				aFeedInfo->LastUpdated().FormatL(updatedDate, KDateFormat());
			}
		}
	}
	aListboxData->SetDisabled(aIsUpdating);
	aListboxData->SetDimmed(aIsUpdating);

	aListboxData->SetTextL(unplayedShows, EQikListBoxSlotText2);
	aListboxData->SetTextL(updatedDate, EQikListBoxSlotText3);
	}

void CPodcastClientFeedView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
	{
	if (ViewContext() == NULL) 
		{
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
	MQikListBoxModel& model(iListbox->Model());

	if(index != KErrNotFound && index<model.Count())
		{
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	

		if(data != NULL)
			{
			CleanupClosePushL(*data);
			UpdateFeedInfoDataL(aFeedInfo, data);
			CleanupStack::PopAndDestroy(data);
			model.DataUpdatedL(index);
			}
		model.ModelEndUpdateL();
		}

	CleanupStack::PopAndDestroy(&feeds);
	}

void CPodcastClientFeedView::UpdateFeedInfoStatusL(TUint aFeedUid, TBool aIsUpdating)
	{
	RFeedInfoArray feeds;
	CleanupClosePushL(feeds);
	iPodcastModel.FeedEngine().GetFeeds(feeds);
	TInt cnt = feeds.Count();
	TInt index = KErrNotFound;
	while(index == KErrNotFound && cnt>0)
	{
		cnt--;
		if(feeds[cnt]->Uid() == aFeedUid)
		{
			index = cnt;
			break;
		}
	}
//	TInt index = feeds.Find(aFeedInfo);

	MQikListBoxModel& model(iListbox->Model());
	
	if(index != KErrNotFound && index < model.Count())
	{
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	
		
		if(data != NULL)
		{
			CleanupClosePushL(*data);
			UpdateFeedInfoDataL(feeds[index], data, aIsUpdating);
			CleanupStack::PopAndDestroy(data);
			model.DataUpdatedL(index);
		}
		model.ModelEndUpdateL();
	}
	
	CleanupStack::PopAndDestroy(&feeds);
	}

void CPodcastClientFeedView::FeedUpdateCompleteL(TUint aFeedUid)
	{
	RDebug::Print(_L("FeedUpdateCompleteL"));
	UpdateFeedInfoStatusL(aFeedUid, EFalse);
	}


void CPodcastClientFeedView::FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload)
	{
	if (ViewContext() == NULL) 
		{
		return;
		}
	
	UpdateFeedInfoStatusL(aFeedUid, ETrue);

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
		TInt len = feeds.Count();
		TBool allUidsMatch = EFalse;
		MQikListBoxModel& model(iListbox->Model());

		if(len == iListbox->Model().Count())
			{
			TUint itemId = 0;
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
				CleanupStack::PopAndDestroy(data);
				model.DataUpdatedL(loop);
				}

			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
			}
		else
			{
			iListbox->RemoveAllItemsL();
			
			model.ModelBeginUpdateLC();
									
			MQikListBoxData* listBoxData;
			TBuf<KMaxShortDateFormatSpec*2> updatedDate;
			TBuf<KMaxUnplayedFeedsLength> unplayedShows;
			
			if (len > 0) 
				{
				for (int i=0;i<len;i++) 
					{
					listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					
					CFeedInfo *fi = feeds[i];
					listBoxData->SetItemId(fi->Uid());
					listBoxData->AddTextL(fi->Title(), EQikListBoxSlotText1);
					//listBoxData->AddTextL(fi->Description(), EQikListBoxSlotText2);


					TUint unplayedCount = 0;
					TUint showCount = 0;
					iPodcastModel.ShowEngine().GetStatsByFeed(fi->Uid(), showCount, unplayedCount);
					HBufC* templateStr = CEikonEnv::Static()->AllocReadResourceAsDes16LC(R_PODCAST_FEEDS_STATUS_FORMAT);
					unplayedShows.Format(*templateStr, unplayedCount, showCount);
					CleanupStack::PopAndDestroy(templateStr);
					
					if (fi->LastUpdated().Int64() == 0) 
						{
						HBufC* neverStr = CEikonEnv::Static()->AllocReadResourceAsDes16LC(R_PODCAST_FEEDS_NEVER_UPDATED);
						updatedDate.Copy(*neverStr);
						CleanupStack::PopAndDestroy(neverStr);
						unplayedShows.Copy(KUnknownUpdateDateString());
						}
					else 
						{
						TTime now;
						TTimeIntervalHours interval;
						now.UniversalTime();
						now.HoursFrom(fi->LastUpdated(), interval);
						if (interval.Int() < 24) 
							{
							fi->LastUpdated().FormatL(updatedDate, KTimeFormat());
							}
						else 
							{
							fi->LastUpdated().FormatL(updatedDate, KDateFormat());
							}
						}
						
					listBoxData->SetEmphasis(unplayedCount > 0);					
					listBoxData->AddTextL(unplayedShows, EQikListBoxSlotText2);
					listBoxData->AddTextL(updatedDate, EQikListBoxSlotText3);
					
					CQikContent* content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientFeed_40x40, EMbmPodcastclientFeed_40x40m);
					CleanupStack::PushL(content);
					listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
					CleanupStack::Pop(content);
					
					CleanupStack::PopAndDestroy(listBoxData);
					}
				} 
			else 
				{
				listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
				CleanupClosePushL(*listBoxData);
					
				TBuf<KMaxFeedNameLength> itemName;
				iEikonEnv->ReadResourceL(itemName, R_PODCAST_FEEDS_NO_FEEDS);
				listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
				
				CleanupStack::PopAndDestroy(listBoxData);
				}

			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
			}
		
		// Update the contextbar so information matches the listbox content
		HBufC* templateStr = NULL;
		switch(iCurrentViewMode)
			{
		case EFeedsAudioBooksMode:
			{
			templateStr = iEikonEnv->AllocReadResourceAsDes16LC(R_PODCAST_BOOKS_TITLE_FORMAT);
			}break;
		default:
			{
			templateStr = iEikonEnv->AllocReadResourceAsDes16LC(R_PODCAST_FEEDS_TITLE_FORMAT);
			}
			break;
			}
		HBufC* titleBuffer = HBufC::NewLC(templateStr->Length()+8);
		titleBuffer->Des().Format(*templateStr, len);
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);
		CleanupStack::PopAndDestroy(templateStr);

		CleanupStack::PopAndDestroy();// close feeds
		}
	
	UpdateCommandsL();
	}


void CPodcastClientFeedView::HandleListBoxEventL(CQikListBox* /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
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
				
			CleanupStack::PopAndDestroy(&feeds);
			break;	
			}
		
		default:
			break;
		}
	}


void CPodcastClientFeedView::UpdateCommandsL()
	{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	
	
	switch(iCurrentViewMode)
	{
	case EFeedsAudioBooksMode:
		{
		SetAppTitleNameL(*iAudioBooksLabel);
		}
		break;
	default:
		{
		SetAppTitleNameL(*iViewLabel);
		}
		break;
	}	

	if (iListbox == NULL)
		return;
	TBool isBookMode = (iCurrentViewMode == EFeedsAudioBooksMode);
	RFeedInfoArray feeds;
	CleanupClosePushL(feeds);
	iPodcastModel.FeedEngine().GetFeeds(feeds);
	
	// hide commands that should not be visible in no feeds
	if (feeds.Count() == 0)
		{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
		comMan.SetInvisible(*this, EPodcastDeleteFeed, ETrue);
		comMan.SetInvisible(*this, EPodcastUpdateAllFeeds, ETrue);
		comMan.SetInvisible(*this, EPodcastEditFeed, ETrue);
		}
	else
		{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
		comMan.SetInvisible(*this, EPodcastDeleteFeed, isBookMode);
		comMan.SetInvisible(*this, EPodcastUpdateAllFeeds, isBookMode);
		comMan.SetInvisible(*this, EPodcastEditFeed, isBookMode);	
		comMan.SetInvisible(*this, EPodcastRemoveAudioBook, !isBookMode);
		}
	
	comMan.SetInvisible(*this, EPodcastAddNewAudioBook, !isBookMode);
	comMan.SetInvisible(*this, EPodcastAddFeed, isBookMode);

	comMan.SetInvisible(*this, EPodcastViewAudioBooks, isBookMode);
	comMan.SetInvisible(*this, EPodcastViewFeeds, !isBookMode);

	CleanupStack::PopAndDestroy(&feeds); 
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
			break;
			}
					
		case EPodcastEditFeed:
			{
			if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();
				RFeedInfoArray feeds;
				CleanupClosePushL(feeds);
				iPodcastModel.FeedEngine().GetFeeds(feeds);
				TInt cnt = feeds.Count();

				if(index< cnt)
					{
					CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel, feeds[index]);
					if(dlg->ExecuteLD(R_PODCAST_EDIT_FEED_DLG))
						{
						UpdateListboxItemsL();
						}
					}
				CleanupStack::PopAndDestroy();//close feeds array
				}
			break;
			}
			

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
				UpdateListboxItemsL();
				}
			break;	
			}
		
		case EPodcastPurgeFeed:
			{
				HandleAddNewAudioBookL();
			/*if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();
				}*/
			break;
			}

	case EPodcastAddNewAudioBook:
		{
		HandleAddNewAudioBookL();
		}break;
	case EPodcastRemoveAudioBook:
		{
		}break;
		default:
			CPodcastClientView::HandleCommandL(aCommand);
			break;
		}
	
	}

void CPodcastClientFeedView::HandleAddNewAudioBookL()
	{
	CDesCArrayFlat* mimeArray = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_NEW_AUDIOBOOK_MIMEARRAY);
	CleanupStack::PushL(mimeArray);
	CDesCArrayFlat* fileNameArray = new (ELeave) CDesCArrayFlat(KDefaultGran);
	CleanupStack::PushL(fileNameArray);
	HBufC* dialogTitle = iEikonEnv->AllocReadResourceLC(R_PODCAST_NEW_AUDIOBOOK_SELECT_FILES);
	TQikDefaultFolderDescription defaultFolder;
	defaultFolder.SetDefaultFolder(EQikFileHandlingDefaultFolderAudio);

	if(CQikSelectFileDlg::RunDlgLD(*mimeArray, *fileNameArray, defaultFolder, dialogTitle, EQikSelectFileDialogEnableMultipleSelect|EQikSelectFileDialogSortByName))
		{
		if(fileNameArray->Count() > 0)
			{
			CPodcastClientAudioBookDlg* titleDialog = new (ELeave) CPodcastClientAudioBookDlg(iPodcastModel, *fileNameArray);
			titleDialog->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG);
			}
		}

	CleanupStack::PopAndDestroy(dialogTitle);
	CleanupStack::PopAndDestroy(fileNameArray);
	CleanupStack::PopAndDestroy(mimeArray);
	}

