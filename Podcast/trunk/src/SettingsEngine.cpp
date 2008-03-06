#include <bautils.h>
#include <s32file.h>

#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "FeedEngine.h"

CSettingsEngine::CSettingsEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
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
	iVolume = KMaxVolume;
	iUpdateFeedInterval = KDefaultUpdateFeedInterval;
	iMaxSimultaneousDownloads = KDefaultMaxSimultaneousDownloads;
	iDownloadAutomatically = EFalse;
	iUpdateAutomatically = EAutoUpdateOff;
	iMaxListItems = KDefaultMaxListItems;
	iIap = 0;
	
	// Connect to file system		
	iFs.Connect();
	
	// Check that our basedir exist. Create it otherwise;
	GetDefaultBaseDirL(iBaseDir);
	RDebug::Print(_L("Base dir: %S"), &iBaseDir);
	BaflUtils::EnsurePathExistsL(iFs, iBaseDir);
	
	// load settings
	TRAPD(loadErr, LoadSettingsL());
	if (loadErr != KErrNone)
		{
		RDebug::Print(_L("CSettingsEngine::ConstructL\tLoadSettingsL returned error=%d"), loadErr);
		RDebug::Print(_L("CSettingsEngine::ConstructL\tImporting default settings instead"));
	
		ImportSettings();
		TRAPD(error,SaveSettingsL());
		if (error != KErrNone) 
			{
			RDebug::Print(_L("error saving: %d"), error);
			}
		}		
	}

void CSettingsEngine::GetDefaultBaseDirL(TDes &aBaseDir)
	{
	CDesCArray* disks = new(ELeave) CDesCArrayFlat(10);
	CleanupStack::PushL(disks);

	BaflUtils::GetDiskListL(iFs, *disks);
	
	#ifdef __WINS__
		aBaseDir.Copy(KInternalPodcastDir);
		CleanupStack::PopAndDestroy(disks);
		return;
	#endif

	if (disks->Count() == 1)  // if only one drive, use C:\Media files\Podcasts
		{
		aBaseDir.Copy(KInternalPodcastDir);
		} 
	else // else we use the first flash drive
		{
		aBaseDir.Copy((*disks)[1]);
		aBaseDir.Append(_L(":"));
		aBaseDir.Append(KFlashPodcastDir);
		}
	CleanupStack::PopAndDestroy(disks);
	}

void CSettingsEngine::LoadSettingsL()
	{
	RDebug::Print(_L("CSettingsEngine::LoadSettingsL\t Trying to load settings"));
	
	// Create path for the config file
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);

	RDebug::Print(_L("Checking settings file: %S"), &configPath);
	if (!BaflUtils::FileExists(iFs, configPath)) 
		{
		User::Leave(KErrNotFound);
		}
	
	CFileStore* store = CDirectFileStore::OpenL(iFs,configPath,EFileRead);
	CleanupStack::PushL(store);
	
	RStoreReadStream stream;
	stream.OpenLC(*store, store->Root());
	
	TInt len = stream.ReadInt32L();
	stream.ReadL(iBaseDir, len);
	iUpdateFeedInterval = stream.ReadInt32L();
	iUpdateAutomatically = static_cast<TAutoUpdateSetting>(stream.ReadInt32L());
	iDownloadAutomatically = stream.ReadInt32L();

	iMaxSimultaneousDownloads = stream.ReadInt32L();
	iIap = stream.ReadInt32L();
	iPodcastModel.SetIap(iIap);
	
	TInt low = stream.ReadInt32L();
	TInt high = stream.ReadInt32L();
	iUpdateFeedTime = MAKE_TINT64(high, low);

	RDebug::Print(_L("CSettingsEngine::LoadSettingsL\t Settings loaded OK"));
	CleanupStack::PopAndDestroy(2); // readStream and iniFile
	}

void CSettingsEngine::SaveSettingsL()
	{
	RDebug::Print(_L("CSettingsEngine::SaveSettingsL\tTrying to save settings"));

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);
	
	CFileStore* store = CDirectFileStore::ReplaceLC(iFs, configPath, EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);

	RStoreWriteStream stream;
	TStreamId id = stream.CreateLC(*store);
		
	stream.WriteInt32L(iBaseDir.Length());
	stream.WriteL(iBaseDir);
	stream.WriteInt32L(iUpdateFeedInterval);
	stream.WriteInt32L(iUpdateAutomatically);
	stream.WriteInt32L(iDownloadAutomatically);
	stream.WriteInt32L(iMaxSimultaneousDownloads);
	stream.WriteInt32L(iIap);
	
	stream.WriteInt32L(I64LOW(iUpdateFeedTime.Int64()));
	stream.WriteInt32L(I64HIGH(iUpdateFeedTime.Int64()));

	stream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(2); // stream and store
	}

void CSettingsEngine::ImportSettings()
	{
	RDebug::Print(_L("CSettingsEngine::ImportSettings"));

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigImportFile);
	
	RDebug::Print(_L("Importing settings from %S"), &configPath);
	
	RFile rfile;
	TInt error = rfile.Open(iFs, configPath,  EFileRead);
	if (error != KErrNone) 
		{
		RDebug::Print(_L("CSettingsEngine::ImportSettings()\tFailed to read settings"));
		return;
		}
	
	TFileText tft;
	tft.Set(rfile);
	
	TBuf<1024> line;
	error = tft.Read(line);
	
	while (error == KErrNone) 
		{
		if (line.Locate('#') == 0) 
			{
			error = tft.Read(line);
			continue;
			}
		
		TInt equalsPos = line.Locate('=');
		if (equalsPos != KErrNotFound) 
			{
			TPtrC tag = line.Left(equalsPos);
			TPtrC value = line.Mid(equalsPos+1);
			RDebug::Print(_L("line: %S, tag: '%S', value: '%S'"), &line, &tag, &value);
			if (tag.CompareF(_L("BaseDir")) == 0) 
				{
				iBaseDir.Copy(value);
				} 
			else if (tag.CompareF(_L("UpdateFeedIntervalMinutes")) == 0) 
				{
				TLex lex(value);
				lex.Val(iUpdateFeedInterval);
				RDebug::Print(_L("Updating automatically every %d minutes"), iUpdateFeedInterval);
				} 
			else if (tag.CompareF(_L("DownloadAutomatically")) == 0) 
				{
				TLex lex(value);
				lex.Val((TInt &) iDownloadAutomatically);
				RDebug::Print(_L("Download automatically: %d"), iDownloadAutomatically);
				} 
			else if (tag.CompareF(_L("MaxSimultaneousDownloads")) == 0) 
				{
				TLex lex(value);
				lex.Val(iMaxSimultaneousDownloads);
				RDebug::Print(_L("Max simultaneous downloads: %d"), iMaxSimultaneousDownloads);
				}
			else if (tag.CompareF(_L("MaxShowsPerFeed")) == 0) 
				{
				TLex lex(value);
				lex.Val(iMaxListItems);
				RDebug::Print(_L("Max shows per feed: %d"), iMaxListItems);
				}
			}
		
		error = tft.Read(line);
		}
	rfile.Close();
	}

TFileName CSettingsEngine::DefaultFeedsFileName()
	{
	TFileName defaultFeeds;
	defaultFeeds.Append(PrivatePath());
	defaultFeeds.Append(KDefaultFeedsFile);
	return defaultFeeds;
	}

TFileName CSettingsEngine::ImportFeedsFileName()
	{
	TFileName importFeeds;
	importFeeds.Append(BaseDir());
	importFeeds.Append(KImportFeedsFile);
	return importFeeds;
	}


TFileName CSettingsEngine::PrivatePath()
	{
	TFileName privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	return privatePath;
	}

TInt CSettingsEngine::MaxListItems() 
	{
	return iMaxListItems;
	}

TFileName& CSettingsEngine::BaseDir()
	{
	return iBaseDir;
	}

void CSettingsEngine::SetBaseDir(TFileName& aFileName)
	{
	TInt length = aFileName.Length();
	if (length > 0) 
		{
		if (aFileName[length-1] != '\\') 
			{
			aFileName.Append(_L("\\"));
			}
		}
	iBaseDir = aFileName;
	}

TInt CSettingsEngine::UpdateFeedInterval() 
	{
	return iUpdateFeedInterval;
	}

void CSettingsEngine::SetUpdateFeedInterval(TInt aInterval)
	{
	iUpdateFeedInterval = aInterval;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

TInt CSettingsEngine::MaxSimultaneousDownloads() 
	{
	return iMaxSimultaneousDownloads;
	}

void CSettingsEngine::SetMaxSimultaneousDownloads(TInt aMaxDownloads)
	{
	iMaxSimultaneousDownloads = aMaxDownloads;
	}

TAutoUpdateSetting CSettingsEngine::UpdateAutomatically() 
	{
	return iUpdateAutomatically;
	}

void CSettingsEngine::SetUpdateAutomatically(TAutoUpdateSetting aAutoSetting)
	{
	iUpdateAutomatically = aAutoSetting;
	}

TBool CSettingsEngine::DownloadAutomatically() 
	{
	return iDownloadAutomatically;
	}

void CSettingsEngine::SetDownloadAutomatically(TBool aDownloadAuto)
	{
	iDownloadAutomatically = aDownloadAuto;
	}

TTime CSettingsEngine::UpdateFeedTime()
	{
	return iUpdateFeedTime;
	}

void CSettingsEngine::SetUpdateFeedTime(TTime aUpdateTime)
	{
	iUpdateFeedTime = aUpdateTime;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

TInt CSettingsEngine::SpecificIAP()
	{
	return iIap;
	}

void CSettingsEngine::SetSpecificIAP(TInt aIap)
	{
	iIap = aIap;
	}

TInt CSettingsEngine::Volume()
	{
	return iVolume;
	}

void CSettingsEngine::SetVolume(TInt aVolume)
	{
	iVolume = aVolume;
	if(&iPodcastModel.SoundEngine() != NULL) 
		{
		iPodcastModel.SoundEngine().SetVolume(iVolume);
		}
	}
