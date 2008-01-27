#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpClientObserver.h"
#include "HttpEventHandler.h"
#include "PodcastModel.h"
#include "es_sock.h"

_LIT8(KUserAgent, "PodcastPlayer (0.2)");
_LIT8(KAccept, "*/*");

class CHttpClient : public CBase
{
public:
	virtual ~CHttpClient();
	static CHttpClient* NewL(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	void GetL(TDesC& url, TDesC& fileName, TBool aSilent = EFalse);
	void Stop();
  	TBool IsActive();
	void ClientRequestCompleteL(TBool aSuccessful);
	void SetResumeEnabled(TBool aEnabled);

private:
	CHttpClient(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	static CHttpClient* NewLC(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	void ConstructL();
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);

private:
	RHTTPSession iSession;
	MHttpClientObserver& iObserver;
	TBool iIsActive;
	RHTTPTransaction iTrans;
	CHttpEventHandler* iHandler;
	TBool iResumeEnabled;
	CPodcastModel& iPodcastModel;
	TInt iTransactionCount;
};
#endif
