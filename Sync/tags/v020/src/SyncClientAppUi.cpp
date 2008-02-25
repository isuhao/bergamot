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
                CEikonEnv::InfoWinL(_L("About Swim"), _L("Swim 0.2\n\n(c) 2008, The Bergamot Project\nhttp://bergamot.googlecode.com"));
        }
        return;
  }
  CQikAppUi::HandleCommandL(aCommand);
}

