#ifndef __PODCASTCLIENTAPPLICATION_H__
#define __PODCASTCLIENTAPPLICATION_H__

#include <QikApplication.h>
#include <EikStart.h>
#include "PodcastClientDocument.h"

class CPodcastClientApplication : public CQikApplication
{
public:
  CApaDocument* CreateDocumentL();
  TUid AppDllUid() const;
  IMPORT_C CApaApplication* NewApplication();
};


#endif

