// KitchenSyncApplication.h
#ifndef __KitchenSyncAPPLICATION_H__
#define __KitchenSyncAPPLICATION_H__

#include <QikApplication.h>
#include <EikStart.h>
#include "KitchenSyncDocument.h"

class CKitchenSyncApplication : public CQikApplication
{
public:
  CApaDocument* CreateDocumentL();
  TUid AppDllUid() const;
  IMPORT_C CApaApplication* NewApplication();
};


#endif

