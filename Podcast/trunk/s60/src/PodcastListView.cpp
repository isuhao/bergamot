/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastListView.h"
#include "Podcast.hrh"
#include "PodcastAppUi.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <aknlists.h> 
#include <aknviewappui.h>
#include <aknnotedialog.h> 

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
	if (iKeyEventListener)
		iKeyEventListener->OfferKeyEventL(aKeyEvent, aType);
	return iListbox->OfferKeyEventL(aKeyEvent, aType);
}

void CPodcastListContainer::ConstructL( const TRect& aRect, TInt aListboxFlags )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    iListbox =static_cast<CEikFormattedCellListBox*>( new (ELeave) CAknDoubleLargeStyleListBox);
	iListbox->SetMopParent( this );
	iListbox->SetContainerWindowL(*this);
	iListbox->ConstructL(this, aListboxFlags);
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
}

CPodcastListView::CPodcastListView()
{
}

void CPodcastListView::ConstructL()
{
	iListContainer = new (ELeave) CPodcastListContainer;
	iListContainer->SetMopParent( this );
	iListContainer->ConstructL(ClientRect(), iListboxFlags);
	iItemArray = new (ELeave)CDesCArrayFlat(KDefaultGran);
	iListContainer->Listbox()->Model()->SetItemTextArray(iItemArray);
	iListContainer->Listbox()->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
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
	if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
}

    
CPodcastListView::~CPodcastListView()
    {
    delete iListContainer;  
    delete iNaviDecorator;
    delete iItemArray;
    iItemIdArray.Close();
    }

	
void CPodcastListView::DoActivateL(const TVwsViewId& /*aPrevViewId */,
	                                  TUid /*aCustomMessageId */,
									  const TDesC8& /* aCustomMessage */)
{	
	if(iListContainer)
	{
		iListContainer->SetRect(ClientRect());
		AppUi()->AddToViewStackL( *this, iListContainer );	
		iListContainer->MakeVisible(ETrue);
		UpdateListboxItemsL();		
		iListContainer->DrawNow();
	}
	
	if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->PushL(*iNaviDecorator);
		}
}

void CPodcastListView::DoDeactivate()
{
	if ( iListContainer )
	{
        AppUi()->RemoveFromViewStack( *this, iListContainer );
		iListContainer->MakeVisible(EFalse);
		if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->Pop(iNaviDecorator);
		}
	}
}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastListView::HandleCommandL(TInt aCommand)
{
	RDebug::Printf("CPodcastListView::HandleCommandL=%d", aCommand);
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
		}break;
	case EPodcastSettings:
		AppUi()->ActivateLocalViewL(KUidPodcastSettingsViewID);
		break;
	case EPodcastZoomSetting:
		break;
	case EPodcastAbout:
		RunAboutDialogL();
		break;
	default:
		break;
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

void CPodcastListView::SetNaviTextL(TDesC &aText)
	{
	if (iNaviPane != NULL)
		{
		iNaviPane->Pop(iNaviDecorator);
		delete iNaviDecorator;
		iNaviDecorator = NULL;
		}

	iNaviDecorator	= iNaviPane->CreateNavigationLabelL(aText);
	iNaviPane->PushL(*iNaviDecorator);
	}

