#include <PodcastClient.rsg>
#include <QikCategoryModel.h>
#include <qikcommand.h>
#include <qikcontent.h>
#include <QikListBoxLayoutPair.h>
#include <QikListBoxLayoutElement.h>
#include <QikListBoxLayout.h>
#include <QikListBoxLayoutProperties.h>
#include <eikdialg.h>

#include <podcastclient.mbg>
#include "PodcastClientShowsView.h"
#include "PodcastClientPlayView.h"
#include "PodcastClientSettingsDlg.h"
#include "PodcastModel.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
_LIT(KSizeDownloadingOf, "%S/%S");


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
	iPodcastModel.ShowEngine().RemoveObserver(this);
	iPodcastModel.FeedEngine().RemoveObserver(this);
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
	TPodcastClientShowCategory previousCategory = iCurrentCategory;

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

	if(aPrevViewId == TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID) && iCurrentCategory == EShowFeedShows)
	{
		SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID));
	}
	else if(aPrevViewId.iAppUid == KUidPodcastClientID && iCurrentCategory != EShowFeedShows )
	{
		SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID));
	}
	
	UpdateFeedUpdateStateL();
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
		case EPodcastSetOrderAudioBook:
			{
				if(iPodcastModel.ActiveFeedInfo() != NULL)
					{
					CPodcastClientSetAudioBookOrderDlg* dlg = new (ELeave) CPodcastClientSetAudioBookOrderDlg(iPodcastModel, iPodcastModel.ActiveFeedInfo()->Uid());
					dlg->ExecuteLD(R_PODCAST_AUDIOBOOK_PLAYORDERDLG);
					UpdateListboxItemsL();
					}
			}break;
		case EPodcastPurgeShow:
			{				
				if(iEikonEnv->QueryWinL(R_PODCAST_PURGE_SHOW_TITLE, R_PODCAST_PURGE_SHOW_PROMPT))				
				{
					TInt index = iListbox->CurrentItemIndex();
					if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
					{
						if(iPodcastModel.ActiveShowList()[index]->IsBookFile())
							{
							iPodcastModel.ShowEngine().RemoveShow(iPodcastModel.ActiveShowList()[index]->Uid());	
							}
						else
							{
							iPodcastModel.ShowEngine().PurgeShow(iPodcastModel.ActiveShowList()[index]->Uid());	
							}
						UpdateListboxItemsL();
					}
				}
			}break;
		case EPodcastUpdateLibrary:
			iPodcastModel.ShowEngine().CheckFiles();
			break;
		case EPodcastShowUnplayedOnly:
			{
			iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(!iPodcastModel.SettingsEngine().SelectUnplayedOnly());
			UpdateListboxItemsL();
			}break;
		case EPodcastMarkAllPlayed:
			iPodcastModel.ShowEngine().SetSelectionPlayed();
			UpdateListboxItemsL();
			break;
		case EPodcastDownloadShow:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					iPodcastModel.ShowEngine().AddDownload(iPodcastModel.ActiveShowList()[index]);
					UpdateShowItemL(iPodcastModel.ActiveShowList()[index]);
				
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
					HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
					User::InfoPrint(*str);
					CleanupStack::PopAndDestroy(str);
					TRAPD(error, iPodcastModel.FeedEngine().UpdateFeedL(iPodcastModel.ActiveFeedInfo()->Uid()));
					
					if (error != KErrNone) {
						HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_ERROR);
						User::InfoPrint(*str);
						CleanupStack::PopAndDestroy(str);
					}
				} 
			}
			break;
		case EPodcastRemoveDownload:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					if (iPodcastModel.ShowEngine().RemoveDownload(iPodcastModel.ActiveShowList()[index]->Uid())) {
						MQikListBoxModel& model(iListbox->Model());
						model.ModelBeginUpdateLC();
						model.RemoveDataL(index);
						model.ModelEndUpdateL();
					}
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

	UpdateCommandsL();
	
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
		UpdateFeedUpdateStateL();
		// Title might have changed
		UpdateListboxItemsL();
	}
}

void CPodcastClientShowsView::FeedUpdateCompleteL(TUint aFeedUid)
{
	if(iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
	{
		UpdateFeedUpdateStateL();		
	}
}

void CPodcastClientShowsView::FeedUpdateAllCompleteL()
{
}

void CPodcastClientShowsView::FeedDownloadUpdatedL(TUint aFeedUid, TInt /*aPercentOfCurrentDownload*/)
	{
	if(iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
			UpdateFeedUpdateStateL();
		}
	}

void CPodcastClientShowsView::UpdateFeedUpdateStateL()
	{
	TBool listboxDimmed = EFalse;

	if(iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid())
		{
		listboxDimmed = ETrue;
		}
	
	if((iListbox->IsDimmed() && !listboxDimmed) || (!iListbox->IsDimmed() && listboxDimmed))
		{
		iListbox->SetDimmed(listboxDimmed);	
		UpdateCommandsL();
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
		{
		delete aCommand;
		 return NULL;
		}
		

	switch(aCommand->Id())
	{		
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
	switch(aShowInfo->DownloadState())
	{
	case EDownloaded:
		if (aShowInfo->PlayState() == EPlaying) {
			aImageId = EMbmPodcastclientShow_playing_40x40;
			aMaskId = EMbmPodcastclientShow_playing_40x40m;
		} else {
			aImageId = EMbmPodcastclientShow_40x40;
			aMaskId = EMbmPodcastclientShow_40x40m;
		}
		break;	
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

void CPodcastClientShowsView::UpdateShowItemDataL(CShowInfo* aShowInfo, MQikListBoxData* aListboxData, TInt aSizeDownloaded)
{
	TBuf<32> infoSize;
	TInt bitmap = EMbmPodcastclientShow_40x40;
	TInt mask = EMbmPodcastclientShow_40x40m;
	
	GetShowIcons(aShowInfo, bitmap, mask);
	CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
	CleanupStack::PushL(content);
	aListboxData->SetIconL(content, EQikListBoxSlotLeftMediumIcon1);
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
	
	aListboxData->SetTextL(aShowInfo->Title(), EQikListBoxSlotText1);
	aListboxData->SetTextL(infoSize, EQikListBoxSlotText4);
	aListboxData->SetEmphasis(aShowInfo->PlayState() == ENeverPlayed);					
}

void CPodcastClientShowsView::UpdateShowItemL(CShowInfo* aShowInfo, TInt aSizeDownloaded)
{
	// First find the item
	TInt index = iPodcastModel.ActiveShowList().Find(aShowInfo);
	MQikListBoxModel& model(iListbox->Model());

	if(index != KErrNotFound && index < model.Count())
	{
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	
		CleanupClosePushL(*data);
		UpdateShowItemDataL(aShowInfo, data, aSizeDownloaded);
		CleanupStack::PopAndDestroy();// close data
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
	
		SetAppTitleNameL(KNullDesC());

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
			iPodcastModel.ShowEngine().SelectShowsDownloaded();
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
		RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		len = fItems.Count();
		
		if (iListbox != NULL)
		{
			MQikListBoxModel& model(iListbox->Model());
			TBool allUidsMatch = EFalse;

			if(len == iListbox->Model().Count())
			{		
				allUidsMatch = ETrue;
				TInt itemId = 0;
				for(TInt loop = 0;loop< len ;loop++)
				{
					MQikListBoxData* data = model.RetrieveDataL(loop);	
					itemId = data->ItemId();
					data->Close();
					if(fItems[loop]->Uid() != itemId)
					{			
						allUidsMatch = EFalse;
						break;
					}					
				}
			}

			if(allUidsMatch)
			{
				model.ModelBeginUpdateLC();
				for(TInt loop = 0;loop< len ;loop++)
				{											
					MQikListBoxData* data = model.RetrieveDataL(loop);	
					CleanupClosePushL(*data);
					UpdateShowItemDataL(fItems[loop], data);									
					CleanupStack::PopAndDestroy();//close data
					model.DataUpdatedL(loop);
				}

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
				
				if (len > 0) {
					for (TInt i=0;i<len;i++) {
						MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
						CleanupClosePushL(*listBoxData);
						CShowInfo *si = fItems[i];
						listBoxData->SetItemId(si->Uid());
						listBoxData->AddTextL(si->Title(), EQikListBoxSlotText1);
						//listBoxData->AddTextL(si->iDescription, EQikListBoxSlotText2);
						if(si->ShowSize() == 0) {
							showSize.Format(_L("Unknown"));
						} else if(si->ShowSize() < KSizeMb)
						{
							showSize.Format(KShowsSizeFormatKb(), si->ShowSize() / KSizeKb);
						}
						else
						{
							showSize.Format(KShowsSizeFormatMb(), si->ShowSize() / KSizeMb);
						}
						listBoxData->AddTextL(showSize, EQikListBoxSlotText4);
						
						if(si->PubDate().Int64() == 0) {
							showDate.Format(_L("Unknown"));
						} else {
							si->PubDate().FormatL(showDate, KDateFormat());
						}
						listBoxData->AddTextL(showDate, EQikListBoxSlotText3);
						
						listBoxData->SetEmphasis(si->PlayState() == ENeverPlayed);										
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
					HBufC* noItems=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_NO_ITEMS);

					listBoxData->AddTextL(*noItems, EQikListBoxSlotText1);
					CleanupStack::PopAndDestroy(noItems);
					CleanupStack::PopAndDestroy();
				}
				
				// Informs that the update of the list box model has ended
				model.ModelEndUpdateL();
			}
			
			
		}	
		
		UpdateCommandsL();

	}
}

void CPodcastClientShowsView::UpdateCommandsL()
{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TBool removePurgeShowCmd = ETrue;
	TBool removeDownloadShowCmd = ETrue;
	TBool removeRemoveSuspendCmd = EFalse;
	TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
	TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));

	comMan.SetInvisible(*this, EPodcastViewPlayer, !playingPodcast);

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
			}
			else
			{
				if(fItems[index]->DownloadState() == EQueued)
				{
					comMan.SetTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);
				}
				else
				{					
					comMan.SetTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					removeRemoveSuspendCmd = ETrue;
				}
			}

			removePurgeShowCmd = fItems[index]->DownloadState() != EDownloaded;

			if(fItems[index]->DownloadState() == ENotDownloaded)
			{
				 removeDownloadShowCmd = EFalse;
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

	comMan.SetInvisible(*this, EPodcastUpdateFeed, (iCurrentCategory != EShowFeedShows || iPodcastModel.ActiveFeedInfo()->IsBookFeed()));
	comMan.SetInvisible(*this, EPodcastDownloadShow, removeDownloadShowCmd);

	comMan.SetInvisible(*this, EPodcastPurgeShow, removePurgeShowCmd);
	comMan.SetInvisible(*this, EPodcastPurgeFeed, (iCurrentCategory == EShowPendingShows || iCurrentCategory == EShowDownloadedShows));
	comMan.SetInvisible(*this, EPodcastDeleteAllShows, (iCurrentCategory != EShowDownloadedShows));
	comMan.SetChecked(*this, EPodcastShowUnplayedOnly, iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	
	
	comMan.SetAvailable(*this, EPodcastUpdateFeed, !updatingState);
	comMan.SetAvailable(*this, EPodcastDownloadShow, !updatingState);
	comMan.SetAvailable(*this, EPodcastShowUnplayedOnly, !updatingState);
	comMan.SetAvailable(*this, EPodcastMarkAllPlayed, !updatingState);
	comMan.SetAvailable(*this, EPodcastDeleteAllShows, !updatingState);
	comMan.SetAvailable(*this, EPodcastPurgeShow, !updatingState);
	comMan.SetAvailable(*this, EPodcastPurgeFeed, !updatingState);
	comMan.SetInvisible(*this, EPodcastViewPendingShows, EFalse);
	comMan.SetInvisible(*this, EPodcastViewDownloadedShows, EFalse);
	comMan.SetInvisible(*this, EPodcastSetOrderAudioBook, iPodcastModel.ActiveFeedInfo()?!iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue);

	switch(iCurrentCategory)
	{	
	case EShowPendingShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, ETrue);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, removeRemoveSuspendCmd);
			comMan.SetInvisible(*this, EPodcastStopDownloads, iPodcastModel.ShowEngine().DownloadsStopped());
			comMan.SetInvisible(*this, EPodcastResumeDownloads, !iPodcastModel.ShowEngine().DownloadsStopped());			
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, ETrue);
			comMan.SetInvisible(*this, EPodcastViewPendingShows, ETrue);
			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);

			
		}break;
	case EShowDownloadedShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, EFalse);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
			comMan.SetInvisible(*this, EPodcastViewDownloadedShows, ETrue);
			comMan.SetInvisible(*this, EPodcastDeleteAllShows, ETrue);
		}break;
	default:
		{
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
		}break;
	}
	TInt cnt = iPodcastModel.ActiveShowList().Count();

	if (iCurrentCategory == EShowPendingShows) {
		HBufC* titleBuffer = NULL;
		
		if (iPodcastModel.ShowEngine().DownloadsStopped()) {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_DOWNLOADS_SUSPENDED);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			titleBuffer->Des().Format(*titleFormat, cnt);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		} else {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_DOWNLOAD);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			int numDownloading = 0;
			int numQueued = 0;
			if (cnt > 0) {
				numDownloading = 1;
				numQueued = cnt - 1;
			}
			titleBuffer->Des().Format(*titleFormat, numDownloading, numQueued);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		}
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);
		
	} else {
		TUint unplayed = 0;
		if(cnt == 0)
		{
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, KNullDesC());
		}
		else
		{
			
		
			for (TInt loop = 0;loop<cnt;loop++)
			{
				unplayed+=(iPodcastModel.ActiveShowList()[loop]->PlayState() == ENeverPlayed);
			}
		}
		HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_FORMAT);
		HBufC* titleBuffer = HBufC::NewL(titleFormat->Length()+8);
		titleBuffer->Des().Format(*titleFormat, unplayed, iPodcastModel.ShowEngine().GetGrossSelectionLength());
		CleanupStack::PopAndDestroy(titleFormat);
		CleanupStack::PushL(titleBuffer);
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);	
	}
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
			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();

			if(aItemIndex>=0 && aItemIndex< fItems.Count())
			{
				RDebug::Print(_L("Handle event for podcast %S, downloadState is %d"), &(fItems[aItemIndex]->Title()), fItems[aItemIndex]->DownloadState());
				
				TPckgBuf<TInt> showUid;
				showUid() = fItems[aItemIndex]->Uid();
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
				iQikAppUi.ActivateViewL(viewId, TUid::Uid(KActiveShowUIDCmd), showUid);		

				UpdateCommandsL();
			}
		}
		break;
	default:
		break;
	}
}
