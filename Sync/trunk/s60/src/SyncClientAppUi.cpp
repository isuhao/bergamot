// INCLUDE FILES
#include <avkon.hrh>
#include "SyncClientAppUi.h"
#include "SyncClientAppView.h"
#include "SyncClientS60.hrh"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CSyncClientAppUi::ConstructL()
// ConstructL is called by the application framework
// ----------------------------------------------------------
//
void CSyncClientAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);

    iAppView = new (ELeave) CSyncClientAppView;
    iAppView->SetMopParent(this);
    iAppView->ConstructL(ClientRect());

    AddToStackL(iAppView);
    }

// Constructor
CSyncClientAppUi::CSyncClientAppUi()                              
    {
	// no implementation required
    }

// Destructor
CSyncClientAppUi::~CSyncClientAppUi()
    {
    if (iAppView)
        {
        RemoveFromStack(iAppView);
        delete iAppView;
        iAppView = NULL;
        }
    }

// ----------------------------------------------------
// CSyncClientAppUi::HandleCommandL()
// takes care of command handling
// ----------------------------------------------------
//
void CSyncClientAppUi::HandleCommandL(TInt aCommand)
    {
    switch(aCommand)
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;
      default:
            break;
        }
    }

// End of file