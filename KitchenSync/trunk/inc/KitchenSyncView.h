#ifndef KITCHENSYNCVIEW_H
#define KITCHENSYNCVIEW_H

#include <QikViewBase.h>
#include <QikScrollableContainer.h>
#include "KitchenSync.hrh"
#include "KitchenSyncTimer.h"

#define KITCHENSYNCUID 0x2000E8A1
const TUid KUidKitchenSyncID = {KITCHENSYNCUID};

const TUid KUidListBoxListView = {0x00000001};

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
	void LoadAllProfiles(CQikScrollableContainer* container);
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	
	void CKitchenSyncView::StartaTimer(int profileId, int timeout);
	void CKitchenSyncView::StoppaTimer(int profileId);
	
	CKitchenSyncTimer* timerArray[100];
	};
#endif
