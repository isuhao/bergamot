#include <badesca.h>
#include <eikchlst.h>
#include <QikScrollableContainer.h>
#include <QikRowLayoutManager.h>
#include <QikGridLayoutManager.h>
#include <QikBuildingBlock.h>
#include <QikCommand.h>
#include "SyncClient.hrh"
#include "SyncClientView.h"
#include <SyncClient.rsg>
#include <QikGridLayoutManager.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include "SyncServerTimer.h"
#include "e32debug.h"
#include "f32file.h"
#include <BAUTILS.H>

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CSyncClientView object
*/
CSyncClientView* CSyncClientView::NewLC(CQikAppUi& aAppUi)
	{
	CSyncClientView* self = new (ELeave) CSyncClientView(aAppUi);
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
CSyncClientView::CSyncClientView(CQikAppUi& aAppUi) 
	: CQikViewBase(aAppUi, KNullViewId)
	{
	}

/**
Destructor for the view
*/
CSyncClientView::~CSyncClientView()
	{
	}

/**
2nd stage construction of the view.
*/
void CSyncClientView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CSyncClientView::ViewId()const
	{
	return TVwsViewId(KUidSyncClientID, KUidListBoxListView);
	}

/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CSyncClientView::HandleCommandL(CQikCommand& aCommand)
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


void CSyncClientView::CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16 &caption, int state ) {
	RDebug::Print(_L("CreateChoiceListItem START"));
	// Check if control already exists
	if (container->Controls().Find(EMyViewBuildingBlockBase+id).IsValid() == (int) ETrue) {
		return;
	}
	// Create the System Building Block and set the caption.
		
	CQikBuildingBlock* block = CQikBuildingBlock::CreateSystemBuildingBlockL(EQikCtCaptionedTwolineBuildingBlock);
	container->AddControlLC(block, EMyViewBuildingBlockBase+id);
	block->ConstructL();
	block->SetUniqueHandle(EMyViewBuildingBlockBase+id);
	block->SetDividerBelow(ETrue);
	block->SetCaptionL(caption, EQikItemSlot1); //the slot ids are defined in qikon.hrh
	
	_LIT(KChoiceListText1, "Manually");
	_LIT(KChoiceListText2, "Every 5 minutes");
	_LIT(KChoiceListText3, "Every hour");
	_LIT(KChoiceListText4, "Every four hours");
	_LIT(KChoiceListText5, "Daily");
	
	CEikChoiceList* chlst = new (ELeave) CEikChoiceList();
	block->AddControlLC(chlst, EQikItemSlot2);
	
	chlst->ConstructL(0 /*flags*/, 0 /*maxDisplayChar*/);
	chlst->SetObserver(this);
	CDesCArrayFlat* array = new(ELeave) CDesCArrayFlat(5);
	CleanupStack::PushL(array);
	array->AppendL(KChoiceListText1);
	array->AppendL(KChoiceListText2);
	array->AppendL(KChoiceListText3);
	array->AppendL(KChoiceListText4);
	array->AppendL(KChoiceListText5);
	chlst->SetArrayL(array);
	CleanupStack::Pop(array);
	RDebug::Print(_L("Setting handle to: %d"), EMyViewChoiceListBase+id);
	chlst->SetUniqueHandle(EMyViewChoiceListBase+id);

	chlst->SetCurrentItem(state);
	chlst->SetObserver(this);
	CleanupStack::Pop(chlst);
	CleanupStack::Pop(block);
	RDebug::Print(_L("CreateChoiceListItem END"));
}

void CSyncClientView::ShowSyncProfiles(CQikScrollableContainer* container) {
	RDebug::Print(_L("ShowSyncProfiles BEGIN"));
	RSyncMLSession session;
	
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone) {
		CEikonEnv::InfoWinL(_L("KichenSync"), _L("OpenL error"));
	}

	RSyncMLDataSyncJob job;
	
	RArray<TSmlProfileId> profiles;
	TRAP(error, session.ListProfilesL(profiles, ESmlDataSync));
	if (error!=KErrNone)
		{
		CEikonEnv::InfoWinL(_L("SyncClient"), _L("ListProfilesL error"));
		}
	
	TInt numItems = profiles.Count();
	RDebug::Print(_L("Found %d SyncML profiles"), numItems);
	TBuf<255> Buffer;
	TBool mustRedraw = EFalse;
	
	if (profiles.Count() != lastViewProfiles.Count()) {
		RDebug::Print(_L("Profile count mismatch, clearing list"));
		mustRedraw = ETrue;
	} else {
		for (int i=0;i<profiles.Count();i++) {
			if (lastViewProfiles.Find(profiles[i]) == KErrNotFound) {
				RDebug::Print(_L("Did not find new profile %d, clearing list"), profiles[i]);
				mustRedraw = ETrue;
				break;
			}
		}
	}
	
	if (mustRedraw) {
		lastViewProfiles = profiles;
		CCoeControlArray &array = iContainer->Controls();
	
		// this causes crashes when the container tries to refocus the control
		//iContainer->ResetAndDestroy();
	    // so instead we remove them manually
		for (int i=0;i<array.Count();i++) {
			iContainer->RemoveControl(*(array.At(i).iControl));
		}
		
		for (int i=0;i<profiles.Count();i++) {		
				
			RSyncMLDataSyncProfile profile;
			RDebug::Print(_L("Reading profile %d"), profiles[i]);
			TRAPD(error,profile.OpenL(session, profiles[i],ESmlOpenRead));
			if (error!=KErrNone)
				{
					_LIT(KRowFormatter, "OpenL error: %d");
					Buffer.Format(KRowFormatter, error);
					CEikonEnv::InfoWinL(_L("SyncClient"), Buffer);
				}
		
			TRAP(error, profile.DisplayName());
			if (error!=KErrNone)
				{
					_LIT(KRowFormatter, "DisplayName error: %d");
					Buffer.Format(KRowFormatter, error);
					CEikonEnv::InfoWinL(_L("SyncClient"), Buffer);
				}
			
			TSyncServerPeriod period = serverSession.GetTimer(profiles[i]);
			RDebug::Print(_L("DisplayName: %S, period=%d"), &profile.DisplayName(), period);
	
			if(profile.DisplayName().Compare(_L("iSync")) != 0) {
				int selection = 0;
				if (period != -1) {
					selection = period;
				}
				CreateChoiceListItem(container, profiles[i], (TDesC &)profile.DisplayName(), selection);
			}
	
			profile.Close();
	
		}
	} else {
		RDebug::Print(_L("Nothing new to show"));
	}
	session.Close();
	RDebug::Print(_L("ShowSyncProfiles END"));
}

void CSyncClientView::ViewConstructL()
    {
        // Give a layout manager to the view
        CQikGridLayoutManager* gridlayout = CQikGridLayoutManager::NewLC();
        SetLayoutManagerL(gridlayout);
        CleanupStack::Pop(gridlayout);
                            
        // Create a container and add it to the view
        CQikScrollableContainer* container = new (ELeave) CQikScrollableContainer();
        Controls().Reset();
        Controls().AppendLC(container);
        container->ConstructL(EFalse);
        CleanupStack::Pop(container);
        iContainer = container;
                            
        // Create a layout manager to be used inside the container
        CQikRowLayoutManager* rowlayout = CQikRowLayoutManager::NewLC();
        container->SetLayoutManagerL(rowlayout);
        CleanupStack::Pop(rowlayout);        
        
        User::LeaveIfError(serverSession.Connect());
    }

void CSyncClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	RDebug::Print(_L("ViewActivatedL START"));

	iContainer->BeginUpdateLC();
	ShowSyncProfiles(iContainer);
	iContainer->EndUpdateL();

	CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	RDebug::Print(_L("ViewActivatedL END"));
	}

void CSyncClientView::HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType) {
	if(aEventType == EEventStateChanged)
	{
		int profileId = aControl->UniqueHandle() - EMyViewChoiceListBase;
		CEikChoiceList* chlst = (CEikChoiceList*) aControl; //LocateControlByUniqueHandle<CEikChoiceList>(aControl->UniqueHandle());
		RDebug::Print(_L("Handle: %d"), aControl->UniqueHandle());
		TSyncServerPeriod period = (TSyncServerPeriod) chlst->CurrentItem();
		RDebug::Print(_L("HandleControlEvent for profileNum=%d, item=%d"), profileId, period);
		serverSession.SetTimer(profileId,period);
	}
}
