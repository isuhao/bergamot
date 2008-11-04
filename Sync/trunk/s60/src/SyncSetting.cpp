#include "SyncSetting.h"
#include "debug.h"

CSyncSetting::CSyncSetting(TInt aResourceId, RSyncServerSession &aSession) :
	CAknEnumeratedTextPopupSettingItem(aResourceId, iValue), iSession(aSession),
	iValue(0)
	{
	}

CSyncSetting::~CSyncSetting()
	{
	}

void CSyncSetting::CompleteConstructionL()
	{
	// Complete construction
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();
	LoadL();
	}

void CSyncSetting::SetValue(TUint value) 
	{
	iValue = value;
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
	return ETrue; 
	}
