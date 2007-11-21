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
	TBool DownloadOnlyOnWLAN();		
	
private:
	TFileName iShowDir;
	TFileName iFeedListFile;
	TInt iUpdateFeedInterval;
	TBool iDownloadAutomatically;
	TBool iDownloadOnlyOnWLAN;
	TInt iMaxSimultaneousDownloads;
	
	// the file session used to read and write settings
	RFs iFs;
	
	CPodcastModel &iPodcastModel;
	
};

#endif /*SETTINGSENGINE_H_*/
