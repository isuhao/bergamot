#ifndef FEEDENGINE_H_
#define FEEDENGINE_H_

#include "HttpClientObserver.h"
#include "HttpClient.h"
#include "FeedParser.h"
#include "FeedInfo.h"
#include "ShowInfo.h"
#include <e32cmn.h>
#include "PodcastClientGlobals.h"
#include "FeedEngineObserver.h"
#include "FeedTimer.h"

class CPodcastModel;

enum TClientState {
	EFeed,
	EImage
};

class CFeedEngine : public CBase, public MHttpClientObserver, public MFeedParserObserver
{
public:
	static CFeedEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CFeedEngine();
	
public:
	void AddFeed(TFeedInfo *item);
	void RemoveFeed(TInt aUid);
	void UpdateFeed(TInt aFeedUid);

	void GetFeeds(TFeedInfoArray& array);
	TFeedInfo* GetFeedInfoByUid(int aFeedUid);
	void GetFeedDir(TFeedInfo *aFeedInfo, TFileName &aDir);
	
	void AddObserver(MFeedEngineObserver *observer);
	void UpdateAllFeeds();
	
private:
	void ConstructL();
	CFeedEngine(CPodcastModel& aPodcastModel);

	void LoadUserFeeds();

	void LoadFeeds();
	void SaveFeeds();

	// from HttpClientObserver
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, int aSize);
	void Complete(CHttpClient* aClient, TBool aSuccessful);

	// callbacks from FeedParser
	void NewShow(TShowInfo *item);
	void ParsingComplete(TFeedInfo *item);

	void GetFeedImage(TFeedInfo *aFeedInfo);
	void MakeFileNameFromUrl(TDesC &aUrl, TFileName &fileName);
	
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtml(TDes &str);
	
	static TInt CompareFeedsByTitle(const TFeedInfo &a, const TFeedInfo &b);

private:
	CHttpClient* iFeedClient;
	TClientState iClientState;
	
	CPodcastModel& iPodcastModel;
	
	// RSS parser
	CFeedParser* iParser;
	
	// the list of feeds
	TFeedInfoArray iFeeds;

	TFeedInfo *iActiveFeed;
	
	CFeedTimer iFeedTimer;
	
	// the file session used to read and write settings
	RFs iFs;

	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;
};
#endif /*FEEDENGINE_H_*/
