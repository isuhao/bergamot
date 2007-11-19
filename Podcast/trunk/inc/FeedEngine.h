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

class CFeedEngine : public CBase, public MHttpEventHandlerCallbacks, public MFeedParserCallbacks
{
public:
	static CFeedEngine* NewL();
	virtual ~CFeedEngine();
	
public:
	void UpdateFeed(TInt aFeedUid);
	
	void AddDownload(TShowInfo *info);
	void StopDownloads();

	TShowInfo* ShowDownloading();

	void GetFeeds(TFeedInfoArray& array);
	TFeedInfo* GetFeedInfoByUid(int aFeedUid);
	
	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TInt aFeedUid);
	void SelectShowsByDownloadState(TInt aDownloadState);
	void SelectNewShows();
	void SelectShowsDownloading();
	
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
	void ConnectedCallback(CHttpClient* aClient);
	void DisconnectedCallback(CHttpClient* aClient);
	void ProgressCallback(CHttpClient* aHttpClient, int aBytes, int aTotalBytes);
	void DownloadInfoCallback(CHttpClient* aClient, int aSize);

	void ShowCompleteCallback(TShowInfo *info);
	void FeedCompleteCallback(TFeedInfo *info);

	// callbacks from FeedParser
	void NewShowCallback(TShowInfo *item);
	void ParsingCompleteCallback(TFeedInfo *item);
	
	void AddShow(TShowInfo *item);
	void AddFeed(TFeedInfo *item);

	void GetFeedDir(TFeedInfo *aFeedInfo, TFileName &aDir);
	void GetFeedImage(TFeedInfo *aFeedInfo);
	void MakeFileNameFromUrl(TDesC &aUrl, TFileName &fileName);

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
	
	// list of shows waiting to download
	TShowInfoArray iShowsDownloading;
	
	// where we store our shows
	TFileName iShowDir;
	
	// the list of feeds
	TFeedInfoArray iFeeds;
	TFileName iFeedListFile;

	// the file session used to read and write settings
	RFs iFs;
	
	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;

	// The show we are currently downloading
	TShowInfo* iShowDownloading;
};
#endif /*FEEDENGINE_H_*/
