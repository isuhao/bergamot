#ifndef __PODCASTCLIENTAPPUI_H__
#define __PODCASTCLIENTAPPUI_H__

#include <qikappui.h>
#include "PodcastClientView.h"
class CPodcastClientPlayView;
class CPodcastModel;
class CPodcastClientAppUi : public CQikAppUi
{
public:
  void ConstructL();
  ~CPodcastClientAppUi();

private:
  void HandleCommandL(CQikCommand &aCommand);
   CPodcastClientPlayView* iPlayView;
  CPodcastClientView* iBaseView;
  CPodcastModel* iPodcastModel;
};


#endif


