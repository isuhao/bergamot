#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpClientObserver.h"
#include "HttpEventHandler.h"
#include "es_sock.h"

_LIT8(KUserAgent, "PodcastPlayer (0.2)");
_LIT8(KAccept, "*/*");

class CHttpClient : public CBase
{
public:
	virtual ~CHttpClient();
	static CHttpClient* NewL(MHttpClientObserver& aResObs);
	void GetL(TDesC& url, TDesC& fileName, TInt aIap = -1, TBool aSilent = EFalse);
	void Stop();
  	TBool IsActive();
	void ClientRequestCompleteL(TBool aSuccessful);
	void ManageConnections(TBool aRequireWLAN);
	void SetResumeEnabled(TBool aEnabled);

private:
	CHttpClient(MHttpClientObserver& aResObs);
	static CHttpClient* NewLC(MHttpClientObserver& aResObs);
	void ConstructL();
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);

private:
	RHTTPSession iSession;
	MHttpClientObserver& iObserver;
	TBool iIsActive;
	TInt iTransactionCount;
	RHTTPTransaction iTrans;
	CHttpEventHandler* iHandler;
	TBool iResumeEnabled;
	
	RSocketServ iSocketServ;
	RConnection iConnection;
};
#endif
