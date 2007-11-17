#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"
#include "FeedEngineObserver.h"

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	static CFeedEngine* NewL();
	virtual ~CFeedEngine();
	
public:
	void GetFeed(TFeedInfo* feedInfo);
	
	void AddDownload(TShowInfo *info);
	void StopDownloads();

	void GetAllShows(TShowInfoArray &array);	
	void GetShowsDownloading(TShowInfoArray &array);
	void GetShowsByFeed(TFeedInfo *info, TShowInfoArray &array);
	void GetFeeds(TFeedInfoArray& array);
	
	void AddObserver(MFeedEngineObserver *observer);
	
private:
	void ConstructL();
	CFeedEngine();


	void GetShow(TShowInfo *info);

	void LoadSettings();
	
	void LoadFeeds();
	void SaveFeeds();

	void LoadMetaData();
	void SaveMetaData();

	void ListAllFiles();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);

	// callbacks from HttpClient
	void ConnectedCallback();
	void DisconnectedCallback();
	void ProgressCallback(int percent);
	void DownloadInfoCallback(int size);

	void ShowCompleteCallback(TShowInfo *info);
	void FeedCompleteCallback(TFeedInfo *info);

	// callbacks from FeedParser
	void NewShowCallback(TShowInfo *item);
	void ParsingCompleteCallback();
	
	void AddShow(TShowInfo *item);
	void DownloadNextShow();
	
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtml(TDes &str);
private:
	// two HTTP connections, one for feeds and one for shows
	CHttpClient* iShowClient;
	CHttpClient* iFeedClient;

	// RSS parser
	CFeedParser* iParser;

	// the complete database of shows
	TShowInfoArray iShows;
	// where we store our shows
	TFileName iShowDir;
	
	// the list of feeds
	TFeedInfoArray iFeeds;
	TFileName iFeedListFile;

	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;
};

#endif /*FEEDENGINE_H_*/
