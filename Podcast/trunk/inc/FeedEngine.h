#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpEventHandlerCallbacks.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"
#include <mdaaudiosampleplayer.h>

_LIT(KMetaDataSemaphoreName, "MetaData");
const int KMetaDataFileVersion = 1;

class CFeedEngine : public MHttpEventHandlerCallbacks, public MMdaAudioPlayerCallback, public MFeedParserCallbacks
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
	TShowInfoArray* GetAllShows();	
	TShowInfoArray* GetShowsDownloading();
	TShowInfoArray* GetShowsByFeed(TDesC &feedTitle);

	void ListAllFiles();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);
	
	TFeedInfoArray& GetFeeds();
	
	void LoadMetaDataFromFile(TShowInfo *info);
	
private:
	void ConstructL();
	CFeedEngine();

	void ConnectedCallback();
	void DisconnectedCallback();
	void ProgressCallback(int percent);
	void DownloadInfoCallback(int size);

	void ShowCompleteCallback(TShowInfo *info);
	void FeedCompleteCallback(TFeedInfo *info);

    void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);

	void DownloadNextShow();
	void CleanHtml(TDes &str);

	void Item(TShowInfo *item);
	void ParsingComplete();
	
	void AddShow(TShowInfo *item);

private:
	// two HTTP connections, one for feeds and one for shows
	CHttpClient* showClient;
	CHttpClient* feedClient;

	CFeedParser parser;

	// the complete database of shows
	TShowInfoArray shows;
	// where we store our shows
	TFileName iShowDir;
	
	// the list of feeds
	TFeedInfoArray feeds;
	TFileName iFeedListFile;

	// for reading meta data from files
    CMdaAudioPlayerUtility *iPlayer;
    
    TBool iDownloading;
};

#endif /*FEEDENGINE_H_*/
