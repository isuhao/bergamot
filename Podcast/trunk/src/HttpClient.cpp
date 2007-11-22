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

CHttpClient::~CHttpClient()
  {
  iSession.Close();
  }

CHttpClient* CHttpClient::NewL(MHttpClientObserver& aObserver)
  {
  CHttpClient* me = NewLC(aObserver);
  CleanupStack::Pop(me);
  return me;
  }

CHttpClient::CHttpClient(MHttpClientObserver& aObserver) : iObserver(aObserver)
  {
  }

CHttpClient* CHttpClient::NewLC(MHttpClientObserver& aObserver)
  {
  CHttpClient* me = new (ELeave) CHttpClient(aObserver);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpClient::ConstructL()
  {

  }

void CHttpClient::ManageConnections(TBool aRequireWLAN)
	{
	// 0. If WLAN required-setting is off, take whatever connection we have, return
	// 1. Test if default connection is WLAN, if so, use this
	// 2. If we have a previously selected WLAN, try to connect to this one
	//    if successful, use it
	// 3. Let the user select a WLAN
	
	// WLAN som connectar men inte routar?
	
	CCommsDatabase *cdb = CCommsDatabase::NewL();
	unsigned long defaultIap;
	CCommsDbTableView* prefTableView = cdb->OpenTableLC( TPtrC( IAP ) );
	prefTableView->ReadUintL( TPtrC( COMMDB_ID), defaultIap );
	RDebug::Print(_L("default IAP: %d"), defaultIap);

	CCommsDbTableView *view = cdb->OpenIAPTableViewMatchingBearerSetLC(KCommDbBearerWLAN | KCommDbBearerLAN | KCommDbBearerPAN, ECommDbConnectionDirectionUnknown);
	unsigned long val = 0;
	int error =view->GotoFirstRecord();
	TBool isWLAN = EFalse;
	while (error == KErrNone) {
		TBuf<100> iapName;
		view->ReadTextL(TPtrC(COMMDB_NAME), iapName);
		RDebug::Print(_L("iapName: %S"), &iapName);
		view->ReadTextL(TPtrC(IAP_BEARER_TYPE), iapName);
		RDebug::Print(_L("bearer type: %S"), &iapName);
		view->ReadTextL(TPtrC(IAP_SERVICE_TYPE), iapName);
		RDebug::Print(_L("service type: %S"), &iapName);

		view->ReadUintL(TPtrC(COMMDB_ID), val);
		RDebug::Print(_L("IAP ID: %d, default=%d"), val, defaultIap);
		if (val == defaultIap) {
			isWLAN = ETrue;
		}
		
		/*view->ReadUintL(TPtrC(IAP_NETWORK), val);
		RDebug::Print(_L("Network ID: %d"), val);
		
	    view->ReadUintL(TPtrC(IAP_DIALOG_PREF), val);
		RDebug::Print(_L("Dialog pref: %d"), val);*/
		error = view->GotoNextRecord();
	}
	
	if (isWLAN) {
		User::InfoPrint(_L("On WLAN"));
	} else {
		User::InfoPrint(_L("No WLAN"));	
	}
	CleanupStack::PopAndDestroy(view);
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

void CHttpClient::GetL(TDesC& url, TDesC& fileName) {
	RDebug::Print(_L("CHttpClient::Get START"));
	//ManageConnections(ETrue);
	//return;
	iIsActive = ETrue;
	TBuf8<256> url8;
	url8.Copy(url);
	if (iTransactionCount == 0) {
		RDebug::Print(_L("** Opening session"));
		iSession.OpenL();
		RHTTPConnectionInfo connInfo = iSession.ConnectionInfo();
		THTTPHdrVal val;
		RStringPool pool = iSession.StringPool();

		//TInt connPtr=REINTERPRET_CAST(TInt, &connection);
		connInfo.Property(pool.StringF(HTTP::EHttpSocketConnection, 
				RHTTPSession::GetTable()), val);
		//
		// TInt connPtr = REINTERPRET_CAST(TInt, &iConnection);
		/*RConnection* connection=reinterpret_cast<RConnection*>(val.Int());
		TBuf<200> bearer;
		connection->GetDesSetting(TPtrC(IAP_BEARER_TYPE), bearer);
		RDebug::Print(_L("Bearer: %S"), &bearer);*/
			
		/*connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketConnection, 
				RHTTPSession::GetTable()), THTTPHdrVal(connPtr));*/
		
	}
	
	RStringPool strP = iSession.StringPool();
	RStringF method;
	method = strP.StringF(HTTP::EGET, RHTTPSession::GetTable());
	
	TUriParser8 uri; 
	uri.Parse(url8);
	CHttpEventHandler* eHandler;
	RDebug::Print(_L("Getting %S to %S"), &url, &fileName);
	eHandler = CHttpEventHandler::NewL(this, iObserver);
	eHandler->SetSaveFileName(fileName);
	RHTTPTransaction trans = iSession.OpenTransactionL(uri, *eHandler, method);
	  
	RHTTPHeaders hdr = trans.Request().GetHeaderCollection();
	// Add headers appropriate to all methods
	SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
	SetHeaderL(hdr, HTTP::EAccept, KAccept);
	
	iTransactionCount++;
	// submit the transaction
	trans.SubmitL();
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

