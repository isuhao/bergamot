// KitchenSyncAppUi.h

#ifndef __KitchenSyncAPPUI_H__
#define __KitchenSyncAPPUI_H__

#include <QikAppUi.h>
#include "KitchenSyncView.h"


class CKitchenSyncAppUi : public CQikAppUi
{
public:
  void ConstructL();
  ~CKitchenSyncAppUi();

private:
  void HandleCommandL(CQikCommand &aCommand);

  CKitchenSyncView* iBaseView;
};


#endif


