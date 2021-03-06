#include <badesca.h>
#include <qikcommand.h>
#include <PodcastClient.rsg>
#include <e32debug.h>
#include <eikedwin.h>
#include <eiklabel.h>
#include <qikslider.h>
#include <coecobs.h>
#include <qikappui.h>
#include <eikimage.h>
#include <QikZoomDlg.h>
#include <devicekeys.h>
#include <e32keys.h>

#include "PodcastClientSettingsDlg.h"
#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientPlayView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"

const TInt KAudioTickerPeriod = 1000000;
const TInt KMaxCoverImageHeight = 96;

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
	delete iPlaybackTicker;
	delete iCurrentCoverImage;
	delete iBitmapConverter;
	}

/**
2nd stage construction of the view.
*/
void CPodcastClientPlayView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	iPlaybackTicker = CPeriodic::NewL(CActive::EPriorityStandard);
	iPodcastModel.SoundEngine().SetObserver(this);
	iPodcastModel.FeedEngine().AddObserver(this);
	iBitmapConverter = CQikImageConverter::NewL(iEikonEnv->FsSession(), *this);
	iPodcastModel.ShowEngine().AddObserver(this);
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientPlayView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
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

TKeyResponse CPodcastClientPlayView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	CQikViewBase::OfferKeyEventL(aKeyEvent, aType);
	if(aType == EEventKey)
	{
		switch(aKeyEvent.iCode)
		{
		case '*':
		case '6':
		case EDeviceKeyFourWayRight:
			if(iPodcastModel.SettingsEngine().Volume() < KMaxVolume)
			{
				iPodcastModel.SettingsEngine().SetVolume(iPodcastModel.SettingsEngine().Volume()+1);
			}
			break;
		case '4':
		case '#':
		case EDeviceKeyFourWayLeft:
				if(iPodcastModel.SettingsEngine().Volume() > 0)
			{
				iPodcastModel.SettingsEngine().SetVolume(iPodcastModel.SettingsEngine().Volume()-1);
			}
			break;
		}
	}
}


TInt CPodcastClientPlayView::PlayingUpdateStaticCallbackL(TAny* aPlayView)
{
	static_cast<CPodcastClientPlayView*>(aPlayView)->UpdatePlayStatusL();
	return KErrNone;
}

void CPodcastClientPlayView::UpdatePlayStatusL()
{
	CQikCommandManager& comMan = CQikCommandManager::Static();

	if(iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
	{
		comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PAUSE_CMD);
	}
	else
	{
		comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
	}


	comMan.SetDimmed(*this, EPodcastPlay, iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized);
	comMan.SetDimmed(*this, EPodcastStop, (iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized || iPodcastModel.SoundEngine().State() == ESoundEngineStopped));
	if(iProgress != NULL)
	{
		iProgress->SetDimmed(iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized );
		
		if(iPodcastModel.SoundEngine().PlayTime()>0)
		{
			TUint duration = iPodcastModel.SoundEngine().PlayTime();
			TUint pos = iPodcastModel.SoundEngine().Position().Int64()/1000000;
			iProgress->SetValue((100*pos)/duration);
			iProgress->DrawDeferred();
		}
		else
		{
			iProgress->SetValue(0);
			iProgress->DrawDeferred();
		}
	}
}

void CPodcastClientPlayView::PlaybackInitializedL()
{
	UpdateViewL();
}


void CPodcastClientPlayView::PlaybackStartedL()
{
	iPlaybackTicker->Cancel();
	iPlaybackTicker->Start(KAudioTickerPeriod, KAudioTickerPeriod, TCallBack(PlayingUpdateStaticCallbackL, this));
	UpdatePlayStatusL();
}

void CPodcastClientPlayView::PlaybackStoppedL()
{
	iPlaybackTicker->Cancel();
	UpdatePlayStatusL();
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
	CQikCommandManager& comMan = CQikCommandManager::Static();

	switch(aCommand.Id())
	{
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

	case EPodcastPlay:
		{

			if(iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
			{
				iPodcastModel.SoundEngine().Pause();
				comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
			}
			else
			{
				iPodcastModel.SoundEngine().Play();
				comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PAUSE_CMD);
			}
		}break;
	case EPodcastStop:
		{
			comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);

			iPodcastModel.SoundEngine().Stop();	
		}break;
	case EPodcastDownloadShow:
		{
			iPodcastModel.ShowEngine().AddDownload(iPodcastModel.PlayingPodcast());
			UpdateViewL();
		}break;
	case EPodcastViewMain:
		{			
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID);
			iQikAppUi.ActivateViewL(playView);
		}break;
	case EPodcastViewFeeds:
		{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID);
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
	iTimeLabel = LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewProgressTime);
	iInformationEdwin = LocateControlByUniqueHandle<CEikEdwin>(EPodcastPlayViewInformation);
	iInformationEdwin->CreateScrollBarFrameL();
	iInformationEdwin->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
	CEikScrollBarFrame::EAuto);
	iVolumeSlider = LocateControlByUniqueHandle<CQikSlider>(EPodcastPlayViewVolumeCtrl);

	iCoverImageCtrl = LocateControlByUniqueHandle<CEikImage>(EPodcastPlayViewCoverImage);

	ViewContext()->AddTextL(EPodcastPlayViewTitleCtrl, KNullDesC(), EHCenterVCenter);
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_CATEGORY);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
    }

void CPodcastClientPlayView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
{
	CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	SelectCategoryL(EShowAllShows);
	
	if(iPodcastModel.PlayingPodcast() == NULL && aPrevViewId.iAppUid != KUidPodcastClientID)
	{
		iPodcastModel.SetPlayingPodcast(iLastShowInfo);
	
	}

	iLastShowInfo = NULL;
	
	if(	iLastZoomLevel !=  iPodcastModel.ZoomState())
		{
			iLastZoomLevel = iPodcastModel.ZoomState();
			SetZoomFactorL(CQikAppUi::ZoomFactorL(iLastZoomLevel , *iEikonEnv));
		}

	UpdateViewL();
	SetParentView( aPrevViewId );
}

void CPodcastClientPlayView::ViewDeactivated()
{
	CQikViewBase::ViewDeactivated();

	if(iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.SoundEngine().State() != ESoundEnginePlaying && iPodcastModel.SoundEngine().State() != ESoundEnginePaused)
	{
		iLastShowInfo = iPodcastModel.PlayingPodcast();
		iPodcastModel.SetPlayingPodcast(NULL);
	}
}

void CPodcastClientPlayView::ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal)

{
	if(aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload < KOneHundredPercent && iPodcastModel.PlayingPodcast() == iPodcastModel.ShowEngine().ShowDownloading())
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

	if(aPercentOfCurrentDownload == KOneHundredPercent && iPodcastModel.PlayingPodcast() == iPodcastModel.ShowEngine().ShowDownloading())
	{
		// To update icon list status and commands
		TShowInfo* playingPodcast = iPodcastModel.PlayingPodcast();
		// Reset current podcast statsus
		iPodcastModel.PlayPausePodcastL(NULL);
		iPodcastModel.PlayPausePodcastL(playingPodcast);

		UpdateViewL();
	}

}

void CPodcastClientPlayView::ImageConverterEventL(TQikImageConverterEvent aMessage, TInt aErrCode)
{
	if(aErrCode == KErrNone && aMessage == MQikImageConverterObserver::EImageConvertComplete)
	{
		if(iCurrentCoverImage->SizeInPixels().iHeight<=KMaxCoverImageHeight)
		{
			iCoverImageCtrl->SetBitmap(iCurrentCoverImage);
			iCurrentCoverImage = NULL;
			RequestRelayout(this);
		}
		else
		{
			iBitmapConverter->RescaleImage(*iCurrentCoverImage, TSize(iCurrentCoverImage->SizeInPixels().iWidth, KMaxCoverImageHeight));
		}
	}
	else if(aErrCode == KErrNone && aMessage == MQikImageConverterObserver::EBitmapRescaleComplete)
	{
		iCoverImageCtrl->SetBitmap(iCurrentCoverImage);
		iCurrentCoverImage = NULL;
		RequestRelayout(this);
	}
	else if(aErrCode != KErrNone)
	{
		delete iCurrentCoverImage;
		iCurrentCoverImage = NULL;
	}
	
}

void CPodcastClientPlayView::UpdateViewL()
{
		CQikCommandManager& comMan = CQikCommandManager::Static();

		if(iPodcastModel.PlayingPodcast() != NULL)
		{

			TShowInfo showInfo = *iPodcastModel.PlayingPodcast();
			ViewContext()->ChangeTextL(EPodcastPlayViewTitleCtrl, showInfo.iTitle);
			TBuf<32> time = _L("00:00");
			TUint playtime = iPodcastModel.SoundEngine().PlayTime();
			if(playtime > 0)
			{
				time.Format(_L("%02d:%02d"), (playtime/60), (playtime%60));
			}

			if(iTimeLabel != NULL)
			{
				iTimeLabel->SetText(time);
				iTimeLabel->SetSize(iTimeLabel->MinimumSize());
			}

			iInformationEdwin->SetTextL(&showInfo.iDescription);

			iInformationEdwin->HandleTextChangedL();
			if(showInfo.iDownloadState == ENotDownloaded)
			{
				comMan.SetInvisible(*this, EPodcastDownloadShow, EFalse);
				comMan.SetInvisible(*this, EPodcastPlay, ETrue);
			}
			else if(showInfo.iDownloadState != EDownloaded)
			{
				comMan.SetInvisible(*this, EPodcastPlay, ETrue);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			}
			else
			{
				comMan.SetInvisible(*this, EPodcastPlay, EFalse);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			}

			TFeedInfo* feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(showInfo.iFeedUid);
			
			if(feedInfo != NULL && feedInfo->iImageFileName.Length()>0)
			{
				if(feedInfo->iImageFileName != iLastImageFileName)
				{
					iLastImageFileName = feedInfo->iImageFileName;
					if(!iBitmapConverter->IsActive())
					{
						TRAPD(err, iBitmapConverter->LoadImageDataL(feedInfo->iImageFileName));
						if(err == KErrNone)
						{
							iBitmapConverter->ConvertToBitmapL(iCurrentCoverImage, 0);
							iCoverImageCtrl->MakeVisible(ETrue);				
						}
						else
						{
							iLastImageFileName = KNullDesC();
						}
					}
				}
			}
			else
			{
				iLastImageFileName = KNullDesC();
			}
		}
		else
		{	
			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
		}

	
		UpdatePlayStatusL();

		RequestRelayout(this);
}

