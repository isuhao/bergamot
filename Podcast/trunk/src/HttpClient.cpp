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

// HttpClient.cpp


#include <e32base.h>
#include "HttpClient.h"
#include <http/rhttpheaders.h>
#include <http.h>
#include <eikenv.h>


// Standard headers used by default
_LIT8(KUserAgent, "HTTPExampleClient (1.0)");
_LIT8(KAccept, "*/*");

CHttpClient::~CHttpClient()
  {
  iSession.Close();
  if(iTransObs)
    delete iTransObs;
  }

void CHttpClient::StartClientL()
  {
  TBuf8<256> url8;
  url8.Copy(iUrl);
  RStringPool strP = iSession.StringPool();
  RStringF method;
  method = strP.StringF(HTTP::EGET, RHTTPSession::GetTable());
  InvokeHttpMethodL(url8, method);
  }

void CHttpClient::StopClient()
	{
	if (iTrans != NULL) {
		iTrans->Cancel();
	}
	}
CHttpClient* CHttpClient::NewL(MHttpEventHandlerCallbacks& aResObs)
  {
  CHttpClient* me = NewLC(aResObs);
  CleanupStack::Pop(me);
  return me;
  }

CHttpClient::CHttpClient(MHttpEventHandlerCallbacks& aResObs) : iResObs(aResObs)
  {
  iTrans = NULL;
  }

CHttpClient* CHttpClient::NewLC(MHttpEventHandlerCallbacks& aResObs)
  {
  CHttpClient* me = new (ELeave) CHttpClient(aResObs);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpClient::ConstructL()
  {
  iSession.OpenL();
  iTransObs = CHttpEventHandler::NewL(iResObs);
  }

void CHttpClient::InvokeHttpMethodL(const TDesC8& aUri, RStringF aMethod)
  {
  RDebug::Print(_L("InvokeHttpMethodL"));
  TUriParser8 uri; 
  uri.Parse(aUri);
  RHTTPTransaction trans = iSession.OpenTransactionL(uri, *iTransObs, aMethod);
  RHTTPHeaders hdr = trans.Request().GetHeaderCollection();
  // Add headers appropriate to all methods
  SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
  SetHeaderL(hdr, HTTP::EAccept, KAccept);

  // submit the transaction
  trans.SubmitL();
  // Start the scheduler, once the transaction completes or is cancelled on 
  // an error the scheduler will be stopped in the event handler
  CActiveScheduler::Start();
  }

void CHttpClient::SetUrl(TDesC &url)
	{
	RDebug::Print(_L("SetUrl: %S"), &url);
	iUrl.Copy(url);
	}

void CHttpClient::SetSaveFileName(TDesC &fName)
	{
	RDebug::Print(_L("SetSaveFileName: %S"), &fName);
	iTransObs->SetSaveFileName(fName);
	}

void CHttpClient::SetHeaderL(RHTTPHeaders aHeaders,
                             TInt aHdrField,
                             const TDesC8& aHdrValue)
  {
  RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
  THTTPHdrVal val(valStr);
  aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField,
                                                   RHTTPSession::GetTable()),
                     val);
  valStr.Close();
  }


void CHttpClient::GetFeed(TFeedInfo *info)
	{
	iTransObs->SetSaveFileName(info->fileName);
	iTransObs->SetFeedInfo(info);
	iUrl.Copy(info->url);
	
	StartClientL();	
	}

void CHttpClient::GetShow(TShowInfo *info)
	{
	iTransObs->SetSaveFileName(info->fileName);
	iTransObs->SetShowInfo(info);
	iUrl.Copy(info->url);
	
	StartClientL();
	
	}
