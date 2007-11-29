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
	
	CShowInfo* ShowDownloading();

	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TUint aFeedUid);
	void SelectShowsByDownloadState(TInt aDownloadState);
	void SelectNewShows();
	void SelectShowsDownloading();
	
	void Complete(CHttpClient* aClient, TBool aSuccessful);
	void AddShow(CShowInfo *item);
	void SaveShows();

	void PurgeShowsByFeed(TInt aFeedUid);
	void PurgePlayedShows();
	void PurgeOldShows();
	void PurgeShow(TInt aShowUid);
	
	CShowInfoArray& GetSelectedShows();
	
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

	void AppendToSelection(CShowInfo *aInfo);
	static TInt CompareShowsByDate(const CShowInfo &a, const CShowInfo &b);

	void DownloadNextShow();
private:
	CHttpClient* iShowClient;

	// the complete database of shows
	CShowInfoArray iShows;

	// the current selection of shows
	CShowInfoArray iSelectedShows;
	
	// list of shows waiting to download
	CShowInfoArray iShowsDownloading;
	
	// the file session used to read and write settings
	RFs iFs;
	
	// The show we are currently downloading
	CShowInfo* iShowDownloading;
	
	CPodcastModel& iPodcastModel;

	// prevents auto download on files read from DB
	TBool iSuppressAutoDownload;
	
	// observers that will receive callbacks
    RArray<MShowEngineObserver*> iObservers;

    TLinearOrder<CShowInfo>* iLinearOrder;
};

#endif /*SHOWENGINE_H_*/
