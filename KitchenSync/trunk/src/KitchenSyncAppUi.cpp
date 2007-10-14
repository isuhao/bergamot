// KitchenSyncAppUi.cpp

#include "KitchenSyncAppUi.h"
#include "KitchenSyncView.h"
#include <QikCommand.h>


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CKitchenSyncAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  CKitchenSyncView* baseView = CKitchenSyncView::NewLC(*this);
  AddViewL(*baseView);
  iBaseView = baseView;
  CleanupStack::Pop(baseView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CKitchenSyncAppUi::~CKitchenSyncAppUi()
{
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CKitchenSyncAppUi::HandleCommandL(CQikCommand &aCommand)
{
  TQikViewMode viewMode = iBaseView->ViewMode();

  switch(aCommand.Id())
  {
        case EMyAbout:
        {
                CEikonEnv::InfoWinL(_L("KitchenSync"), _L("Version 1.0"));
        }
        return;
  }
  CQikAppUi::HandleCommandL(aCommand);
}

