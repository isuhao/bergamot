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


const TUint KMaxVolume = 100;
const TUint KVolumeDelta = 10;

enum TAutoUpdateSetting
{
	EAutoUpdateOff,
	EAutoUpdatePeriodically,
	EAutoUpdateAtTime
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
	TTime UpdateFeedTime();
	TInt MaxSimultaneousDownloads();
	TAutoUpdateSetting UpdateAutomatically();	
	TBool DownloadAutomatically();
	TFileName& DefaultFeedsFileName();
	TInt SpecificIAP();
	TInt MaxListItems();
	void SetBaseDir(TFileName& aFileName);
	void SetUpdateFeedInterval(TInt aInterval);
	void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
	void SetUpdateAutomatically(TAutoUpdateSetting aAutoOn);
	void SetDownloadAutomatically(TBool aAutoDownloadOn);
	void SetUpdateFeedTime(TTime aTime);
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
	TAutoUpdateSetting iUpdateAutomatically;
	TBool iDownloadAutomatically;
	TInt iMaxSimultaneousDownloads;
	TInt iIap;
	TInt iVolume;
	TInt iMaxListItems;
	TTime iUpdateFeedTime;
	// the file session used to read and write settings
	RFs iFs;
	
	// reference to the model
	CPodcastModel &iPodcastModel;

};

#endif /*SETTINGSENGINE_H_*/
