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

const int KMetaDataFileVersion = 1;

class CFeedEngine : public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	static CFeedEngine* NewL();
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
	void GetAllShows(TShowInfoArray &array);	
	void GetShowsDownloading(TShowInfoArray &array);
	void GetShowsByFeed(TFeedInfo *info, TShowInfoArray &array);

	void ListAllFiles();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);

	void GetFeeds(TFeedInfoArray& array);
	
	void LoadMetaDataFromFile(TShowInfo *info);
	
	void AddObserver(MFeedEngineObserver *observer);
	
private:
	void ConstructL();
	CFeedEngine();

	void ConnectedCallback();
	void DisconnectedCallback();
	void ProgressCallback(int percent);
	void DownloadInfoCallback(int size);

	void ShowCompleteCallback(TShowInfo *info);
	void FeedCompleteCallback(TFeedInfo *info);

	void DownloadNextShow();

	void Item(TShowInfo *item);
	void ParsingComplete();
	
	void AddShow(TShowInfo *item);
	
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtml(TDes &str);

private:
	// two HTTP connections, one for feeds and one for shows
	CHttpClient* showClient;
	CHttpClient* feedClient;

	CFeedParser* iParser;

	// the complete database of shows
	TShowInfoArray shows;
	// where we store our shows
	TFileName iShowDir;
	
	// the list of feeds
	TFeedInfoArray feeds;
	TFileName iFeedListFile;

    RArray<MFeedEngineObserver*> observers;
    
    TBool iDownloading;
};

#endif /*FEEDENGINE_H_*/
