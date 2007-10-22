#ifndef KITCHENSYNCVIEW_H
#define KITCHENSYNCVIEW_H

#include <QikViewBase.h>
#include <QikScrollableContainer.h>
#include "KitchenSync.hrh"
#include "KitchenSyncTimer.h"
#include "S32FILE.H"
#include "e32debug.h"
#include <e32cmn.h>

_LIT(KFullNameOfFileStore,"c:\\private\\2000E8A1\\config.dat");
_LIT(KDirNameOfFileStore,"c:\\private\\2000E8A1");
#define KITCHENSYNCUID 0x2000E8A1
const TUid KUidKitchenSyncID = {KITCHENSYNCUID};

const TUid KUidListBoxListView = {0x00000001};

class CKitchenSyncData {
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TInt profileId;
	TInt period;
	CKitchenSyncTimer* timer;
};

class CKitchenSyncView : public CQikViewBase
	{
public:
	static CKitchenSyncView* NewLC(CQikAppUi& aAppUi);
	~CKitchenSyncView();
	
	// from CQikViewBase
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
	
protected: 
	// from CQikViewBase
	void ViewConstructL();
	
private:
	CKitchenSyncView(CQikAppUi& aAppUi);
	void ConstructL();
	void CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16& caption, int state);
	void ShowAllProfiles(CQikScrollableContainer* container);
	void LoadAllProfiles();
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	
	void StartTimer(int profileId, int period);
	void StopTimer(int profileId);
	void LoadSettings();
	void SaveSettings();
	RArray<CKitchenSyncData> timerArray;
	CQikScrollableContainer* iContainer;
	};
#endif
