#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"

class TPodcastId {
public:
	TPodcastId(int aId, const TDesC &aFileName, const TDesC &aTitle) {
		iId =aId;
		iFileName.Copy(aFileName);
		iTitle.Copy(aTitle);
		iPlaying = EFalse;
	}
	
	int iId;
	TBuf<256> iFileName;
	TBuf<256> iTitle;
	TTimeIntervalMicroSeconds iPosition;
	TBool iPlaying;
};

typedef RArray<TPodcastId*> TPodcastIdArray;

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	CFeedEngine();
	virtual ~CFeedEngine();
	
public:
	void GetFeed(TFeedInfo& feedInfo);
	void GetPodcast(TPodcastInfo &info);
	void Cancel();
	
	void LoadSettings();
	void LoadFeeds();
	void SaveFeeds();

	RArray<TPodcastId*> & GetPodcasts();
	void ListAllPodcasts();
	void ListDir(RFs &rfs, TDesC &folder, TPodcastIdArray &files);
	
	RArray <TPodcastInfo*>& GetItems();
	RArray <TFeedInfo*>& GetFeeds();
	
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
	TPodcastIdArray files;
	RArray<TFeedInfo*> feeds;
	TFileName iFeedListFile;
	TFileName iPodcastDir;
};

#endif /*FEEDENGINE_H_*/
