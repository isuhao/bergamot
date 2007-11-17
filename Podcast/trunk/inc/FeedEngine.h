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
	void UpdateFeed(TInt aFeedUid);
	
	void AddDownload(TShowInfo *info);
	void StopDownloads();

	void GetFeeds(TFeedInfoArray& array);
	TFeedInfo* GetFeedInfoByUid(int aFeedUid);
	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TInt aFeedUid);
	void SelectShowsByDownloadState(TInt aDownloadState);
	
	TShowInfoArray& GetSelectedShows();
	
	void AddObserver(MFeedEngineObserver *observer);
	
private:
	void ConstructL();
	CFeedEngine();

	void GetShow(TShowInfo *info);

	void LoadSettings();
	void LoadUserFeeds();

	void LoadFeeds();
	void SaveFeeds();

	void LoadShows();
	void SaveShows();

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
	void ParsingCompleteCallback(TFeedInfo *item);
	
	void AddShow(TShowInfo *item);
	void AddFeed(TFeedInfo *item);

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

	// the current selection of shows
	TShowInfoArray iSelectedShows;
	
	// where we store our shows
	TFileName iShowDir;
	
	// the list of feeds
	TFeedInfoArray iFeeds;
	TFileName iFeedListFile;

	// the file session used to read and write settings
	RFs iFs;
	
	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;
};

#endif /*FEEDENGINE_H_*/
