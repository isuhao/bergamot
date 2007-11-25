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
	void AddDownload(TShowInfo *info);
	void RemoveDownload(TInt aUid);

	void StopDownloads();
	void ResumeDownloads();
	
	TShowInfo* ShowDownloading();

	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TInt aFeedUid);
	void SelectShowsByDownloadState(TInt aDownloadState);
	void SelectNewShows();
	void SelectShowsDownloading();
	
	void Complete(CHttpClient* aClient, TBool aSuccessful);
	void AddShow(TShowInfo *item);
	void SaveShows();

	void PurgeShowsByFeed(TInt aFeedUid);
	void PurgePlayedShows();
	void PurgeOldShows();
	void PurgeShow(TInt aShowUid);
	
	TShowInfoArray& GetSelectedShows();
	
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

	void GetShow(TShowInfo *info);
	
	void LoadShows();

	void AppendToSelection(TShowInfo *aInfo);
	void ListAllFiles();
	void ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files);

	static TInt CompareShowsByDate(const TShowInfo &a, const TShowInfo &b);

	void DownloadNextShow();
	void MakeFileNameFromUrl(TDesC &aUrl, TFileName &fileName);
private:
	CHttpClient* iShowClient;

	// the complete database of shows
	TShowInfoArray iShows;

	// the current selection of shows
	TShowInfoArray iSelectedShows;
	
	// list of shows waiting to download
	TShowInfoArray iShowsDownloading;
	
	// the file session used to read and write settings
	RFs iFs;
	
	// The show we are currently downloading
	TShowInfo* iShowDownloading;
	
	CPodcastModel& iPodcastModel;

	// prevents auto download on files read from DB
	TBool iSuppressAutoDownload;
	
	// observers that will receive callbacks
    RArray<MShowEngineObserver*> iObservers;

    TLinearOrder<TShowInfo>* iLinearOrder;
};

#endif /*SHOWENGINE_H_*/
