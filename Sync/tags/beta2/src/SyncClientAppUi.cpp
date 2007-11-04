#include "SyncClientAppUi.h"
#include "SyncClientView.h"
#include <QikCommand.h>


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CSyncClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  CSyncClientView* baseView = CSyncClientView::NewLC(*this);
  AddViewL(*baseView);
  iBaseView = baseView;
  CleanupStack::Pop(baseView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CSyncClientAppUi::~CSyncClientAppUi()
{
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CSyncClientAppUi::HandleCommandL(CQikCommand &aCommand)
{
  TQikViewMode viewMode = iBaseView->ViewMode();

  switch(aCommand.Id())
  {
        case EMyAbout:
        {
                CEikonEnv::InfoWinL(_L("SyncClient"), _L("Version 1.0"));
        }
        return;
  }
  CQikAppUi::HandleCommandL(aCommand);
}

