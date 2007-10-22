#include <badesca.h>
#include <eikchlst.h>
#include <QikScrollableContainer.h>
#include <QikRowLayoutManager.h>
#include <QikGridLayoutManager.h>
#include <QikBuildingBlock.h>
#include <QikCommand.h>
#include "KitchenSync.hrh"
#include "KitchenSyncView.h"
#include <KitchenSync.rsg>
#include <QikGridLayoutManager.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include "KitchenSyncTimer.h"
#include "e32debug.h"
#include "f32file.h"
#include <BAUTILS.H>

void CKitchenSyncData::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteInt32L(profileId);
	aStream.WriteInt32L(period);
	RDebug::Print(_L("Wrote %d, %d"), profileId, period);
}

void CKitchenSyncData::InternalizeL(RReadStream& aStream) {
	profileId = aStream.ReadInt32L();
	period = aStream.ReadInt32L();
	RDebug::Print(_L("Read %d, %d"), profileId, period);
}
	
/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CKitchenSyncView object
*/
CKitchenSyncView* CKitchenSyncView::NewLC(CQikAppUi& aAppUi)
	{
	CKitchenSyncView* self = new (ELeave) CKitchenSyncView(aAppUi);
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
CKitchenSyncView::CKitchenSyncView(CQikAppUi& aAppUi) 
	: CQikViewBase(aAppUi, KNullViewId)
	{
	}

/**
Destructor for the view
*/
CKitchenSyncView::~CKitchenSyncView()
	{
	}

/**
2nd stage construction of the view.
*/
void CKitchenSyncView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CKitchenSyncView::ViewId()const
	{
	return TVwsViewId(KUidKitchenSyncID, KUidListBoxListView);
	}

/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CKitchenSyncView::HandleCommandL(CQikCommand& aCommand)
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


void CKitchenSyncView::CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16 &caption, int state ) {
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
	chlst->SetUniqueHandle(EMyViewChoiceListBase+id);
	chlst->SetCurrentItem(state);
	chlst->SetObserver(this);
	CleanupStack::Pop(chlst);
	CleanupStack::Pop(block);
}

void CKitchenSyncView::LoadAllProfiles() {
	RDebug::Print(_L("LoadAllProfiles"));
	RSyncMLSession session;
	
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone)
		{
		CEikonEnv::InfoWinL(_L("KichenSync"), _L("OpenL error"));
		}
	RSyncMLDataSyncJob job;
	
	RArray<TSmlProfileId> profileArray;
	TRAP(error, session.ListProfilesL(profileArray, ESmlDataSync));
	if (error!=KErrNone)
		{
		CEikonEnv::InfoWinL(_L("KitchenSync"), _L("ListProfilesL error"));
		}
	
	TInt numItems = profileArray.Count();
	RDebug::Print(_L("Found %d SyncML profiles"), numItems);
	for (TInt i=0;i<numItems;i++) {		
		CKitchenSyncData *data = new CKitchenSyncData;
		data->period = -1;
		data->profileId = profileArray[i];
		data->timer = NULL;
		timerArray.Append(*data);
		RDebug::Print(_L("Created SyncData for profile=%d"), data->profileId);

	}
	
	session.Close();
}

void CKitchenSyncView::ShowAllProfiles(CQikScrollableContainer* container) {
	RDebug::Print(_L("ShowAllProfiles"));
	RSyncMLSession session;
	TRAPD(error, session.OpenL());
	
	TBuf<255> Buffer;
	for (int i=0;i<timerArray.Count();i++) {		
		CKitchenSyncData &data = timerArray[i];
			
		RSyncMLDataSyncProfile profile;
		RDebug::Print(_L("Reading profile %d"), data.profileId);
		TRAPD(error,profile.OpenL(session, data.profileId,ESmlOpenRead));
		if (error!=KErrNone)
			{
				_LIT(KRowFormatter, "OpenL error: %d");
				Buffer.Format(KRowFormatter, error);
				CEikonEnv::InfoWinL(_L("KitchenSync"), Buffer);
			}
	
		TRAP(error, profile.DisplayName());
		if (error!=KErrNone)
			{
				_LIT(KRowFormatter, "DisplayName error: %d");
				Buffer.Format(KRowFormatter, error);
				CEikonEnv::InfoWinL(_L("KitchenSync"), Buffer);
			}
		RDebug::Print(_L("DisplayName: %S, period=%d"), &profile.DisplayName(), data.period);

		if(profile.DisplayName().Compare(_L("iSync")) != 0) {
			int selection = 0;
			if (data.period != -1) {
				selection = data.period;
			}
			CreateChoiceListItem(container, i, (TDesC &)profile.DisplayName(), selection);
		}

		profile.Close();

	}
	
	session.Close();
}

void CKitchenSyncView::ViewConstructL()
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
        
        LoadAllProfiles();
        LoadSettings();
        ShowAllProfiles(iContainer);

    }

void CKitchenSyncView::SaveSettings() 
	{
	RDebug::Print(_L("SaveSettings"));
	RFs fsSession;
	fsSession.Connect(); 
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	RDebug::Print(_L("Folder: %S"), &KDirNameOfFileStore);
	RDebug::Print(_L("File: %S"), &KFullNameOfFileStore);
	fsSession.Parse(KFullNameOfFileStore, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(fsSession, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	for (int i=0;i<timerArray.Count();i++) {
		RDebug::Print(_L("Storing account %i"), i);
		outstream  << timerArray[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	fsSession.Close();
	
	CleanupStack::Pop(store);
	}

void CKitchenSyncView::LoadSettings() 
	{
	RFs fsSession;
	fsSession.Connect(); 
	TFileName path;
	TParse	filestorename;
	if (!BaflUtils::FileExists(fsSession, KFullNameOfFileStore)) {
		RDebug::Print(_L("No config file"));	
		return;
	}
	fsSession.Parse(KFullNameOfFileStore,filestorename);
	
	
	CFileStore* store = CDirectFileStore::OpenLC(fsSession,filestorename.FullName(),EFileRead);

	RStoreReadStream instream;
	instream.OpenLC(*store,store->Root());

	CKitchenSyncData readData;
	int error = 0;
	
	/*
	 * read all stored profiles and copy data to matching profiles in the system
	 */
	TRAP(error, instream  >> readData);	
	while (error==0) {
		// iterate over system profiles
		for (int i=0;i<timerArray.Count();i++) {
			CKitchenSyncData &data = timerArray[i];
			if (data.profileId == readData.profileId) {
				RDebug::Print(_L("Found profile=%d"), data.profileId);
				data.period = readData.period;
				StartTimer(i, data.period);
			}
		}
		TRAP(error, instream  >> readData);
	}
	
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);
	}

void CKitchenSyncView::HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType) {
	if(aEventType == EEventStateChanged)
	{
		int syncProfile = aControl->UniqueHandle() - EMyViewChoiceListBase;
		
		CEikChoiceList* chlst = LocateControlByUniqueHandle<CEikChoiceList>(aControl->UniqueHandle());
		int item = chlst->CurrentItem();
		RDebug::Print(_L("HandleControlEvent for profile=%d, item=%d"), syncProfile, item);
		TBuf<255> Buffer;
		StartTimer((int)syncProfile,item);
		SaveSettings();
	}
}

void CKitchenSyncView::StartTimer(int profile, int period) {
	RDebug::Print(_L("Starting timer for profile=%d, period=%d"), profile, period);
	StopTimer(profile);

    CKitchenSyncTimer *sTimer = new CKitchenSyncTimer(profile);
	sTimer->ConstructL();
	sTimer->SetPeriod(period);
    sTimer->RunPeriodically();
	timerArray[profile].timer=sTimer;
	timerArray[profile].period = period;
}

void CKitchenSyncView::StopTimer(int profile) {
	RDebug::Print(_L("Stopping timer for profile=%d"), profile);
	CKitchenSyncTimer* timer = timerArray[profile].timer;
	if (timer != NULL) {
		timer->Cancel();
		delete timer;
		timerArray[profile].timer = NULL;
	} else {
		RDebug::Print(_L("No timer running"));
	}
}
