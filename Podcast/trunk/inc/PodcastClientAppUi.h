#ifndef __PODCASTCLIENTAPPUI_H__
#define __PODCASTCLIENTAPPUI_H__

#include <QikAppUi.h>
#include "PodcastClientView.h"


class CPodcastClientAppUi : public CQikAppUi
{
public:
  void ConstructL();
  ~CPodcastClientAppUi();

private:
  void HandleCommandL(CQikCommand &aCommand);

  CPodcastClientView* iBaseView;
};


#endif


