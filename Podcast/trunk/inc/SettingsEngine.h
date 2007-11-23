#ifndef SETTINGSENGINE_H_
#define SETTINGSENGINE_H_

#include <e32base.h>
#include "PodcastModel.h"

#ifdef __WINS__
	_LIT(KPodcastDir, "c:\\logs\\");
#else
	_LIT(KPodcastDir, "e:\\podcasts\\");	
#endif

_LIT(KConfigFile, "config.cfg");
_LIT(KFeedsFileName, "feeds.cfg");
_LIT(KShowDB, "metadata.db");
_LIT(KFeedDB, "feeds.db");
_LIT(KDefaultFeedsFile, "defaultfeeds.cfg");

class CSettingsEngine : public CBase
{
public:
	static CSettingsEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CSettingsEngine();
	
private:
	void ConstructL();
	CSettingsEngine(CPodcastModel& aPodcastModel);
	void LoadSettings();

public:
	TFileName& ShowDir();
	TFileName& FeedListFile();
	TInt UpdateFeedInterval();
	TInt MaxSimultaneousDownloads();
	TBool DownloadAutomatically();	
	TFileName& DefaultFeedsFileName();
	TBool DownloadOnlyOnWLAN();	
	TInt SpecificIAP();
	void SetShowDir(TFileName& aFileName);
	void SetUpdateFeedInterval(TInt aInterval);
	void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
	void SetDownloadAutomatically(TBool aAutoOn);
	void SetDownloadOnlyOnWLAN(TBool aOnlyOnWLAN);
	void SetSpecificIAP(TInt aIap);
private:
	TFileName iShowDir;
	TFileName iFeedListFile;
	TFileName iDefaultFeedsFile;
	TInt iUpdateFeedInterval;
	TBool iDownloadAutomatically;
	TBool iDownloadOnlyOnWLAN;
	TInt iMaxSimultaneousDownloads;
	TInt iIap;
	
	TFileName PrivatePath();
	
	// the file session used to read and write settings
	RFs iFs;
	
	CPodcastModel &iPodcastModel;
	
};

#endif /*SETTINGSENGINE_H_*/
