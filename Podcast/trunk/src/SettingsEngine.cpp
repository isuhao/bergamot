#include "SettingsEngine.h"
#include <bautils.h>

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
	iFeedListFile.Copy(KFeedsFileName);
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
			RDebug::Print(_L("line: %S, tag: '%S', value: '%S'"), &line, &tag, &value);
			if (tag.CompareF(_L("PodcastDir")) == 0) {
				iShowDir.Copy(value);
			} else if (tag.CompareF(_L("FeedList")) == 0) {
				iFeedListFile.Copy(value);
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

