#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	CFeedEngine();
	virtual ~CFeedEngine();
	
public:
	void GetFeed(TFeedInfo& feedInfo);
	void GetPodcast(TShowInfo *info);
	void Cancel();
	void LoadSettings();
	void LoadFeeds();
	void SaveFeeds();

	TShowInfoArray& GetPodcasts();
	void ListAllPodcasts();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);
	
	TShowInfoArray & GetItems();
	TFeedInfoArray& GetFeeds();
	
private:
	void ConnectedCallback();
	void DisconnectedCallback();
	void ProgressCallback(int percent);
	void FileCompleteCallback(TFileName &fileName);
	void DownloadInfoCallback(int size);
	
private:
	CHttpClient* iClient;
	TFileName iFileName;
	CFeedParser parser;
	TShowInfoArray items;
	void Item(TShowInfo *item);
	TShowInfoArray files;
	TFeedInfoArray feeds;
	TFileName iFeedListFile;
	TFileName iPodcastDir;
	TShowInfoArray downloadQueue;
};

#endif /*FEEDENGINE_H_*/
