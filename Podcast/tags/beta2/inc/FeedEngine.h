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
	void AddFeed(CFeedInfo *item);
	void RemoveFeed(TUint aUid);
	void UpdateFeedL(TUint aFeedUid);
	void UpdateAllFeeds();

	void GetFeeds(RFeedInfoArray& array);
	CFeedInfo* GetFeedInfoByUid(TUint aFeedUid);	

	void AddObserver(MFeedEngineObserver *observer);

	void RunFeedTimer();
	
	void FileNameFromUrl(TDesC &aUrl, TFileName &aFileName);
	void EnsureProperPathName(TFileName &aPath);
	
private:
	void ConstructL();
	CFeedEngine(CPodcastModel& aPodcastModel);

	void ImportFeeds(TFileName &aFile);
	TBool LoadFeeds();
	void SaveFeeds();

	// from HttpClientObserver
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, int aSize);
	void Complete(CHttpClient* aClient, TBool aSuccessful);

	// from FeedParser
	void NewShow(CShowInfo *item);
	void ParsingComplete(CFeedInfo *item);

	void GetFeedImageL(CFeedInfo *aFeedInfo);
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtml(TDes &str);
	
	static TInt CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b);
	void UpdateNextFeed();
private:
	CHttpClient* iFeedClient;
	TClientState iClientState;
	CFeedTimer iFeedTimer;

	CPodcastModel& iPodcastModel;
	
	// RSS parser
	CFeedParser* iParser;
	
	// the list of feeds
	RFeedInfoArray iFeeds;

	CFeedInfo *iActiveFeed;
	TFileName iUpdatingFeedFileName;
	// the file session used to read and write settings
	RFs iFs;

	RFeedInfoArray iFeedsUpdating;
	
	// observers that will receive callbacks
    RArray<MFeedEngineObserver*> iObservers;
};
#endif /*FEEDENGINE_H_*/
