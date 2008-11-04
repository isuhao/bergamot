#include "SyncSetting.h"
#include "debug.h"

CSyncSetting::CSyncSetting(TInt aResourceId, TInt aData, RSyncServerSession &aSession) :
	CAknEnumeratedTextPopupSettingItem(aResourceId, iStatusIndex), iSession(aSession),
	iStatusIndex(0)
	{
	}

CSyncSetting::~CSyncSetting()
	{
	}

void CSyncSetting::CompleteConstructionL()
	{
	// Complete construction
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

	// Set internal value.
	LoadL();
	}

void CSyncSetting::EditItemL(TBool aCalledFromMenu)
	{
	DP("CSyncSetting::EditItemL BEGIN");
	CAknEnumeratedTextPopupSettingItem::EditItemL(aCalledFromMenu);
	StoreL();
	DP("CSyncSetting::EditItemL END");
	}

void CSyncSetting::HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType)
	{
	CAknSettingItem::HandleSettingPageEventL(aSettingPage, aEventType);
	if (aEventType == EEventSettingOked) 
		{
		}
	}

TInt CSyncSetting::HandleSetting()
	{
	return (iStatusIndex == 0);
	}
