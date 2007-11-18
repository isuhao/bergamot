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

// HttpEventHandler.h

#ifndef HTTPEVENTHANDLER_H
#define HTTPEVENTHANDLER_H

#include <e32base.h>
#include <http/mhttpdatasupplier.h>
#include <http/rhttpsession.h>
#include <http/rhttpheaders.h>
#include <http/mhttptransactioncallback.h>
#include <httpstringconstants.h>
#include "HttpEventHandlerCallbacks.h"

const TInt KMaxSubmitSize = 1024;
const TInt KMaxHeaderNameLen = 32;
const TInt KMaxHeaderValueLen = 128;

class CHttpEventHandler : public CBase,
                          public MHTTPTransactionCallback
  {
  public:
	virtual ~CHttpEventHandler();
	static CHttpEventHandler* NewLC(CHttpClient* aClient, MHttpEventHandlerCallbacks &aCallbacks);
	static CHttpEventHandler* NewL(CHttpClient* aClient, MHttpEventHandlerCallbacks &aCallbacks);
	void SetVerbose(TBool aVerbose);
	void SetSaveFileName(TDesC &fName);
	TBool Verbose() const;
	//
	// methods from MHTTPTransactionCallback
	//
	virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);

protected:
	CHttpEventHandler(CHttpClient* aClient, MHttpEventHandlerCallbacks &aCallbacks);
	void ConstructL();
private:
	void DumpRespHeadersL(RHTTPTransaction& aTrans);
	void DumpRespBody(RHTTPTransaction& aTrans);
	void DumpIt(const TDesC8& aData);
	TBool iVerbose;
	TBool iSavingResponseBody;
	RFs iFileServ;
	RFile iRespBodyFile;
	//TFileName iRespBodyFileName;
	TFileName iFileName;
	TParse iParsedFileName;
	MHTTPDataSupplier* iRespBody;
	MHttpEventHandlerCallbacks& iCallbacks;
	TInt iBytesDownloaded;
	TInt iBytesTotal;

	/** Http client which owns this event handler */
	CHttpClient* iHttpClient;
};

#endif
