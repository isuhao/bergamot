#include <badesca.h>
#include <qikscrollablecontainer.h>
#include <qikrowlayoutmanager.h>
#include <qikgridlayoutmanager.h>
#include <qikbuildingblock.h>
#include <qikcommand.h>
#include <QikZoomDlg.h>
#include <PodcastClient.rsg>
#include <qikgridlayoutmanager.h>
#include "PodcastClientShowsView.h"
#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"
#include "PodcastClientSettingsDlg.h"

/**
Constructor for the view.
Passes the application UI reference to the construction of the super class.

KNullViewId should normally be passed as parent view for the applications 
default view. The parent view is the logical view that is normally activated 
when a go back command is issued. KNullViewId will activate the system 
default view. 

@param aAppUi Reference to the application UI
*/
CPodcastClientView::CPodcastClientView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel) 
	: CQikViewBase(aAppUi, KNullViewId), iPodcastModel(aPodcastModel)
	{
	}

/**
Destructor for the view
*/
CPodcastClientView::~CPodcastClientView()
	{
	delete iViewLabel;
	}

/**
2nd stage construction of the view.
*/
void CPodcastClientView::ConstructL()
	{
	//RDebug::Print(_L("ConstructL"));
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CPodcastClientView::HandleCommandL(CQikCommand& aCommand)
{
	switch(aCommand.Id())
	{
	case EPodcastViewMain:
		{			
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID);
			iQikAppUi.ActivateViewL(playView);
		}break;
	case EPodcastViewPlayer:
		{
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
			iQikAppUi.ActivateViewL(playView);
		}
		break;		
	case EPodcastViewAllShows:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowAllShows), KNullDesC8());
		}break;
	case EPodcastViewNewShows:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowNewShows), KNullDesC8());
		}break;
	case EPodcastViewDownloadedShows:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowDownloadedShows), KNullDesC8());
		}break;
	case EPodcastViewPendingShows:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowPendingShows), KNullDesC8());
		}break;
	case EPodcastViewFeeds:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID);
			iQikAppUi.ActivateViewL(podcastsView);
		}break;		
	case EPodcastUpdateAllFeeds:
		{
			TFeedInfoArray array;
			CleanupClosePushL(array);
			iPodcastModel.FeedEngine().GetFeeds(array);
			
			for (int i=0;i<array.Count();i++) 
			{
				TBuf<1024> buf;
				buf.Format(_L("Getting %S"), &(array[i]->iTitle));
				User::InfoPrint(buf);
				iPodcastModel.FeedEngine().UpdateFeed(array[i]->iUid);				
			}
			CleanupStack::PopAndDestroy();//close array
		}
		break;
	case EPodcastSettings:
		{
			CPodcastClientSettingsDlg* dlg = new (ELeave) CPodcastClientSettingsDlg(iPodcastModel);
			dlg->ExecuteLD(R_PODCAST_SETTINGS_DLG);
		}break;

	case EPodcastZoomSetting:
		{
			// Launch the zoom dialog
			const TInt zoomFactor = CQikZoomDialog::RunDlgLD(iPodcastModel.ZoomState());
			
			// If zoom state have changed it will be stored to persistent
			// storage and a relayout will be performed
			if(iPodcastModel.SetZoomState(zoomFactor))
			{
				// Sets the zoom factor for the view
				iLastZoomLevel = zoomFactor;
				SetZoomFactorL(CQikAppUi::ZoomFactorL(zoomFactor , *iEikonEnv));
			}
		}
		break;
		// Just issue simple info messages to show that
		// the commands have been selected
	default:
		CQikViewBase::HandleCommandL(aCommand);
		break;
	}
}


void CPodcastClientView::ViewConstructL()
{
    //RDebug::Print(_L("ViewConstructL"));
	ViewContext()->AddTextL(EPodcastListViewContextLabel, KNullDesC(), EHRightVCenter);

	// Get the list box and the list box model
	iListbox = LocateControlByUniqueHandle<CQikListBox>(EPodcastListViewListCtrl);
	iListbox->SetListBoxObserver(this);
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_CATEGORY);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
}

void CPodcastClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
		CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
		MakeVisible(ETrue);
		SelectCategoryL(EShowAllShows);
		SetAppTitleNameL(KNullDesC());

		if(	iLastZoomLevel !=  iPodcastModel.ZoomState())
		{
			iLastZoomLevel = iPodcastModel.ZoomState();
			SetZoomFactorL(CQikAppUi::ZoomFactorL(iLastZoomLevel , *iEikonEnv));
		}

		UpdateListboxItemsL();
	}

void CPodcastClientView::ViewDeactivated()
{
	CQikViewBase::ViewDeactivated();
	MakeVisible(EFalse);
}

void CPodcastClientView::HandleControlEventL(CCoeControl */*aControl*/, TCoeEvent aEventType)
	{
	RDebug::Print(_L("HandleControlEvent"));
	}
