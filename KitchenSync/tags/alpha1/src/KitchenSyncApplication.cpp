// Application.cpp

#include "KitchenSyncView.h"
#include "KitchenSyncApplication.h"


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
CApaDocument* CKitchenSyncApplication::CreateDocumentL()
{
  return CKitchenSyncDocument::NewL(*this);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
TUid CKitchenSyncApplication::AppDllUid() const
{
  return KUidKitchenSyncID;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
EXPORT_C CApaApplication* CKitchenSyncApplication::NewApplication()
{
  return new CKitchenSyncApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
LOCAL_C CApaApplication* NewApplication()
{
  return new CKitchenSyncApplication;
}


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
GLDEF_C TInt E32Main()
{
  return EikStart::RunApplication(NewApplication);
}


