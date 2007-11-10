#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include <e32cmn.h>

_LIT(KPodcastDirectory, "c:\\logs\\");

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	CFeedEngine();
	virtual ~CFeedEngine();
	
public:
	void GetFeed(TFeedInfo& feedInfo);
	void GetPodcast(TDesC &fileUrl);
	
private:
	CHttpClient* iClient;
	
	void ConnectedCallback();
	void TransactionFinishedCallback();
	void DisconnectedCallback();
	void FileCompleteCallback(TFileName &fileName);
	TFileName iFileName;
	CFeedParser parser;
	
	void Item(TPodcastItem *item);
};

#endif /*FEEDENGINE_H_*/
