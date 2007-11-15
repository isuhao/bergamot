#include <badesca.h>
#include <qikcommand.h>
#include <PodcastClient.rsg>
#include <e32debug.h>
#include <eikedwin.h>
#include <eiklabel.h>
#include <qikslider.h>
#include <coecobs.h>
#include <qikappui.h>

#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientPlayView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientPlayView object
*/
CPodcastClientPlayView* CPodcastClientPlayView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientPlayView* self = new (ELeave) CPodcastClientPlayView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
Constructor for the view.
Passes the application UI reference to the construction of the super class.

KNullViewId should normally be passed as parent view for the applications 
default view. The parent view is the logical view that is normally activated 
when a go back command is issued. KNullViewId will activate the system 
default view. 

@param aAppUi Reference to the application UI
*/
CPodcastClientPlayView::CPodcastClientPlayView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel) 
	: CQikViewBase(aAppUi, KNullViewId), iPodcastModel(aPodcastModel)
	{
	}

/**
Destructor for the view
*/
CPodcastClientPlayView::~CPodcastClientPlayView()
	{
	}

/**
2nd stage construction of the view.
*/
void CPodcastClientPlayView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientPlayView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastClientPlayViewID);
	}

void CPodcastClientPlayView::HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType)
{
	CQikViewBase::HandleControlEventL(aControl, aEventType);

	if(aEventType == MCoeControlObserver::EEventStateChanged)
	{
		// Set position in current playback
		if(aControl == iProgress)
		{
			TInt value = iProgress->CurrentValue();
			TUint ptime = iPodcastModel.SoundEngine().PlayTime();
			if(ptime > 0)
			{
				iPodcastModel.SoundEngine().SetPosition((value*ptime)/100);
			}
		}
	}
}


/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CPodcastClientPlayView::HandleCommandL(CQikCommand& aCommand)
{
	switch(aCommand.Id())
	{
	case EPodcastPlay:
		{
			if(iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
			{
				iPodcastModel.SoundEngine().Pause();
			}
			else
			{
				iPodcastModel.SoundEngine().Play();
			}
		}break;
	case EPodcastStop:
		{
			iPodcastModel.SoundEngine().Stop();	
		}break;
	case EPodcastViewMain:
		{			
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientBaseViewID);
			iQikAppUi.ActivateViewL(playView);
		}break;
	case EPodcastViewPodcasts:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientPodcastsViewID);
			iQikAppUi.ActivateViewL(podcastsView);
		}break;
	case EPodcastViewFeeds:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastClientFeedViewID);
			iQikAppUi.ActivateViewL(podcastsView);
		}break;		
		// Just issue simple info messages to show that
		// the commands have been selected
	default:
		CQikViewBase::HandleCommandL(aCommand);
		break;
	}
}


void CPodcastClientPlayView::ViewConstructL()
    {
    //RDebug::Print(_L("ViewConstructL"));
    ViewConstructFromResourceL(R_PODCAST_PLAYVIEW_UI_CONFIGURATIONS);

	iProgress =LocateControlByUniqueHandle<CQikSlider>(EPodcastPlayViewProgressCtrl);
//	iTitleLabel = LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewTitleCtrl);
	iTimeLabel = LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewProgressTime);
	iInformationEdwin = LocateControlByUniqueHandle<CEikEdwin>(EPodcastPlayViewInformation);
	iInformationEdwin->CreateScrollBarFrameL();
	iInformationEdwin->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
	CEikScrollBarFrame::EAuto);
	iVolumeSlider = LocateControlByUniqueHandle<CQikSlider>(EPodcastPlayViewVolumeCtrl);
	ViewContext()->AddTextL(EPodcastPlayViewTitleCtrl, KNullDesC(), EHCenterVCenter);
    }

void CPodcastClientPlayView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	UpdateViewL();
	}

void CPodcastClientPlayView::UpdateViewL()
{
		if(iPodcastModel.PlayingPodcast() != NULL)
		{
			TShowInfo showInfo = *iPodcastModel.PlayingPodcast();
			ViewContext()->ChangeTextL(EPodcastPlayViewTitleCtrl, showInfo.title);
			TBuf<32> time = _L("00:00");
			TUint playtime = iPodcastModel.SoundEngine().PlayTime();
			if(playtime > 0)
			{
				time.Format(_L("%02d:%02d"), (playtime/60), (playtime%60));
			}
			iTimeLabel->SetText(time);
			iTimeLabel->SetSize(iTimeLabel->MinimumSize());

			iInformationEdwin->SetTextL(&showInfo.description);
			iInformationEdwin->SetTextL(&_L("Lots of text that has come from the description field in RSS, and perhaps even a picture!!"));

			iInformationEdwin->UpdateAllFieldsL();
			iInformationEdwin->HandleTextChangedL();
			if(iProgress != NULL)
			{			
				iProgress->SetValue(50);
			}
					
			RequestRelayout(this);
		}
}

