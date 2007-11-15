#include "PodcastClientAppUi.h"
#include "PodcastClientBaseView.h"
#include "PodcastClientFeedView.h"
#include "PodcastClientPodcastsView.h"
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

  iPodcastsView = CPodcastClientPodcastsView::NewLC(*this, *iPodcastModel);
  AddViewL(*iPodcastsView);
  CleanupStack::Pop(iPodcastsView);

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

