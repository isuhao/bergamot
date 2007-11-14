#include "PodcastClientAppUi.h"
#include "PodcastClientView.h"
#include "PodcastModel.h"
#include <qikcommand.h>

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  iPodcastModel = CPodcastModel::NewL();
  CPodcastClientView* baseView = CPodcastClientView::NewLC(*this, *iPodcastModel);
  AddViewL(*baseView);
  iBaseView = baseView;
  CleanupStack::Pop(baseView);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CPodcastClientAppUi::~CPodcastClientAppUi()
{
	delete iPodcastModel;
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

