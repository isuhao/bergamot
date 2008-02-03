#include <bautils.h>
#include <eikappui.h>
#include <eikapp.h>
#include <s32file.h>

#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "FeedEngine.h"

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
	iUpdateFeedInterval = 60;
	iMaxSimultaneousDownloads = 1;
	iDownloadAutomatically = EFalse;
	iUpdateAutomatically = EAutoUpdateOff;
	iIap = 0;
	iFs.Connect();
	
	GetDefaultBaseDir(iBaseDir);
	RDebug::Print(_L("Base dir: %S"), &iBaseDir);
	iDefaultFeedsFile.Copy(PrivatePath());
	iDefaultFeedsFile.Append(KDefaultFeedsFile);
	iMaxListItems = 100;
	
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);
	
	RDebug::Print(_L("Checking settings file: %S"), &configPath);
	if (BaflUtils::FileExists(iFs, configPath)) {
		TRAPD(error, LoadSettingsL());
		RDebug::Print(_L("LoadSettingsL returned error=%d"), error);
	} else {
		RDebug::Print(_L("Importing default settings"));
		ImportSettings();
		TRAPD(error,SaveSettingsL());
		if (error != KErrNone) {
			RDebug::Print(_L("error saving: %d"), error);
		}
	
	}

	BaflUtils::EnsurePathExistsL(iFs, iBaseDir);
	}

void CSettingsEngine::GetDefaultBaseDir(TDes &aBaseDir)
	{
	CDesCArray* disks = new(ELeave) CDesCArrayFlat(10);
	CleanupStack::PushL(disks);

	BaflUtils::GetDiskListL(iFs, *disks);
	
	#ifdef __WINS__
		aBaseDir.Copy(KInternalPodcastDir);
		CleanupStack::PopAndDestroy(disks);
		return;
	#endif

	// if only one drive, use C:\Media files\Podcasts
	if (disks->Count() == 1) 
		{
		aBaseDir.Copy(KInternalPodcastDir);
	// else we use the first flash drive
		} 
	else 
		{
		aBaseDir.Copy((*disks)[1]);
		aBaseDir.Append(_L(":"));
		aBaseDir.Append(KFlashPodcastDir);
	}
	
	CleanupStack::PopAndDestroy(disks);
	}

void CSettingsEngine::LoadSettingsL()
	{
	RDebug::Print(_L("LoadSettingsL"));
	//CDictionaryStore* iniFile = CEikonEnv::Static()->EikAppUi()->Application()->OpenIniFileLC(iFs);
	//RDictionaryReadStream stream;
	//stream.OpenLC(*iniFile, TUid::Uid(KSettingsUid));	
	
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);
	RDebug::Print(_L("Checking settings file: %S"), &configPath);
	if (!BaflUtils::FileExists(iFs, configPath)) {
		User::Leave(KErrNotFound);
	}
	
	CFileStore* store = CDirectFileStore::OpenL(iFs,configPath,EFileRead);
	CleanupStack::PushL(store);
	
	RStoreReadStream stream;
	stream.OpenLC(*store, store->Root());
	
	int len = stream.ReadInt32L();
	stream.ReadL(iBaseDir, len);
	iUpdateFeedInterval = stream.ReadInt32L();
	iUpdateAutomatically = (TAutoUpdateSetting) stream.ReadInt32L();
	iDownloadAutomatically = stream.ReadInt32L();

	iMaxSimultaneousDownloads = stream.ReadInt32L();
	iIap = stream.ReadInt32L();
	//iUpdateFeedTime = stream.ReadInt64L();

	iPodcastModel.SetIap(iIap);
	CleanupStack::PopAndDestroy(2); // readStream and iniFile
	}

void CSettingsEngine::SaveSettingsL()
	{
	RDebug::Print(_L("SaveSettingsL"));
	//CDictionaryStore* iniFile = CEikonEnv::Static()->EikAppUi()->Application()->OpenIniFileLC(iFs);
	//RDictionaryWriteStream stream;
	//stream.AssignLC(*iniFile, TUid::Uid(KSettingsUid));
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
	//stream.WriteInt64L(iUpdateFeedTime.Int64());
	stream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(2); // stream and store
	}

void CSettingsEngine::ImportSettings()
	{
	RDebug::Print(_L("ImportSettings"));
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigImportFile);
	
	BaflUtils::EnsurePathExistsL(iFs, configPath);
	RDebug::Print(_L("Importing settings from %S"), &configPath);
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
			RDebug::Print(_L("line: %S, tag: '%S', value: '%S'"), &line, &tag, &value);
			if (tag.CompareF(_L("BaseDir")) == 0) {
				iBaseDir.Copy(value);
			} else if (tag.CompareF(_L("UpdateFeedIntervalMinutes")) == 0) {
				TLex lex(value);
				lex.Val(iUpdateFeedInterval);
				RDebug::Print(_L("Updating automatically every %d minutes"), iUpdateFeedInterval);
			} else if (tag.CompareF(_L("DownloadAutomatically")) == 0) {
				TLex lex(value);
				lex.Val((TInt &) iDownloadAutomatically);
				RDebug::Print(_L("Download automatically: %d"), iDownloadAutomatically);
			} else if (tag.CompareF(_L("MaxSimultaneousDownloads")) == 0) {
				TLex lex(value);
				lex.Val(iMaxSimultaneousDownloads);
				RDebug::Print(_L("Max simultaneous downloads: %d"), iMaxSimultaneousDownloads);
			} else if (tag.CompareF(_L("MaxShowsPerFeed")) == 0) {
				TLex lex(value);
				lex.Val(iMaxListItems);
				RDebug::Print(_L("Max shows per feed: %d"), iMaxListItems);
			}
		}
		
		error = tft.Read(line);
		}
	
	rfile.Close();
	}

TFileName& CSettingsEngine::BaseDir()
	{
	return iBaseDir;
	}

TInt CSettingsEngine::UpdateFeedInterval() 
	{
	return iUpdateFeedInterval;
	}

TInt CSettingsEngine::MaxSimultaneousDownloads() 
	{
	return iMaxSimultaneousDownloads;
	}

TAutoUpdateSetting CSettingsEngine::UpdateAutomatically() 
	{
	return iUpdateAutomatically;
	}

TBool CSettingsEngine::DownloadAutomatically() 
	{
	return iDownloadAutomatically;
	}

TTime CSettingsEngine::UpdateFeedTime()
	{
	return iUpdateFeedTime;
	}

TInt CSettingsEngine::MaxListItems() 
	{
	return iMaxListItems;
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

void CSettingsEngine::SetBaseDir(TFileName& aFileName)
	{
	int len = aFileName.Length();
	if (len > 0) {
		if (aFileName[len-1] != '\\') {
			aFileName.Append(_L("\\"));
		}
	}
	iBaseDir = aFileName;

	}

void CSettingsEngine::SetUpdateFeedInterval(TInt aInterval)
	{
	iUpdateFeedInterval = aInterval;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

void CSettingsEngine::SetMaxSimultaneousDownloads(TInt aMaxDownloads)
	{
	iMaxSimultaneousDownloads = aMaxDownloads;
	}

void CSettingsEngine::SetUpdateAutomatically(TAutoUpdateSetting aAutoSetting)
	{
	iUpdateAutomatically = aAutoSetting;
	}

void CSettingsEngine::SetDownloadAutomatically(TBool aDownloadAuto)
	{
	iDownloadAutomatically = aDownloadAuto;
	}

void CSettingsEngine::SetUpdateFeedTime(TTime aUpdateTime)
	{
	iUpdateFeedTime = aUpdateTime;
	}


void CSettingsEngine::SetSpecificIAP(TInt aIap)
	{
	RDebug::Print(_L("SetSpecificIAP: %d"), aIap);
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
