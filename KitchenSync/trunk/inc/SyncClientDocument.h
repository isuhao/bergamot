// KitchenSyncDocument.h

#ifndef __KitchenSyncDOCUMENT_H__
#define __KitchenSyncDOCUMENT_H__

#include <QikDocument.h>
#include <QikApplication.h>
#include <QikAppUi.h>
#include "KitchenSyncAppUi.h"


class CKitchenSyncDocument : public CQikDocument
{
public:
  static CKitchenSyncDocument* NewL(CQikApplication& aAppUi);
  ~CKitchenSyncDocument();
private:
  CQikAppUi* CreateAppUiL();
  CKitchenSyncDocument (CQikApplication& aApp);
  void ConstructL();
};

#endif

