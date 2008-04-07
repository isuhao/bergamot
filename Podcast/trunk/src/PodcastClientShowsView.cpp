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
_LIT(KChapterFormatting, "%03d");

const TInt KSizeBufLen = 64;

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
		case EPodcastMarkAsPlayed:
			{
			TInt index = iListbox->CurrentItemIndex();
			if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
			{
				iPodcastModel.ActiveShowList()[index]->SetPlayState(EPlayed);
				UpdateListboxItemsL();
			}
			}break;
		case EPodcastMarkAsUnplayed:
			{
			TInt index = iListbox->CurrentItemIndex();
			if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
			{
				iPodcastModel.ActiveShowList()[index]->SetPlayState(ENeverPlayed);
				UpdateListboxItemsL();
			}
			}break;
		case EPodcastSetOrderAudioBook:
			{
				if(iPodcastModel.ActiveFeedInfo() != NULL)
					{
					CPodcastClientSetAudioBookOrderDlg* dlg = new (ELeave) CPodcastClientSetAudioBookOrderDlg(iPodcastModel, iPodcastModel.ActiveFeedInfo()->Uid());
					dlg->ExecuteLD(R_PODCAST_AUDIOBOOK_PLAYORDERDLG);
					UpdateListboxItemsL();
					}
			}break;
		case EPodcastDeleteShowHardware:
		case EPodcastDeleteShow:
			{				
				TInt index = iListbox->CurrentItemIndex();

				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
					{
					TBool isBook = (iPodcastModel.ActiveShowList()[index]->ShowType() == EAudioBook);
					if(iEikonEnv->QueryWinL(isBook?R_PODCAST_REMOVE_CHAPTER_TITLE:R_PODCAST_DELETE_SHOW_TITLE, 
											isBook?R_PODCAST_REMOVE_CHAPTER_PROMPT:R_PODCAST_DELETE_SHOW_PROMPT))				
						{											
							if(isBook)
								{
								iPodcastModel.ShowEngine().DeleteShow(iPodcastModel.ActiveShowList()[index]->Uid(), EFalse);	
								}
							else
								{
								iPodcastModel.ShowEngine().DeleteShow(iPodcastModel.ActiveShowList()[index]->Uid());	
								}
						
							MQikListBoxModel& model(iListbox->Model());
							model.ModelBeginUpdateLC();
							model.RemoveDataL(index);
							model.ModelEndUpdateL();

							UpdateListboxItemsL();						
						}
					}
			}break;
		case EPodcastUpdateLibrary:
			HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
			User::InfoPrint(*str);
			CleanupStack::PopAndDestroy(str);
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
		case EPodcastDeleteAllPlayed:
			{
				if(iEikonEnv->QueryWinL(R_PODCAST_DELETE_PLAYED_TITLE, R_PODCAST_DELETE_PLAYED_PROMPT))				
				{
				iPodcastModel.ShowEngine().DeletePlayedShows();
				UpdateListboxItemsL();
				}
			}break;
		case EPodcastUpdateFeed:
			{
			// flag to know whether to disable catchup mode here or not,
			// otherwise we might disable an Update All initiated from
			// the feed view
			iDisableCatchupMode = EFalse;
			if (iPodcastModel.ActiveFeedInfo()->LastUpdated().Int64() == 0) {
					TBuf<200> message;
					TBuf<100> title;
					CEikonEnv::Static()->ReadResourceL(message, R_CATCHUP_FEED);
					CEikonEnv::Static()->ReadResourceL(title, R_CATCHUP_FEED_TITLE);
					if (CEikonEnv::Static()->QueryWinL(title, message)) {
						iPodcastModel.FeedEngine().SetCatchupMode(ETrue);
						iDisableCatchupMode = ETrue;
					}
			}
				
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
		case EPodcastRemoveAllDownloads:
			{
			iPodcastModel.ShowEngine().RemoveAllDownloads();
			UpdateListboxItemsL();
			if (iProgressAdded) {
				ViewContext()->RemoveAndDestroyProgressInfo();
			}
			}
			break;
		case EPodcastRemoveDownloadHardware:
		case EPodcastRemoveDownload:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					if (iPodcastModel.ShowEngine().RemoveDownload(iPodcastModel.ActiveShowList()[index]->Uid())) {
						if (index > 0) {
							MQikListBoxModel& model(iListbox->Model());
							model.ModelBeginUpdateLC();
							model.RemoveDataL(index);
							model.ModelEndUpdateL();
						}

						UpdateListboxItemsL();												
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
	if (iDisableCatchupMode) {
		iPodcastModel.FeedEngine().SetCatchupMode(EFalse);
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
		CShowInfo *showInfo = NULL;
		TInt index = iListbox->CurrentItemIndex();
		if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
		{
					showInfo = iPodcastModel.ActiveShowList()[index];
		}

		// we show progress bar only for pending shows and inside the feed to which
	    // the active download belongs
		// (iCurrentCategory == EShowPendingShows ||
		//iPodcastModel.ShowEngine().ShowDownloading()->FeedUid() == iPodcastModel.ActiveFeedInfo()->Uid()) &&

		// now we only show when the active downloa has focus in a list
		if(showInfo != NULL && showInfo->Uid() == iPodcastModel.ShowEngine().ShowDownloading()->Uid() &&				
				aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload < KOneHundredPercent)
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
	if (aShowInfo->ShowType() == EAudioBook) {
		aImageId = EMbmPodcastclientAudiobookchapter_40x40;
		aMaskId = EMbmPodcastclientAudiobookchapter_40x40m;
	} else {
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
	
	if(aShowInfo->ShowType() == EAudioBook)
	{
		infoSize.Format(KChapterFormatting(), aShowInfo->TrackNo());															
	}
	else
	{
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
				totSize.Format(KShowsSizeFormatMb(), (aShowInfo->ShowSize()+KSizeMb/2) / KSizeMb);
			}
			
			if(aSizeDownloaded < KSizeMb)
			{
				dlSize.Format(KShowsSizeFormatKb(), aSizeDownloaded / KSizeKb);
			}
			else
			{
				dlSize.Format(KShowsSizeFormatMb(), (aSizeDownloaded+KSizeMb/2) / KSizeMb);
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
				infoSize.Format(KShowsSizeFormatMb(), (aShowInfo->ShowSize()+KSizeMb/2) / KSizeMb);
			}
			
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
						if(si->ShowType() == EAudioBook)
							{
							showSize.Format(KChapterFormatting(), si->TrackNo());
							if(si->PlayTime() != 0)
								{
								TInt playtime = si->PlayTime();
								TInt hour = playtime/3600;
								playtime = playtime-(hour*3600);
								
								TInt sec = (playtime%60);
								TInt min = (playtime/60);
								showDate.Format(_L("%01d:%02d:%02d"),hour, min, sec);
								}
							else
								{								
									showDate = KNullDesC();								
								}
							}
						else
							{
							if(si->ShowSize() == 0) 
								{
								showSize = KNullDesC();
								} 
							else if(si->ShowSize() < KSizeMb)
								{
								showSize.Format(KShowsSizeFormatKb(), si->ShowSize() / KSizeKb);
								}
							else
								{
								showSize.Format(KShowsSizeFormatMb(), (si->ShowSize()+KSizeMb/2)/ KSizeMb);
								}

							if(si->PubDate().Int64() == 0) 
								{
								showDate = KNullDesC();
								} 
								else 
								{
								si->PubDate().FormatL(showDate, KDateFormat());
								}
							}					

						listBoxData->AddTextL(showSize, EQikListBoxSlotText4);												
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
	TInt itemCnt = fItems.Count();
	TBool removePurgeShowCmd = ETrue;
	TBool removeDownloadShowCmd = ETrue;
	TBool removeRemoveSuspendCmd = EFalse;
	TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
	TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));
	TBool removeSetPlayed = EFalse;
	comMan.SetInvisible(*this, EPodcastViewPlayer, !playingPodcast);

	if(iListbox != NULL)
	{
		TInt index = iListbox->CurrentItemIndex();

		if(index>= 0 && index < itemCnt)
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
			
			if(fItems[index]->PlayState() == EPlayed) {
				removeSetPlayed = ETrue;
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

	comMan.SetInvisible(*this, EPodcastDeleteShow, removePurgeShowCmd);
	comMan.SetInvisible(*this, EPodcastDeleteShowHardware, removePurgeShowCmd);
	comMan.SetInvisible(*this, EPodcastDeleteAllPlayed, (iCurrentCategory == EShowPendingShows || (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed():EFalse) || iPodcastModel.SettingsEngine().SelectUnplayedOnly()));
	comMan.SetChecked(*this, EPodcastShowUnplayedOnly, iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	
	comMan.SetAvailable(*this, EPodcastUpdateFeed, !updatingState);
	comMan.SetAvailable(*this, EPodcastDownloadShow, !updatingState);
	comMan.SetAvailable(*this, EPodcastShowUnplayedOnly, !updatingState);
	comMan.SetAvailable(*this, EPodcastMarkAllPlayed, !updatingState);
	comMan.SetAvailable(*this, EPodcastDeleteShow, !updatingState);
	comMan.SetAvailable(*this, EPodcastDeleteShowHardware, !updatingState);
	comMan.SetAvailable(*this, EPodcastDeleteAllPlayed, !updatingState);
	comMan.SetInvisible(*this, EPodcastViewPendingShows, EFalse);
	comMan.SetInvisible(*this, EPodcastViewDownloadedShows, EFalse);
	comMan.SetInvisible(*this, EPodcastSetOrderAudioBook, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue)));
	comMan.SetInvisible(*this, EPodcastRemoveAllDownloads, ETrue);
	comMan.SetInvisible(*this, EPodcastMarkAsPlayed, removeSetPlayed || !itemCnt);
	comMan.SetInvisible(*this, EPodcastMarkAsUnplayed, !removeSetPlayed || !itemCnt);

	switch(iCurrentCategory)
	{	
	case EShowPendingShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, ETrue);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, (removeRemoveSuspendCmd || !itemCnt));
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, (removeRemoveSuspendCmd || !itemCnt));
			comMan.SetInvisible(*this, EPodcastRemoveAllDownloads, !itemCnt);
			comMan.SetAvailable(*this, EPodcastRemoveAllDownloads, iPodcastModel.ShowEngine().DownloadsStopped());
			comMan.SetInvisible(*this, EPodcastStopDownloads, (iPodcastModel.ShowEngine().DownloadsStopped()));
			comMan.SetInvisible(*this, EPodcastResumeDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped()));			
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, ETrue);
			comMan.SetInvisible(*this, EPodcastViewPendingShows, ETrue);
			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAsPlayed, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAsUnplayed, ETrue);
		}break;
	case EShowDownloadedShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, EFalse);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
			comMan.SetInvisible(*this, EPodcastViewDownloadedShows, ETrue);
		}break;
	default:
		{
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
			
		}break;
	}	

	if (iCurrentCategory == EShowPendingShows) {
		HBufC* titleBuffer = NULL;
		
		if (iPodcastModel.ShowEngine().DownloadsStopped()) {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_DOWNLOADS_SUSPENDED);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			titleBuffer->Des().Format(*titleFormat, itemCnt);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		} else {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_DOWNLOAD);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			titleBuffer->Des().Format(*titleFormat, itemCnt);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		}
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);
		
	} else {
		TUint unplayed = 0;
		if(itemCnt == 0)
		{
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, KNullDesC());
		}
		else
		{
			
		
			for (TInt loop = 0;loop<itemCnt;loop++)
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

TKeyResponse CPodcastClientShowsView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	//RDebug::Print(_L("CPodcastClientShowsView::OfferKeyEventL"));
	CQikViewBase::OfferKeyEventL(aKeyEvent, aType);
	
	if(aType == EEventKey)
		{
			switch(aKeyEvent.iCode)
			{
			case '*':
			case EKeyLeftArrow:
				RDebug::Print(_L("CPodcastClientShowsView::OfferKeyEventL switching play state"));
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					if (iPodcastModel.ActiveShowList()[index]->PlayState() == EPlayed) {
						iPodcastModel.ActiveShowList()[index]->SetPlayState(ENeverPlayed);
					} else {
						iPodcastModel.ActiveShowList()[index]->SetPlayState(EPlayed);
					}

				UpdateListboxItemsL();
				return EKeyWasConsumed;
				}
			}
		}
	
	return EKeyWasNotConsumed;
	}
