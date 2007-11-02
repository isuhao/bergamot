#include "PodcastClientAppUi.h"
#include "PodcastClientView.h"
#include <QikCommand.h>

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  CPodcastClientView* baseView = CPodcastClientView::NewLC(*this);
  AddViewL(*baseView);
  iBaseView = baseView;
  CleanupStack::Pop(baseView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientAppUi::~CPodcastClientAppUi()
{
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::HandleCommandL(CQikCommand &aCommand)
{
  TQikViewMode viewMode = iBaseView->ViewMode();

  switch(aCommand.Id())
  {
        case EMyAbout:
        {
                CEikonEnv::InfoWinL(_L("PodcastClient"), _L("Version 1.0"));
        }
        return;
  }
  CQikAppUi::HandleCommandL(aCommand);
}

