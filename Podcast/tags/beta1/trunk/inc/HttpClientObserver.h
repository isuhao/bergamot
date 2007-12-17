#ifndef _MHTTPCLIENTOBSERVER_H_
#define _MHTTPCLIENTOBSERVER_H_

#include <e32cmn.h>

class CHttpClient;

class MHttpClientObserver
{
public:
  virtual void Connected(CHttpClient* aHttpClient) = 0;
  virtual void Disconnected(CHttpClient* aHttpClient) = 0;
  virtual void Complete(CHttpClient* aHttpClient, TBool aSuccessful) = 0;
  virtual void Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes) = 0;
  virtual void DownloadInfo(CHttpClient* aHttpClient, int aTotalBytes) = 0;
};

#endif
