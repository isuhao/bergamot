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
	ENotUpdating,
	EUpdatingFeed,
	EUpdatingImage
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
	void UpdateAllFeedsL();

	void GetFeeds(RFeedInfoArray& array);
	CFeedInfo* GetFeedInfoByUid(TUint aFeedUid);	

	void AddObserver(MFeedEngineObserver *observer);
	void RemoveObserver(MFeedEngineObserver *observer);

	void RunFeedTimer();
	
	void FileNameFromUrl(TDesC &aUrl, TFileName &aFileName);
	void EnsureProperPathName(TFileName &aPath);
	
	/**
	 * Returns the current internal state of the feed engine4
	 */
	TClientState ClientState();

	/**
	 * Returns the current updating client UID if clientstate is != ENotUpdateing
	 * @return TUint
	 */
	TUint ActiveClientUid();
private:
	void ConstructL();
	CFeedEngine(CPodcastModel& aPodcastModel);

	void ImportFeedsL(TFileName &aFile);
	TBool LoadFeeds();
	void SaveFeeds();

	// from HttpClientObserver
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, TInt aBytes, TInt aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, TInt aSize);
	void CompleteL(CHttpClient* aClient, TBool aSuccessful);

	// from FeedParser
	TBool NewShow(CShowInfo *item);
	void ParsingComplete(CFeedInfo *item);

	void GetFeedImageL(CFeedInfo *aFeedInfo);
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement);
	void CleanHtml(TDes &str);
	
	static TInt CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b);
	void UpdateNextFeedL();
	void NotifyFeedUpdateComplete();
	
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
