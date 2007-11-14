#include "PodcastClientAppUi.h"
#include "PodcastClientView.h"
#include "PodcastClientPlayView.h"
#include "PodcastModel.h"
#include <qikcommand.h>

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  iPodcastModel = CPodcastModel::NewL();
  iBaseView= CPodcastClientView::NewLC(*this, *iPodcastModel);
  AddViewL(*iBaseView);
  CleanupStack::Pop(iBaseView);

  iPlayView = CPodcastClientPlayView::NewLC(*this, *iPodcastModel);
  AddViewL(*iPlayView);
  CleanupStack::Pop(iPlayView);

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

