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
#include <commdb.h>
#include <eikenv.h>
#include <es_sock.h>
#include <bautils.h>
#include <CommDbConnPref.h>

CHttpClient::~CHttpClient()
  {
  iSession.Close();
  }

CHttpClient* CHttpClient::NewL(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver)
  {
  CHttpClient* me = NewLC(aPodcastModel, aObserver);
  CleanupStack::Pop(me);
  return me;
  }

CHttpClient::CHttpClient(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver) : iPodcastModel(aPodcastModel), iObserver(aObserver)
  {
  iResumeEnabled = EFalse;
  }

CHttpClient* CHttpClient::NewLC(CPodcastModel& aPodcastModel, MHttpClientObserver& aObserver)
  {
  CHttpClient* me = new (ELeave) CHttpClient(aPodcastModel, aObserver);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpClient::ConstructL()
  {

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

void CHttpClient::SetResumeEnabled(TBool aEnabled)
	{
	iResumeEnabled = aEnabled;
	}

void CHttpClient::GetL(TDesC& url, TDesC& fileName,  TBool aSilent) {
	RDebug::Print(_L("CHttpClient::Get START"));
	iIsActive = ETrue;
		
	TBuf8<256> url8;
	url8.Copy(url);
	
	if (iTransactionCount == 0) {
		RDebug::Print(_L("** Opening session"));
		iSession.OpenL();
		iPodcastModel.ConnectHttpSessionL(iSession);
	}
		
	TUriParser8 uri; 
	uri.Parse(url8);
	RDebug::Print(_L("Getting '%S' to '%S'"), &url, &fileName);

	iHandler = CHttpEventHandler::NewL(this, iObserver);
	iHandler->SetSilent(aSilent);

	RFs rfs;
	rfs.Connect();
	TEntry entry;
	TBuf8<100> rangeText;
	if (iResumeEnabled && rfs.Entry(fileName, entry) == KErrNone) {
		RDebug::Print(_L("Found file, with size=%d"), entry.iSize);
		// file exists, so we should probably resume
		rangeText.Format(_L8("bytes=%d-"), entry.iSize-KByteOverlap);
		iHandler->SetSaveFileName(fileName, ETrue);
	} else {
		// otherwise just make sure the directory exists
		BaflUtils::EnsurePathExistsL(rfs,fileName);
		iHandler->SetSaveFileName(fileName);
	}
	rfs.Close();
	
	RStringPool strP = iSession.StringPool();
	RStringF method;
	method = strP.StringF(HTTP::EGET, RHTTPSession::GetTable());

	iTrans = iSession.OpenTransactionL(uri, *iHandler, method);
	RHTTPHeaders hdr = iTrans.Request().GetHeaderCollection();
	// Add headers appropriate to all methods
	SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
	SetHeaderL(hdr, HTTP::EAccept, KAccept);
	TBuf<100> range16;
	range16.Copy(rangeText);
	RDebug::Print(_L("range text: %S"), &range16);
	SetHeaderL(hdr, HTTP::ERange, rangeText);
	iTransactionCount++;
	// submit the transaction
	iTrans.SubmitL();
	RDebug::Print(_L("CHttpClient::Get END"));
}

void CHttpClient::Stop()
	{
	iIsActive = EFalse;
	if(iHandler != NULL)
	{
		iHandler->CloseSaveFile();
		iTrans.Cancel();
	}
	//iSession.Close();
	}

void CHttpClient::ClientRequestCompleteL(TBool aSuccessful) {
	iIsActive = EFalse;
	iObserver.Complete(this, aSuccessful);
	RDebug::Print(_L("CHttpClient::Get END"));
	iTransactionCount--;
	
	if(iTransactionCount == 0) {
		RDebug::Print(_L("** Closing session"));
		iSession.Close();
	}
}

