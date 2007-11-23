#include <bautils.h>

#include "SettingsEngine.h"
#include "SoundEngine.h"

const TInt KMaxVolume = 100;
CSettingsEngine::CSettingsEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
{
	iVolume = KMaxVolume;
}

CSettingsEngine::~CSettingsEngine()
{
	iFs.Close();
}

CSettingsEngine* CSettingsEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CSettingsEngine* self = new (ELeave) CSettingsEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CSettingsEngine::ConstructL()
	{
	// default values
	iFeedListFile.Copy(KFeedsFileName);
	iUpdateFeedInterval = 60;
	iMaxSimultaneousDownloads = 1;
	iDownloadAutomatically = EFalse;
	iDownloadOnlyOnWLAN = EFalse;
	iShowDir.Copy(KPodcastDir);
	iDefaultFeedsFile.Copy(PrivatePath());
	iDefaultFeedsFile.Append(KDefaultFeedsFile);
	iFs.Connect();
	LoadSettings();
	}

void CSettingsEngine::LoadSettings()
	{
	TBuf<100> configPath;
	iFs.PrivatePath(configPath);
	configPath.Append(KConfigFile);
	
	BaflUtils::EnsurePathExistsL(iFs, configPath);
	RDebug::Print(_L("Reading config from %S"), &configPath);
	RFile rfile;
	int error = rfile.Open(iFs, configPath,  EFileRead);
	
	if (error != KErrNone) {
		RDebug::Print(_L("Failed to read settings"));
		return;
	}
	
	TFileText tft;
	tft.Set(rfile);
	
	TBuf<1024> line;
	error = tft.Read(line);
	
	while (error == KErrNone) {
		if (line.Locate('#') == 0) {
			error = tft.Read(line);
			continue;
		}
		
		int equalsPos = line.Locate('=');
		if (equalsPos != KErrNotFound) {
			TPtrC tag = line.Left(equalsPos);
			TPtrC value = line.Mid(equalsPos+1);
			//RDebug::Print(_L("line: %S, tag: '%S', value: '%S'"), &line, &tag, &value);
			if (tag.CompareF(_L("PodcastDir")) == 0) {
				iShowDir.Copy(value);
			} else if (tag.CompareF(_L("FeedList")) == 0) {
				iFeedListFile.Copy(value);
			} else if (tag.CompareF(_L("UpdateFeedIntervalMinutes")) == 0) {
				TLex lex(value);
				lex.Val(iUpdateFeedInterval);
				RDebug::Print(_L("Updating automatically every %d minutes"), iUpdateFeedInterval);
			} else if (tag.CompareF(_L("DownloadAutomatically")) == 0) {
				TLex lex(value);
				lex.Val(iDownloadAutomatically);
				RDebug::Print(_L("Download automatically: %d"), iDownloadAutomatically);
			} else if (tag.CompareF(_L("MaxSimultaneousDownloads")) == 0) {
				TLex lex(value);
				lex.Val(iMaxSimultaneousDownloads);
				RDebug::Print(_L("Max simultaneous downloads: %d"), iMaxSimultaneousDownloads);
			} else if (tag.CompareF(_L("DownloadOnlyOnWLAN")) == 0) {
				TLex lex(value);
				lex.Val(iDownloadOnlyOnWLAN);
				RDebug::Print(_L("Download only on WLAN: %d"), iDownloadOnlyOnWLAN);
			}
		}
		
		error = tft.Read(line);
		}
	
	rfile.Close();
	}

TFileName& CSettingsEngine::ShowDir()
	{
	return iShowDir;
	}

TFileName& CSettingsEngine::FeedListFile() 
	{
	return iFeedListFile;
	}

TInt CSettingsEngine::UpdateFeedInterval() 
	{
	return iUpdateFeedInterval;
	}

TInt CSettingsEngine::MaxSimultaneousDownloads() 
	{
	return iMaxSimultaneousDownloads;
	}

TBool CSettingsEngine::DownloadAutomatically() 
	{
	return iDownloadAutomatically;
	}

TBool CSettingsEngine::DownloadOnlyOnWLAN() 
	{
	return iDownloadOnlyOnWLAN;
	}

TFileName CSettingsEngine::PrivatePath()
	{
	TFileName privatePath;
	RFs rfs;
	rfs.Connect();
	rfs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(rfs, privatePath);
	rfs.Close();
	return privatePath;
	}

TFileName& CSettingsEngine::DefaultFeedsFileName()
{
	return iDefaultFeedsFile;
}

TInt CSettingsEngine::SpecificIAP()
	{
	return iIap;
	}


void CSettingsEngine::SetShowDir(TFileName& aFileName)
	{
	iShowDir = aFileName;
	}

void CSettingsEngine::SetUpdateFeedInterval(TInt aInterval)
	{
	iUpdateFeedInterval = aInterval;
	}

void CSettingsEngine::SetMaxSimultaneousDownloads(TInt aMaxDownloads)
	{
	iMaxSimultaneousDownloads = aMaxDownloads;
	}

void CSettingsEngine::SetDownloadAutomatically(TBool aAutoOn)
	{
	iDownloadAutomatically = aAutoOn;
	}

void CSettingsEngine::SetDownloadOnlyOnWLAN(TBool aOnlyOnWLAN)
	{
	iDownloadOnlyOnWLAN = aOnlyOnWLAN;
	}

void CSettingsEngine::SetSpecificIAP(TInt aIap)
	{
	iIap = aIap;
	}


TUint CSettingsEngine::Volume()
{
	return iVolume;
}

void CSettingsEngine::SetVolume(TUint aVolume)
{
	iVolume = aVolume;
	if(&iPodcastModel.SoundEngine() != NULL) {
		iPodcastModel.SoundEngine().SetVolume(iVolume);
	}
}

