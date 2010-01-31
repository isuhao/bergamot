/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#include "PodcastListView.h"
#include "Podcast.hrh"
#include "PodcastAppUi.h"
#include <podcast.rsg>
#include <aknlists.h> 
#include <aknviewappui.h>
#include <aknnotedialog.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <akntabgrp.h>
#include <akntoolbarextension.h>
#include<BARSREAD.H>


const TInt KDefaultGran = 5;
CPodcastListContainer::CPodcastListContainer()
{
}

void CPodcastListContainer::SetKeyEventListener(MKeyEventListener *aKeyEventListener)
	{
	iKeyEventListener = aKeyEventListener;
	}

TKeyResponse CPodcastListContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	TKeyResponse response = iListbox->OfferKeyEventL(aKeyEvent, aType);
	if (iKeyEventListener)
		iKeyEventListener->OfferKeyEventL(aKeyEvent, aType);
	return response;
}

void CPodcastListContainer::ConstructL( const TRect& aRect, TInt aListboxFlags )
{
	CreateWindowL();

	iBgContext = 
		    CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, 
		                                              aRect, 
		                                              ETrue );
		
	 // Set the windows size
    SetRect( aRect );    
    iListbox =static_cast<CEikFormattedCellListBox*>( new (ELeave) CAknDoubleLargeStyleListBox);
    iListbox->ConstructL(this, aListboxFlags);
    iListbox->SetMopParent( this );
	iListbox->SetContainerWindowL(*this);
	iListbox->CreateScrollBarFrameL(ETrue);
	iListbox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EAuto, CEikScrollBarFrame::EAuto );
	
	iListbox->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );

	iListbox->SetSize(aRect.Size());
	iListbox->MakeVisible(ETrue);
    MakeVisible(EFalse);

	// Activate the window, which makes it ready to be drawn
    ActivateL();   
}

TInt CPodcastListContainer::CountComponentControls() const
    {
    return 1; // return number of controls inside this container
    }

CCoeControl* CPodcastListContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        case 0:
            return iListbox;
        default:
            return NULL;
        }
    }

void CPodcastListContainer::HandleResourceChange(TInt aType)
{
	switch( aType )
    	{
	    case KEikDynamicLayoutVariantSwitch:
		    SetRect(iEikonEnv->EikAppUi()->ClientRect());
		    break;
	    }
}

void CPodcastListContainer::ScrollToVisible() {
	if (iListbox != NULL) {
		iListbox->ScrollToMakeItemVisible(iListbox->CurrentItemIndex());
	}
}
void CPodcastListContainer::SizeChanged()
{
	DP2("CPodcastListContainer::SizeChanged(), width=%d, height=%d",Size().iWidth, Size().iHeight);
	if(iListbox != NULL)
	{
		iListbox->SetSize(Size());
	}
}

CEikFormattedCellListBox* CPodcastListContainer::Listbox()
{
	return iListbox;
}


CPodcastListContainer::~CPodcastListContainer()
{
	delete iListbox;
	delete iBgContext;
}


void CPodcastListContainer::Draw(const TRect& aRect) const
	{
	CWindowGc& gc = SystemGc();
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	AknsDrawUtils::Background(skin, cc, (CCoeControl*) this, gc, aRect);
	}


TTypeUid::Ptr CPodcastListContainer::MopSupplyObject( TTypeUid aId )
    {
    if (iBgContext )
    {
      return MAknsControlContext::SupplyMopObject( aId, iBgContext );
    }
    return CCoeControl::MopSupplyObject(aId);
    }

void CPodcastListContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
	if (iPointerListener)
		iPointerListener->PointerEventL(aPointerEvent);

	// Call base class HandlePointerEventL()
	CCoeControl::HandlePointerEventL(aPointerEvent);
	}


void CPodcastListContainer::SetPointerListener(MPointerListener *aPointerListener)
	{
	iPointerListener = aPointerListener;
	}


CPodcastListView::CPodcastListView()
{
}

void CPodcastListView::ConstructL()
{
	iListContainer = new (ELeave) CPodcastListContainer;
	iListContainer->ConstructL(ClientRect(), iListboxFlags);
	iListContainer->SetMopParent(this);
	iListContainer->ActivateL();
	iItemArray = new (ELeave)CDesCArrayFlat(KDefaultGran);
	iListContainer->Listbox()->Model()->SetItemTextArray(iItemArray);
	iListContainer->Listbox()->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);

	if (Toolbar()) {
		iToolbar = Toolbar();
		iToolbar->SetToolbarObserver(this);
	}
	
	iLongTapDetector = CAknLongTapDetector::NewL(this);
	iListContainer->SetPointerListener(this);
}

void CPodcastListView::HandleViewRectChange()
{
    if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
}

void CPodcastListView::HandleStatusPaneSizeChange()
{
	DP2("CPodcastListView::HandleStatusPaneSizeChange(), width=%d, height=%d", ClientRect().Width(), ClientRect().Height());

	if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
	
}

    
CPodcastListView::~CPodcastListView()
    {
    delete iListContainer;  
    delete iItemArray;
    iItemIdArray.Close();
    }

	
void CPodcastListView::DoActivateL(const TVwsViewId& /*aPrevViewId */,
	                                  TUid /*aCustomMessageId */,
									  const TDesC8& /* aCustomMessage */)
{	
	DP("CPodcastListView::DoActivateL()");
	
	if(iListContainer)
	{
		iListContainer->SetSize(ClientRect().Size());
		iListContainer->SetMopParent(this);
		
		AppUi()->AddToStackL(*this, iListContainer);
		iListContainer->MakeVisible(ETrue);
		UpdateListboxItemsL();		
		iListContainer->DrawNow();
	}
}

void CPodcastListView::DoDeactivate()
{
	DP("CPodcastListView::DoDeactivate()");
	if ( iListContainer )
	{
		AppUi()->RemoveFromViewStack( *this, iListContainer);
		iListContainer->MakeVisible(EFalse);
	}
}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastListView::HandleCommandL(TInt aCommand)
{
	DP1("CPodcastListView::HandleCommandL=%d", aCommand);
	switch(aCommand)
	{
	case EAknSoftkeyExit:
	case EEikCmdExit:
		{
            AppUi()->Exit();
            break;
		}
	case EAknSoftkeyBack:
		{
		AppUi()->ActivateViewL(iPreviousView);
		if (iPreviousView.iViewUid == KUidPodcastFeedViewID) {
			((CPodcastAppUi*)AppUi())->SetActiveTab(0);
		}
		}break;
	case EPodcastSettings:
		AppUi()->ActivateLocalViewL(KUidPodcastSettingsViewID);
		break;
	case EPodcastAbout:
		RunAboutDialogL();
		break;
	default:
		AppUi()->HandleCommandL(aCommand);
	}
}


void CPodcastListView::RunAboutDialogL()
{
	CAknNoteDialog* dlg = new(ELeave) CAknNoteDialog();
	CleanupStack::PushL(dlg);
	
	/*TBuf<255> aboutMessage;
	iEikonEnv->ReadResourceL(aboutMessage, STRING_R_ABOUT_TEXT_S60);
	dlg->SetTextL(aboutMessage);*/
	
	CleanupStack::Pop(dlg);
	dlg->ExecuteLD(R_DLG_ABOUT);
}

void CPodcastListView::SetEmptyTextL(TInt aResourceId)
	{
	HBufC* emptyText =  iEikonEnv->AllocReadResourceLC(aResourceId);
	iListContainer->Listbox()->View()->SetListEmptyTextL(*emptyText);
	CleanupStack::PopAndDestroy(emptyText);	
	}

void CPodcastListView::ShowOkMessage(TDesC &aText)
	{
	CAknNoteDialog* dlg= new(ELeave) CAknNoteDialog();
	CleanupStack::PushL(dlg);
	dlg->SetTextL(aText);
	CleanupStack::Pop(dlg);
	dlg->ExecuteLD(R_MESSAGEDLG_OK);
	}

void CPodcastListView::ShowErrorMessage(TDesC &aText)
	{
	CAknNoteDialog* dlg= new(ELeave) CAknNoteDialog();
	CleanupStack::PushL(dlg);
	dlg->SetTextL(aText);
	CleanupStack::Pop(dlg);
	dlg->ExecuteLD(R_ERRORDLG_OK);
	}


void CPodcastListView::CloseToolbarExtension()
{
	CAknToolbar* toolbar = Toolbar();
	if (toolbar) {
		CAknToolbarExtension* toolbarExtension = toolbar->ToolbarExtension();
		if (toolbarExtension) {
		toolbarExtension->SetShown( EFalse );
		}
	}
}

void CPodcastListView::PointerEventL(const TPointerEvent& aPointerEvent)
	{
	DP("CPodcastListView::PointerEventL");
	// Pass the pointer event to Long tap detector component
	iLongTapDetector->PointerEventL(aPointerEvent);
	}


void CPodcastListView::HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& /* aPenEventScreenLocation */)
{
	DP("CPodcastListView::HandleLongTapEventL BEGIN");
    if(!iStylusPopupMenu)
    {
        iStylusPopupMenu = CAknStylusPopUpMenu::NewL( this , aPenEventLocation);
        TResourceReader reader;
        iCoeEnv->CreateResourceReaderLC(reader,R_FEEDVIEW_POPUP_MENU);
        iStylusPopupMenu->ConstructFromResourceL(reader);
        CleanupStack::PopAndDestroy();
    }
    iStylusPopupMenu->ShowMenu();
    iStylusPopupMenu->SetPosition(aPenEventLocation);
    iLongTapUnderway=ETrue; // this will disable listbox events
	DP("CPodcastListView::HandleLongTapEventL END");
}


void CPodcastListView::DynInitToolbarL (TInt /*aResourceId*/, CAknToolbar * /*aToolbar*/)
	{
	DP("CPodcastListView::DynInitToolbarL");
	UpdateToolbar();
	}


void CPodcastListView::OfferToolbarEventL(TInt aCommand)
	{
	HandleCommandL(aCommand);
	}

