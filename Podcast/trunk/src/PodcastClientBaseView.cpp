#include <QikContent.h>
#include <qikcommand.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>
#include "PodcastClientShowsView.h"
#include "PodcastClientBaseView.h"
#include "PodcastModel.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientBaseView* CPodcastClientBaseView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientBaseView* self = new (ELeave) CPodcastClientBaseView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientBaseView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID);
	}

void CPodcastClientBaseView::ViewConstructL()
{
    //RDebug::Print(_L("ViewConstructL"));
    ViewConstructFromResourceL(R_PODCAST_MAINVIEW_UI_CONFIGURATIONS);

	ViewContext()->AddTextL(EPodcastListViewContextLabel, KNullDesC(), EHCenterVCenter);

	// Get the list box and the list box model
	iListbox = LocateControlByUniqueHandle<CQikListBox>(EPodcastListViewListCtrl);
	iListbox->SetListBoxObserver(this);
	MQikListBoxModel& model(iListbox->Model());
	model.ModelBeginUpdateLC();
	
	MQikListBoxData* data = model.RetrieveDataL(0);	
	data->SetItemId(EBaseViewNewShows);
	data->Close();
	data = model.RetrieveDataL(1);	
	data->SetItemId(EBaseViewDownloadedShows);
	data->Close();
	data = model.RetrieveDataL(2);	
	data->SetItemId(EBaseViewFeeds);
	data->Close();
	model.ModelEndUpdateL();

    UpdateListboxItemsL();
}


CPodcastClientBaseView::CPodcastClientBaseView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
}

CPodcastClientBaseView::~CPodcastClientBaseView()
{
}

CQikCommand* CPodcastClientBaseView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
{
	switch(aCommand->Id())
	{
	case EPodcastViewMain:
	case EPodcastUpdateFeed:
	case EPodcastViewNewShows:
	case EPodcastViewDownloadedShows:
	case EPodcastUpdateAllFeeds:
		aCommand = NULL;
		break;
	default:
		break;
	}
	

	return aCommand;
}


void CPodcastClientBaseView::UpdateListboxItemsL()
{
 // Update already existing items perhaps for special layout?
	if(iListbox != NULL)
	{
		MQikListBoxModel& model(iListbox->Model());
		TInt cnt = model.Count();

		model.ModelBeginUpdateLC();
		if(iPodcastModel.PlayingPodcast() != NULL)
		{
			TBool found = EFalse;
			for(TInt loop = 0;loop <cnt;loop++)
			{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				TInt itemId = data->ItemId();
				
				
				if(itemId == EBaseViewPlayer)
				{	
					found = ETrue;
					data->SetTextL(iPodcastModel.PlayingPodcast()->iTitle, EQikListBoxSlotText2);
					data->Close();
					model.DataUpdatedL(loop);
					break;
				}

				data->Close();				
			}	
		
			if(!found)
			{
				MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal, 0);
				CleanupClosePushL(*listBoxData);
				listBoxData->SetItemId(EBaseViewPlayer);
				
				HBufC* playerTxt = iEikonEnv->AllocReadResourceLC(R_PODCAST_MAIN_PLAYER_CMD);
				listBoxData->AddTextL(playerTxt, EQikListBoxSlotText1);			
				listBoxData->AddTextL(iPodcastModel.PlayingPodcast()->iTitle, EQikListBoxSlotText2);			
				CleanupStack::Pop(playerTxt);
				
				CQikContent* content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientPlay_40x40, EMbmPodcastclientPlay_40x40m);
				
				CleanupStack::PushL(content);
				listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
				CleanupStack::Pop(content);
				
				CleanupStack::PopAndDestroy(); // close listbox data
			}
		}
		else
		{
			for(TInt loop = 0;loop <cnt;loop++)
			{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				TInt itemId = data->ItemId();
				data->Close();

				if(itemId == EBaseViewPlayer)
				{
					model.RemoveDataL(loop);
					break;
				}
			
			}	
		}
		model.ModelEndUpdateL();
		
	}
}


void CPodcastClientBaseView::HandleListBoxEventL(CQikListBox * /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);

	switch (aEventType)
	{
	default:
		break;
	case EEventHighlightMoved:
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			MQikListBoxModel& model(iListbox->Model());
			MQikListBoxData* data = model.RetrieveDataL(aItemIndex);	
			TInt itemId = data->ItemId();
			TUid newview = TUid::Uid(0);
			TUid messageUid = TUid::Uid(0);
			switch(itemId)
			{
			case EBaseViewPlayer:
				{
					newview = KUidPodcastPlayViewID;
				
				}break;
			case EBaseViewNewShows:
				{
					newview = KUidPodcastShowsViewID;
					messageUid = TUid::Uid(EShowNewShows);
				}break;
			case EBaseViewDownloadedShows:
				{
					newview = KUidPodcastShowsViewID;
					messageUid = TUid::Uid(EShowDownloadedShows);
				}break;
			case EBaseViewFeeds:
				{
					newview = KUidPodcastFeedViewID;
				}break;
			default:
				break;
			}
			if(newview.iUid != 0)
			{
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, newview);
				iQikAppUi.ActivateViewL(viewId,  messageUid, KNullDesC8());
			}
			
		}break;
	}
	
}
