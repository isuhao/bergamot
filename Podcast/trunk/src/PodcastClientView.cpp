#include <badesca.h>
#include <eikchlst.h>
#include <qikscrollablecontainer.h>
#include <qikrowlayoutmanager.h>
#include <qikgridlayoutmanager.h>
#include <qikbuildingblock.h>
#include <qikcommand.h>
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include <PodcastClient.rsg>
#include <qikgridlayoutmanager.h>
#include "e32debug.h"
#include <eiklabel.h>
#include <eikcmbut.h>
#include "PodcastDownloader.h"
#include "HttpClient.h"


/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientView* CPodcastClientView::NewLC(CQikAppUi& aAppUi)
	{
	CPodcastClientView* self = new (ELeave) CPodcastClientView(aAppUi);
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
CPodcastClientView::CPodcastClientView(CQikAppUi& aAppUi) 
	: CQikViewBase(aAppUi, KNullViewId)
	{
	}

/**
Destructor for the view
*/
CPodcastClientView::~CPodcastClientView()
	{
	}

/**
2nd stage construction of the view.
*/
void CPodcastClientView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidListBoxListView);
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
		// Just issue simple info messages to show that
		// the commands have been selected
		default:
			CQikViewBase::HandleCommandL(aCommand);
			break;
		}
	}


void CPodcastClientView::ViewConstructL()
    {
    
        // Give a layout manager to the view
        CQikGridLayoutManager* gridlayout = CQikGridLayoutManager::NewLC();
        SetLayoutManagerL(gridlayout);
        CleanupStack::Pop(gridlayout);
                            
        // Create a container and add it to the view
        CQikScrollableContainer* container = new (ELeave) CQikScrollableContainer();
        Controls().AppendLC(container);
        container->ConstructL(ETrue);
        int one = 1; int zero = 0;
        container->ScrollBarsNeeded(one, zero);
        CleanupStack::Pop(container);
        iContainer = container;
                            
        // Create a layout manager to be used inside the container
        CQikRowLayoutManager* rowlayout = CQikRowLayoutManager::NewLC();
        container->SetLayoutManagerL(rowlayout);
        CleanupStack::Pop(rowlayout);        
        
        User::LeaveIfError(serverSession.Connect());
        iPlayer = CMdaAudioPlayerUtility::NewL(*this);

        
        iContainer->BeginUpdateLC();
        TRAPD(error, ListAllPodcastsL());
        RDebug::Print(_L("ListAllPodcastsL error: %d"), error);
        iContainer->EndUpdateL();        
        TSize sz = rowlayout->CalcMinimumSize(); //)Container->MinimumSize(); //iContainer->PageSize();
        RDebug::Print(_L("sz.iWidth: %d, sz.iHeight: %d"), sz.iWidth, sz.iHeight);
        TSize mySize;
        mySize.iWidth = 400;
        mySize.iHeight = 600;
       iContainer->SetPageSize(mySize);
    //ViewConstructFromResourceL(R_MY_SCROLL_BAR_UI_CONFIGURATIONS, R_MY_SCROLL_BAR_VIEW_CONTROLS);

	   	// create an active scheduler to use
	   	//CActiveScheduler* scheduler = new(ELeave) CActiveScheduler();
	   	//CleanupStack::PushL(scheduler);
	   //	CActiveScheduler::Install(scheduler);
	
	   	// Create and start the client
	   	CHttpClient* httpCli = CHttpClient::NewLC();
	   	httpCli->StartClientL();
	
	   	CleanupStack::PopAndDestroy(1); // httpCli, scheduler
    }

void CPodcastClientView::CreatePodcastListItem(TPodcastId *pID)
	{
	// Create the System Building Block and set the caption.
	CQikBuildingBlock* block = CQikBuildingBlock::CreateSystemBuildingBlockL(EQikCtCaptionedTwolineBuildingBlock);
	iContainer->AddControlLC(block, EMyViewBuildingBlockBase+pID->iId);
	block->ConstructL();
	//block->SetUniqueHandle(EMyViewBuildingBlockBase+pID->iId);
	block->SetDividerBelow(ETrue);
	block->SetCaptionL(pID->iTitle, EQikItemSlot1); //the slot ids are defined in qikon.hrh
	CEikCommandButton* cmdbutton = new (ELeave) CEikCommandButton();
    cmdbutton->SetTextL(_L("Play/Pause"));
    cmdbutton->SetObserver(this);
    cmdbutton->SetUniqueHandle(EMyViewCommandButton+pID->iId);
	block->AddControlLC(cmdbutton, EQikItemSlot2);
    CleanupStack::Pop(cmdbutton);
	
	CleanupStack::Pop(block);
}

void CPodcastClientView::PlayPausePodcast(TPodcastId *podcast) 
	{
	if (iPlayingPodcast == podcast->iId) {
		if (podcast->iPlaying) {
			RDebug::Print(_L("Pausing"));
			iPlayer->Pause();
			iPlayer->GetPosition(podcast->iPosition);
			podcast->iPlaying = EFalse;
		} else {
			RDebug::Print(_L("Resuming"));
			iPlayer->Play();
			podcast->iPlaying = ETrue;
		}
	} else {
		iPlayer->Stop();
		RDebug::Print(_L("Starting: %S"), &(podcast->iFileName));
		TRAPD(error, iPlayer->OpenFileL(podcast->iFileName));
	    if (error != KErrNone) {
	    	RDebug::Print(_L("Error: %d"), error);
	    }
	    podcast->iPlaying = ETrue;
		iPlayingPodcast = podcast->iId;

	}
}
void CPodcastClientView::HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType)
	{
	int id = aControl->UniqueHandle() - EMyViewCommandButton;
	TPodcastId *podcast = podcasts[id];
	switch(aEventType)
    {
    case EEventStateChanged:
    	RDebug::Print(_L("EEventStateChanged"));
    	RDebug::Print(_L("HandleControlEventL: %S, %S"), &(podcast->iTitle), &(podcast->iFileName));
    	PlayPausePodcast(podcast);
        // The internal state of the Command Button was changed.
        break;
                                
    case EEventRequestExit:
    	//RDebug::Print(_L("EEventRequestExit"));
		break;
                                
    case EEventRequestCancel:
    	//RDebug::Print(_L("EEventRequestCancel"));
        break;
                                
    case EEventRequestFocus:
    	//RDebug::Print(_L("EEventRequestFocus"));
        // The Command Button received a pointer down event
        break;                      
    case EEventPrepareFocusTransition:
    	//RDebug::Print(_L("EEventPrepareFocusTransition"));
        // A focus change is about to appear
        break;
                            
    case EEventInteractionRefused:
    	//RDebug::Print(_L("EEventInteractionRefused"));
        // The Command Button is dimmed and received a pointer down event.
        break;                
    default:
        break;
    }

	}

void CPodcastClientView::ListDirL(RFs &rfs, TDesC &folder) {
	CDirScan *dirScan = CDirScan::NewLC(rfs);
	RDebug::Print(_L("Listing dir: %S"), &folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);
	
	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (int i=0;i<dirPtr->Count();i++) {
		const TEntry &entry = (*dirPtr)[i];
		if (entry.IsDir())  {
			TBuf<100> subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDirL(rfs, subFolder);
		}
		RDebug::Print(entry.iName);
		int id = podcasts.Count();
		TBuf<100> fileName;
		fileName.Copy(folder);
		fileName.Append(entry.iName);

		TPodcastId *pID = new TPodcastId(id, fileName, entry.iName);
		podcasts.Append(pID);
		CreatePodcastListItem(pID);
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

void CPodcastClientView::ListAllPodcastsL()
{
	RFs rfs;
	rfs.Connect();
	
	TBuf<100> podcastDir;
	podcastDir.Copy(KPodcastDir);

	RDebug::Print(_L("Listing podcasts"));
	
	ListDirL(rfs, podcastDir);
	RDebug::Print(_L("Listing complete"));
}

void CPodcastClientView::MapcPlayComplete(TInt aError) {
	RDebug::Print(_L("Play Complete: %d"), aError);
}

void CPodcastClientView::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration) {
	RDebug::Print(_L("Init Complete: %d"), aError);
	
	if (aError == KErrNone) {
	  iPlayer->Play();
	  //  player->
	}
}
