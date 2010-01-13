#ifndef SETTINGSENGINE_H_
#define SETTINGSENGINE_H_

#include <e32base.h>
#include "PodcastModel.h"

#ifdef UIQ3
_LIT(KPodcastDir1, "E:\\Podcasts\\");
_LIT(KPodcastDir2, "D:\\Podcasts\\");
_LIT(KPodcastDir3, "C:\\Media files\\Podcasts\\");
#else
_LIT(KPodcastDir1, "E:\\Podcasts\\");
_LIT(KPodcastDir2, "C:\\Podcasts\\");
_LIT(KPodcastDir3, "C:\\Podcasts\\");
#endif

_LIT(KConfigImportFile, "config.cfg");
_LIT(KDefaultFeedsFile, "defaultfeeds.xml");
_LIT(KImportFeedsFile, "feeds.xml");

_LIT(KConfigFile, "config.db");
_LIT(KShowDB, "shows.db");
_LIT(KFeedDB, "feeds.db");
_LIT(KBookDB, "books.db");
_LIT(KEscarpodDB, "escarpod.sqlite");

// constants
const TInt KSettingsUid = 1000;
const TInt KMaxVolume = 100;
const TInt KVolumeDelta = 10;
const TInt KDefaultUpdateFeedInterval = 60;
const TInt KDefaultMaxSimultaneousDownloads = 1;
const TInt KDefaultMaxListItems = 200;
const TInt KDefaultSeekTime = 15;

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
	

		TFileName DefaultFeedsFileName();
		TFileName ImportFeedsFileName();
		TFileName PrivatePath();

		TInt MaxListItems();

		IMPORT_C TFileName& BaseDir();
		IMPORT_C void SetBaseDir(TFileName& aFileName);

		IMPORT_C TInt UpdateFeedInterval();
		IMPORT_C void SetUpdateFeedInterval(TInt aInterval);

		IMPORT_C TInt MaxSimultaneousDownloads();
		IMPORT_C void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
		
		IMPORT_C TAutoUpdateSetting UpdateAutomatically();	
		IMPORT_C void SetUpdateAutomatically(TAutoUpdateSetting aAutoOn);
		
		IMPORT_C TBool DownloadAutomatically();
		IMPORT_C void SetDownloadAutomatically(TBool aAutoDownloadOn);
		
		IMPORT_C TTime UpdateFeedTime();
		IMPORT_C void SetUpdateFeedTime(TTime aTime);
		
		IMPORT_C TInt SpecificIAP();
		IMPORT_C void SetSpecificIAP(TInt aIap);
		
		TInt Volume();
		void SetVolume(TInt aVolume);

		IMPORT_C TInt SeekStepTime();
		IMPORT_C void SetSeekStepTimek(TInt aSeekStepTime);

		IMPORT_C void SetSelectUnplayedOnly(TBool aOnlyUnplayed);
		IMPORT_C TBool SelectUnplayedOnly();

		IMPORT_C void SaveSettingsL();


	private:
		CSettingsEngine(CPodcastModel& aPodcastModel);
		void ConstructL();
		void ImportSettings();
		void LoadSettingsL();
		void GetDefaultBaseDirL(TDes &aBaseDir);


	private:
		// the settings we serialize
		TFileName iBaseDir;
		TInt iUpdateFeedInterval;
		TAutoUpdateSetting iUpdateAutomatically;
		TBool iDownloadAutomatically;
		TInt iMaxSimultaneousDownloads;
		TInt iIap;
		TInt iVolume;
		TInt iMaxListItems;
		TTime iUpdateFeedTime;
		TInt iSeekStepTime;

		// Other member variables
		RFs iFs;						// File system
		CPodcastModel &iPodcastModel; 	// reference to the model

	    TBool iSelectOnlyUnplayed;
	};

#endif /*SETTINGSENGINE_H_*/