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

CHttpClient::~CHttpClient()
  {
  iSession.Close();
  }

CHttpClient* CHttpClient::NewL(MHttpEventHandlerCallbacks& aCallbacks)
  {
  CHttpClient* me = NewLC(aCallbacks);
  CleanupStack::Pop(me);
  return me;
  }

CHttpClient::CHttpClient(MHttpEventHandlerCallbacks& aCallbacks) : iCallbacks(aCallbacks)
  {
  }

CHttpClient* CHttpClient::NewLC(MHttpEventHandlerCallbacks& aCallbacks)
  {
  CHttpClient* me = new (ELeave) CHttpClient(aCallbacks);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpClient::ConstructL()
  {
  iSession.OpenL();
  }

void CHttpClient::SetHeaderL(RHTTPHeaders aHeaders,
                             TInt aHdrField,
                             const TDesC8& aHdrValue)
  {
  RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
  THTTPHdrVal val(valStr);
  aHeaders.SetFieldL(
		  iSession.StringPool().StringF(aHdrField,
				  RHTTPSession::GetTable()),
                  val);
  valStr.Close();
  }

TBool CHttpClient::IsActive()
	{
	return iIsActive;
	}

void CHttpClient::GetFeed(TFeedInfo *info)
	{
	RDebug::Print(_L("CHttpClient::GetFeed START"));
	Get(info->iUrl, info->iFileName);
	iCallbacks.FeedCompleteCallback(info);
	RDebug::Print(_L("CHttpClient::GetFeed END"));
	}

void CHttpClient::GetShow(TShowInfo *info)
	{
	RDebug::Print(_L("CHttpClient::GetShow START"));
	Get(info->iUrl, info->iFileName);
	iCallbacks.ShowCompleteCallback(info);
	RDebug::Print(_L("CHttpClient::GetShow END"));
	}

void CHttpClient::Get(TDesC& url, TDesC& fileName) {
	iIsActive = ETrue;
	TBuf8<256> url8;
	url8.Copy(url);
	RStringPool strP = iSession.StringPool();
	RStringF method;
	method = strP.StringF(HTTP::EGET, RHTTPSession::GetTable());
	
	TUriParser8 uri; 
	uri.Parse(url8);
	CHttpEventHandler* eHandler;
	RDebug::Print(_L("Getting %S to %S"), &url, &fileName);
	eHandler = CHttpEventHandler::NewL(iCallbacks);
	eHandler->SetSaveFileName(fileName);
	RHTTPTransaction trans = iSession.OpenTransactionL(uri, *eHandler, method);
	  
	RHTTPHeaders hdr = trans.Request().GetHeaderCollection();
	// Add headers appropriate to all methods
	SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
	SetHeaderL(hdr, HTTP::EAccept, KAccept);
	
	// submit the transaction
	trans.SubmitL();
	// Start the scheduler, once the transaction completes or is cancelled on 
	// an error the scheduler will be stopped in the event handler
	CActiveScheduler::Start();
	iIsActive = EFalse;
}
