#ifndef SHOWENGINE_H_
#define SHOWENGINE_H_

#include <e32base.h>
#include "ShowInfo.h"
#include "PodcastModel.h"
#include "HttpClient.h"
#include "ShowEngineObserver.h"

class CShowEngine : public CBase, public MHttpClientObserver
{
public:
	static CShowEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CShowEngine();
	
public:
	void AddDownload(CShowInfo *info);
	void RemoveDownload(TInt aUid);

	void StopDownloads();
	void ResumeDownloads();
	TBool DownloadsStopped();
	
	CShowInfo* ShowDownloading();
	CShowInfo* GetShowByUidL(TUint aShowUid);

	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TUint aFeedUid);
	void SelectShowsDownloaded();
	void SelectNewShows();
	void SelectShowsDownloading();
	
	void Complete(CHttpClient* aClient, TBool aSuccessful);
	void AddShow(CShowInfo *item);
	void SaveShows();

	void PurgeShowsByFeed(TInt aFeedUid);
	void PurgePlayedShows();
	void PurgeOldShows();
	void PurgeShow(TInt aShowUid);
	
	RShowInfoArray& GetSelectedShows();
	void SetPlayedByFeed(TUint aFeedUid);
	void CheckFiles();
	void GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed );
	void SetSelectUnplayedOnly(TBool aOnlyUnplayed);
	
	void AddObserver(MShowEngineObserver *observer);
protected:
	// from HttpClientObserver, dont have to be public
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, int aSize);
private:
	CShowEngine(CPodcastModel& aPodcastModel);
	void ConstructL();

	void GetShow(CShowInfo *info);
	
	void LoadShows();

	static TInt CompareShowsByDate(const CShowInfo &a, const CShowInfo &b);

	void DownloadNextShow();
	

	void ListDir(TFileName &folder);

	static TBool CompareShowsByUid(const CShowInfo &a, const CShowInfo &b);
private:
	CHttpClient* iShowClient;

	// the complete database of shows
	RShowInfoArray iShows;

	// the current selection of shows
	RShowInfoArray iSelectedShows;
	
	// list of shows waiting to download
	RShowInfoArray iShowsDownloading;
	
	// the file session used to read and write settings
	RFs iFs;
	
	// The show we are currently downloading
	CShowInfo* iShowDownloading;
	
	CPodcastModel& iPodcastModel;

	// prevents auto download on files read from DB
	TBool iSuppressAutoDownload;
	
	// observers that will receive callbacks
    RArray<MShowEngineObserver*> iObservers;

    TBool iDownloadsSuspended;
    
    TBool iSelectOnlyUnplayed;
    
    TUint iDownloadErrors;
    TLinearOrder<CShowInfo>* iLinearOrder;
};

#endif /*SHOWENGINE_H_*/
