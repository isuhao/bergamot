#ifndef _MHTTPEVENTHANDLERCALLBACKS_H_
#define _MHTTPEVENTHANDLERCALLBACKS_H_

#include <e32cmn.h>
#include "ShowInfo.h"
#include "FeedInfo.h"

class CHttpClient;

class MHttpEventHandlerCallbacks
{
public:
  virtual void ConnectedCallback(CHttpClient* aHttpClient) = 0;
  virtual void DisconnectedCallback(CHttpClient* aHttpClient) = 0;
  virtual void ShowCompleteCallback(TShowInfo *info) = 0;
  virtual void FeedCompleteCallback(TFeedInfo *info) = 0;
  
  virtual void ProgressCallback(CHttpClient* aHttpClient, int aBytes, int aTotalBytes) = 0;
  virtual void DownloadInfoCallback(CHttpClient* aHttpClient, int aTotalBytes) = 0;
};

#endif
