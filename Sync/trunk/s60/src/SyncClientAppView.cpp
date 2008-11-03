/*
* ============================================================================
*  Name     : CSyncClientAppView from SyncClientAppView.cpp
*  Part of  : Dynamic Setting List
*  Created  : 09/21/2005 by Forum Nokia
*  Version  : 1.0
*  Copyright: Nokia Corporation
* ============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <SyncClientS60.rsg>
#include "SyncClientS60.hrh"
#include "SyncClientAppView.h"
#include <eikfrlbd.h>
#include "debug.h"

// CONSTANTS
_LIT(KEmptyText, "None");
_LIT(KEnumTitle, "Enum");
_LIT(KEnumText1, "Enum text 1");
_LIT(KEnumText2, "Enum text 2");

// ================= MEMBER FUNCTIONS =======================

// Constructor
CSyncClientAppView::CSyncClientAppView() 
    {
    }

// Destructor
CSyncClientAppView::~CSyncClientAppView()
    {
    serverSession.Close();
	delete iItemList;
    }

// ----------------------------------------------------
// CSyncClientAppView::ConstructL()
// Symbian OS default constructor can leave.
// ----------------------------------------------------
//
void CSyncClientAppView::ConstructL(const TRect& aRect)
    {
    // Create a window for this application view
    CreateWindowL();
    
   	iItemList = new (ELeave) CAknSettingItemList;
   	iItemList->SetMopParent(this);
    iItemList->ConstructFromResourceL(R_ENTRY_SETTINGS_LIST); 
    
    LoadListL();
    
    iItemList->MakeVisible(ETrue);
    iItemList->SetRect(aRect);
    iItemList->ActivateL();
    iItemList->ListBox()->UpdateScrollBarsL();
    iItemList->DrawNow();    

    // Set the windows size
    SetRect(aRect);

    DP("Before serverSession.Connect");
    User::LeaveIfError(serverSession.Connect());
    DP("After serverSession.Connect");
    // Activate the window, which makes it ready to be drawn
    ActivateL();
    }

// ----------------------------------------------------
// CSyncClientAppView::Draw()
// This function is used for window server-initiated 
// redrawing of controls, and for some 
// application-initiated drawing.
// ----------------------------------------------------
//
void CSyncClientAppView::Draw(const TRect& /*aRect*/) const
    {
    // Get the standard graphics context 
    CWindowGc& gc = SystemGc();
    
    // Gets the control's extent
    TRect rect = Rect();
    
    // Clears the screen
    gc.Clear(rect);
    }

// ----------------------------------------------------
// CSyncClientAppView::CountComponentControls()
// Gets the number of controls contained in a compound 
// control. 
// ----------------------------------------------------
//
TInt CSyncClientAppView::CountComponentControls() const
	{
	TInt count = 0;
	if (iItemList)
		count++;
	return count;
	}
	
// ----------------------------------------------------
// CSyncClientAppView::ComponentControl()
// Gets the specified component of a compound control.
// ----------------------------------------------------
//
CCoeControl* CSyncClientAppView::ComponentControl(TInt /*aIndex*/) const
	{
	return iItemList;
	}
	
// ----------------------------------------------------
// CSyncClientAppView::OfferKeyEventL()
// When a key event occurs, the control framework calls 
// this function for each control on the control stack, 
// until one of them can process the key event 
// (and returns EKeyWasConsumed).
// ----------------------------------------------------
//	
TKeyResponse CSyncClientAppView::OfferKeyEventL(	const TKeyEvent& aKeyEvent, 
																TEventCode aType )
	{
    if(aType != EEventKey)
        {
        return EKeyWasNotConsumed;
        }    
    else if(iItemList)
        {
        return iItemList->OfferKeyEventL( aKeyEvent, aType );
        }
    else
        {
        return EKeyWasNotConsumed;
        }
	
	}

void CSyncClientAppView::CreateChoiceListItem(int id, const TPtrC16 &caption, int state ) 
	{
	DP("CreateChoiceListItem START");
	_LIT(KChoiceListText1, "Manually");
	_LIT(KChoiceListText2, "Every 15 minutes");
	_LIT(KChoiceListText3, "Every hour");
	_LIT(KChoiceListText4, "Every 4 hours");
	_LIT(KChoiceListText5, "Every 12 hours");
	_LIT(KChoiceListText6, "Daily");
	_LIT(KChoiceListText7, "Weekly");
	
	/*CEikChoiceList* chlst = new (ELeave) CEikChoiceList();
	block->AddControlLC(chlst, EQikItemSlot2);
	
	chlst->SetObserver(this);
	CDesCArrayFlat* array = new(ELeave) CDesCArrayFlat(7);
	CleanupStack::PushL(array);
	array->AppendL(KChoiceListText1);
	array->AppendL(KChoiceListText2);
	array->AppendL(KChoiceListText3);
	array->AppendL(KChoiceListText4);
	array->AppendL(KChoiceListText5);
	array->AppendL(KChoiceListText6);
	array->AppendL(KChoiceListText7);
	chlst->SetArrayL(array);
	CleanupStack::Pop(array);
	DP1("Setting handle to: %d", EMyViewChoiceListBase+id);
	chlst->SetUniqueHandle(EMyViewChoiceListBase+id);

	chlst->SetCurrentItem(state);
	chlst->SetObserver(this);
	CleanupStack::Pop(chlst);
	CleanupStack::Pop(block);
	*/

	TBool isNumberedStyle = iItemList->IsNumberedStyle();
	CArrayPtr<CGulIcon>* icons = iItemList->ListBox()->ItemDrawer()->FormattedCellData()->IconArray();

	/*Enumerated text setting item*/
	CAknEnumeratedTextPopupSettingItem* item = new (ELeave) CAknEnumeratedTextPopupSettingItem(9, iEnumText);
	CleanupStack::PushL(item);
	// The same resource id can be used for multiple enumerated text setting pages.
	item->ConstructL(isNumberedStyle, 9, caption, icons, R_ENUMERATEDTEXT_SETTING_PAGE, -1, 0, R_POPUP_SETTING_TEXTS);
	
	// Load texts dynamically.
	CArrayPtr<CAknEnumeratedText>* texts = item->EnumeratedTextArray();
	texts->ResetAndDestroy();
	CAknEnumeratedText* enumText;
	// Text 1
	HBufC* text = KChoiceListText1().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(0, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	// Text 2
	text = KChoiceListText2().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(1, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);

	text = KChoiceListText3().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(2, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	
	text = KChoiceListText4().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(3, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	
	text = KChoiceListText5().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(4, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	
	text = KChoiceListText6().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(5, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	
	text = KChoiceListText7().AllocLC();
	enumText = new (ELeave) CAknEnumeratedText(6, text);
	CleanupStack::Pop(text);
	CleanupStack::PushL(enumText);
	texts->AppendL(enumText);
	CleanupStack::Pop(enumText);
	
	iItemList->SettingItemArray()->AppendL(item);
	CleanupStack::Pop(item);

	DP("CreateChoiceListItem END");
}


// ----------------------------------------------------
// CSyncClientAppView::LoadListL()
// Loads the setting item list dynamically.
// ----------------------------------------------------
//
void CSyncClientAppView::LoadListL()
	{

	CreateChoiceListItem(0,_L("Wongo"), 0);
	
	// Required when there is only one setting item.
	iItemList->SettingItemArray()->RecalculateVisibleIndicesL();

	iItemList->HandleChangeInItemArrayOrVisibilityL();
	}

// ----------------------------------------------------
// CSyncClientAppView::StoreSettingsL()
// Stores the settings of the setting list.
// ----------------------------------------------------
//	
void CSyncClientAppView::StoreSettingsL()
	{
	iItemList->StoreSettingsL();
	}
		
// End of file