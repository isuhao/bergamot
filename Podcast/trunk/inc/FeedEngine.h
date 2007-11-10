#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	CFeedEngine();
	virtual ~CFeedEngine();
	
public:
	void GetFeed(TFeedInfo& feedInfo);
	void GetPodcast(TPodcastInfo &info);
	RArray <TPodcastInfo*>& GetItems();
private:
	CHttpClient* iClient;
	
	void ConnectedCallback();
	void TransactionFinishedCallback();
	void DisconnectedCallback();
	void FileCompleteCallback(TFileName &fileName);
	TFileName iFileName;
	CFeedParser parser;
	RArray<TPodcastInfo*> items;
	void Item(TPodcastInfo *item);
};

#endif /*FEEDENGINE_H_*/
