#ifndef KITCHENSYNCVIEW_H
#define KITCHENSYNCVIEW_H

#include <QikViewBase.h>
#include <QikScrollableContainer.h>
#include "KitchenSync.hrh"
#include "KitchenSyncTimer.h"
#include "S32FILE.H"
#include "e32debug.h"
#include <e32cmn.h>
#include "RKitchenSyncServerSession.h"

#define KITCHENSYNCUID 0x2000E8A1
const TUid KUidKitchenSyncID = {KITCHENSYNCUID};
const TUid KUidListBoxListView = {0x00000001};

class CKitchenSyncView : public CQikViewBase
	{
public:
	static CKitchenSyncView* NewLC(CQikAppUi& aAppUi);
	~CKitchenSyncView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
	
protected: 
	void ViewConstructL();
	
private:
	CKitchenSyncView(CQikAppUi& aAppUi);
	void ConstructL();
	void CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16& caption, int state);
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);

public:
	void ShowSyncProfiles(CQikScrollableContainer* container);

private:
	RKitchenSyncServerSession serverSession;
	CQikScrollableContainer* iContainer;
	RArray<TSmlProfileId> profiles;
	};
#endif
