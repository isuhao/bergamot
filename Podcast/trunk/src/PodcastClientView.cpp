#include <badesca.h>
#include <eikchlst.h>
#include <QikScrollableContainer.h>
#include <QikRowLayoutManager.h>
#include <QikGridLayoutManager.h>
#include <QikBuildingBlock.h>
#include <QikCommand.h>
#include "PodcastClient.hrh"
#include "PodcastClientView.h"
#include "PodcastClientGlobals.h"
#include <PodcastClient.rsg>
#include <QikGridLayoutManager.h>
#include "e32debug.h"
#include <eiklabel.h>


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
        container->ConstructL(EFalse);
        CleanupStack::Pop(container);
        iContainer = container;
                            
        // Create a layout manager to be used inside the container
        CQikRowLayoutManager* rowlayout = CQikRowLayoutManager::NewLC();
        container->SetLayoutManagerL(rowlayout);
        CleanupStack::Pop(rowlayout);        
        
        User::LeaveIfError(serverSession.Connect());
        iPlayer = CMdaAudioPlayerUtility::NewL(*this);
        /*TRAPD(error, iPlayer->OpenFileL(_L("d:\\test.mp3")));
        if (error != KErrNone) {
    	RDebug::Print(_L("Error: %d"), error);
        }*/
        
        TRAPD(error, ListAllPodcastsL());
        
        RDebug::Print(_L("ListAllPodcastsL error: %d"), error);
        
    }

void CPodcastClientView::CreatePodcastListItem(CQikScrollableContainer* container, int id, TPtrC fileName)
	{
	// Create the System Building Block and set the caption.
	CQikBuildingBlock* block = CQikBuildingBlock::CreateSystemBuildingBlockL(EQikCtCaptionedTwolineBuildingBlock);
	container->AddControlLC(block, EMyViewBuildingBlockBase+id);
	block->ConstructL();
	block->SetUniqueHandle(EMyViewBuildingBlockBase+id);
	block->SetDividerBelow(ETrue);
	block->SetCaptionL(fileName, EQikItemSlot1); //the slot ids are defined in qikon.hrh
	
	CEikLabel *lbl = new CEikLabel();
	lbl->ConstructL();
	lbl->SetText(_L("Wongo")); //fileName);
	block->AddControlLC(lbl, EQikItemSlot2);
	CleanupStack::Pop(lbl);
	CleanupStack::Pop(block);
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
		CreatePodcastListItem(iContainer, iControlIdCount++, entry.iName);
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

void CPodcastClientView::ListAllPodcastsL()
{
	RFs rfs;
	rfs.Connect();
	
	TBuf<100> podcastDir;
	podcastDir.Copy(_L("c:\\logs\\"));

	RDebug::Print(_L("Listing podcasts"));
	
	ListDirL(rfs, podcastDir);
	RDebug::Print(_L("Listing complete"));
}

void CPodcastClientView::HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType) {

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