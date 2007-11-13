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
	void GetFeed(TFeedInfo* feedInfo);
	void GetShow(TShowInfo *info);
	void Cancel();

	void LoadSettings();
	
	void LoadFeeds();
	void SaveFeeds();

	void LoadMetaData();
	void SaveMetaData();
	
	void AddDownload(TShowInfo *info);
	TShowInfoArray& GetDownloads();
	TShowInfoArray& GetPodcasts();
	void ListAllPodcasts();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);
	
	TShowInfoArray & GetItems();
	TFeedInfoArray& GetFeeds();
	
private:
	void ConnectedCallback();
	void DisconnectedCallback();
	void ProgressCallback(int percent);
	void DownloadInfoCallback(int size);

	void ShowCompleteCallback(TShowInfo *info);
	void FeedCompleteCallback(TFeedInfo *info);
	  
	void DownloadNextShow();
	void CleanHtml(TDes &str);
	
private:
	CHttpClient* iClient;
	TFileName iFileName;
	CFeedParser parser;
	TShowInfoArray items;
	void Item(TShowInfo *item);
	TShowInfoArray files;
	TFeedInfoArray feeds;
	TFileName iFeedListFile;
	TFileName iShowDir;
	TShowInfoArray downloadQueue;
};

#endif /*FEEDENGINE_H_*/
