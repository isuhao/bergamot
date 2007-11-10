#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include <e32cmn.h>

class CFeedEngine : public MHttpEventHandlerCallbacks
{
public:
	CFeedEngine();
	virtual ~CFeedEngine();
	
public:
	void DownloadFeed(TDesC &feedUrl);
	
private:
	CHttpClient* iClient;
	
	void ConnectedCallback();
	void TransactionFinishedCallback();
	void DisconnectedCallback();
};

#endif /*FEEDENGINE_H_*/
