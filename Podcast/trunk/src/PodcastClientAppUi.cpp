#include "PodcastClientAppUi.h"
#include "PodcastClientBaseView.h"
#include "PodcastClientFeedView.h"
#include "PodcastClientShowsView.h"
#include "PodcastClientPlayView.h"
#include "PodcastModel.h"
#include <qikcommand.h>

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CPodcastClientAppUi::ConstructL()
{
  CQikAppUi::ConstructL();
  iPodcastModel = CPodcastModel::NewL();

  iBaseView= CPodcastClientBaseView::NewLC(*this, *iPodcastModel);
  AddViewL(*iBaseView);
  CleanupStack::Pop(iBaseView);
  
  iFeedView = CPodcastClientFeedView::NewLC(*this, *iPodcastModel);
  AddViewL(*iFeedView);
  CleanupStack::Pop(iFeedView);

  iShowsView = CPodcastClientShowsView::NewLC(*this, *iPodcastModel);
  AddViewL(*iShowsView);
  CleanupStack::Pop(iShowsView);

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
  CQikAppUi::HandleCommandL(aCommand);
}

