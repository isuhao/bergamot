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
	iFeedListFile.Copy(KFeedsImportFileName);
	iUpdateFeedInterval = 60;
	iMaxSimultaneousDownloads = 1;
	iDownloadAutomatically = EFalse;
	iDownloadOnlyOnWLAN = EFalse;
	iShowDir.Copy(KPodcastDir);
	iDefaultFeedsFile.Copy(PrivatePath());
	iDefaultFeedsFile.Append(KDefaultFeedsFile);
	iMaxListItems = 100;
	iFs.Connect();
	TRAPD(error, LoadSettingsL());
	if(error != KErrNone) {
		RDebug::Print(_L("Importing settings"));
		ImportSettings();
		TRAP(error,SaveSettingsL());
		if (error != KErrNone) {
			RDebug::Print(_L("error saving: %d"), error);
			}
	}
	}

void CSettingsEngine::LoadSettingsL()
	{
	RDebug::Print(_L("SaveSettingsL"));
	//CDictionaryStore* iniFile = CEikonEnv::Static()->EikAppUi()->Application()->OpenIniFileLC(iFs);
	//RDictionaryReadStream stream;
	//stream.OpenLC(*iniFile, TUid::Uid(KSettingsUid));	
	
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);
	
	if (!BaflUtils::FileExists(iFs, configPath)) {
		User::Leave(KErrNotFound);
	}
	
	CFileStore* store;
	store = CDirectFileStore::OpenL(iFs,configPath,EFileRead);
	CleanupStack::PushL(store);
	
	RStoreReadStream stream;
	stream.OpenLC(*store, store->Root());
	
	int len = stream.ReadInt32L();
	stream.ReadL(iShowDir, len);
	len = stream.ReadInt32L();
	stream.ReadL(iFeedListFile, len);
	iUpdateFeedInterval = stream.ReadInt32L();
	iDownloadAutomatically = stream.ReadInt32L();
	iDownloadOnlyOnWLAN = stream.ReadInt32L();
	iMaxSimultaneousDownloads = stream.ReadInt32L();
	iIap = stream.ReadInt32L();
		
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
		
	stream.WriteInt32L(iShowDir.Length());
	stream.WriteL(iShowDir);
	stream.WriteInt32L(iFeedListFile.Length());
	stream.WriteL(iFeedListFile);
	stream.WriteInt32L(iUpdateFeedInterval);
	stream.WriteInt32L(iDownloadAutomatically);
	stream.WriteInt32L(iDownloadOnlyOnWLAN);
	stream.WriteInt32L(iMaxSimultaneousDownloads);
	stream.WriteInt32L(iIap);
	
	stream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(2); // stream and store
	}

void CSettingsEngine::ImportSettings()
	{
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigImportFile);
	
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
			RDebug::Print(_L("line: %S, tag: '%S', value: '%S'"), &line, &tag, &value);
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


void CSettingsEngine::SetShowDir(TFileName& aFileName)
	{
	iShowDir = aFileName;
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

