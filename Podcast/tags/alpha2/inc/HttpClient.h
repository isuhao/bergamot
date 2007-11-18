/***************************************************************************** 
COPYRIGHT All rights reserved Sony Ericsson Mobile Communications AB 2006. 
The software is the copyrighted work of Sony Ericsson Mobile Communications AB. 
The use of the software is subject to the terms of use or of the end-user license 
agreement which accompanies or is included with the software. The software is 
provided "as is" and Sony Ericsson specifically disclaim any warranty or 
condition whatsoever regarding merchantability or fitness for a specific 
purpose, title or non-infringement. No warranty of any kind is made in 
relation to the condition, suitability, availability, accuracy, reliability, 
merchantability and/or non-infringement of the software provided herein. 
*****************************************************************************/

// HttpClient.h

#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpEventHandlerCallbacks.h"
#include "HttpEventHandler.h"


// Standard headers used by default
_LIT8(KUserAgent, "HTTPExampleClient (1.0)");
_LIT8(KAccept, "*/*");

class CHttpClient : public CBase
{
public:
  virtual ~CHttpClient();
  static CHttpClient* NewL(MHttpEventHandlerCallbacks& aResObs);
  void GetFeed(TFeedInfo *info);
  void GetShow(TShowInfo *info);
  TBool IsActive();
protected:
  CHttpClient(MHttpEventHandlerCallbacks& aResObs);

private:
  static CHttpClient* NewLC(MHttpEventHandlerCallbacks& aResObs);
  void ConstructL();
  void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);
  void Get(TDesC& url, TDesC& fileName);
private:
  RHTTPSession iSession;
  MHttpEventHandlerCallbacks& iCallbacks;
  TBool iIsActive;
};


#endif
