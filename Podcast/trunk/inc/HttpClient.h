#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpClientObserver.h"
#include "HttpEventHandler.h"

_LIT8(KUserAgent, "PodcastPlayer (0.2)");
_LIT8(KAccept, "*/*");

class CHttpClient : public CBase
{
public:
	virtual ~CHttpClient();
	static CHttpClient* NewL(MHttpClientObserver& aResObs);
	void GetL(TDesC& url, TDesC& fileName);
  	TBool IsActive();
	void ClientRequestCompleteL(TBool aSuccessful);
private:
	CHttpClient(MHttpClientObserver& aResObs);
	static CHttpClient* NewLC(MHttpClientObserver& aResObs);
	void ConstructL();
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);

private:
	RHTTPSession iSession;
	MHttpClientObserver& iObserver;
	TBool iIsActive;
};
#endif
