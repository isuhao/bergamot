#ifndef SETTINGSENGINE_H_
#define SETTINGSENGINE_H_

#include <e32base.h>
#include "PodcastModel.h"

const TInt KSettingsUid = 1000;
_LIT(KInternalPodcastDir, "c:\\Media files\\Podcasts\\");
_LIT(KFlashPodcastDir, "\\Podcasts\\");

_LIT(KConfigImportFile, "config.cfg");
_LIT(KDefaultFeedsFile, "defaultfeeds.cfg");

_LIT(KConfigFile, "config.db");
_LIT(KShowDB, "shows.db");
_LIT(KFeedDB, "feeds.db");


const TInt KMaxVolume = 100;
const TInt KVolumeDelta = 10;

enum TAutoDownloadSetting
{
	EAutoDownloadOff,
	EAutoDownloadFeeds,
	EAutoDownloadFeedsAndShows
};

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
	void GetDefaultBaseDir(TDes &aBaseDir);

public:
	TFileName& BaseDir();
	TFileName& FeedListFile();
	TInt UpdateFeedInterval();
	TInt MaxSimultaneousDownloads();
	TAutoDownloadSetting DownloadAutomatically();	
	TFileName& DefaultFeedsFileName();
	TBool DownloadOnlyOnWLAN();	
	TInt SpecificIAP();
	TInt MaxListItems();
	void SetBaseDir(TFileName& aFileName);
	void SetUpdateFeedInterval(TInt aInterval);
	void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
	void SetDownloadAutomatically(TAutoDownloadSetting aAutoOn);
	void SetDownloadOnlyOnWLAN(TBool aOnlyOnWLAN);
	void SetSpecificIAP(TInt aIap);
	TFileName PrivatePath();
	
	TUint Volume();
	void SetVolume(TUint aVolume);

	void SaveSettingsL();

private:
	// the settings we serialize
	TFileName iBaseDir;
	TFileName iDefaultFeedsFile;
	TInt iUpdateFeedInterval;
	TAutoDownloadSetting iDownloadAutomatically;
	TBool iDownloadOnlyOnWLAN;
	TInt iMaxSimultaneousDownloads;
	TInt iIap;
	TInt iVolume;
	TInt iMaxListItems;
	
	// the file session used to read and write settings
	RFs iFs;
	
	// reference to the model
	CPodcastModel &iPodcastModel;

};

#endif /*SETTINGSENGINE_H_*/
