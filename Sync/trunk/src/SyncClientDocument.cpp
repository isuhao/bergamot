// KitchenSyncDocument.cpp
#include "KitchenSyncDocument.h"


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CKitchenSyncDocument::CKitchenSyncDocument (CQikApplication& aApp) : CQikDocument(aApp) {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CKitchenSyncDocument* CKitchenSyncDocument::NewL(CQikApplication& aApp)
{
  CKitchenSyncDocument* self = new (ELeave) CKitchenSyncDocument(aApp);
  CleanupStack::PushL(self);
  self->ConstructL();
  CleanupStack::Pop(self);
  return self;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
void CKitchenSyncDocument::ConstructL() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CKitchenSyncDocument::~CKitchenSyncDocument() {}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CQikAppUi* CKitchenSyncDocument::CreateAppUiL()
{
  return new(ELeave) CKitchenSyncAppUi;
}
