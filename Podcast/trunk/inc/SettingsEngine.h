#ifndef SETTINGSENGINE_H_
#define SETTINGSENGINE_H_

#include <e32base.h>
#include "PodcastModel.h"

#ifdef __WINS__
	_LIT(KPodcastDir, "c:\\Media files\\Podcasts\\");
#else
	_LIT(KPodcastDir, "e:\\Media files\\Podcasts\\");
#endif

const TInt KSettingsUid = 1000;

_LIT(KConfigImportFile, "config.cfg");
_LIT(KFeedsImportFileName, "feeds.cfg");
_LIT(KConfigFile, "config.db");
_LIT(KShowDB, "shows.db");
_LIT(KFeedDB, "feeds.db");
_LIT(KFeedDir, "feeds\\");
_LIT(KDefaultFeedsFile, "defaultfeeds.cfg");

const TInt KMaxVolume = 100;

class CSettingsEngine : public CBase
{
public:
	static CSettingsEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CSettingsEngine();
	
private:
	void ConstructL();
	CSettingsEngine(CPodcastModel& aPodcastModel);
	void ImportSettings();
	void LoadSettingsL();
	void SaveSettingsL();

public:
	TFileName& BaseDir();
	TFileName& FeedListFile();
	TInt UpdateFeedInterval();
	TInt MaxSimultaneousDownloads();
	TBool DownloadAutomatically();	
	TFileName& DefaultFeedsFileName();
	TBool DownloadOnlyOnWLAN();	
	TInt SpecificIAP();
	TInt MaxListItems();
	void SetBaseDir(TFileName& aFileName);
	void SetUpdateFeedInterval(TInt aInterval);
	void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
	void SetDownloadAutomatically(TBool aAutoOn);
	void SetDownloadOnlyOnWLAN(TBool aOnlyOnWLAN);
	void SetSpecificIAP(TInt aIap);
	TFileName PrivatePath();
	
	TUint Volume();
	void SetVolume(TUint aVolume);

private:
	// the settings we serialize
	TFileName iBaseDir;
	TFileName iFeedListFile;
	TInt iUpdateFeedInterval;
	TBool iDownloadAutomatically;
	TBool iDownloadOnlyOnWLAN;
	TInt iMaxSimultaneousDownloads;
	TInt iIap;
	TInt iVolume;
	TInt iMaxListItems;
	
	// the file session used to read and write settings
	RFs iFs;
	// reference to the model
	CPodcastModel &iPodcastModel;
	TFileName iDefaultFeedsFile;

};

#endif /*SETTINGSENGINE_H_*/
