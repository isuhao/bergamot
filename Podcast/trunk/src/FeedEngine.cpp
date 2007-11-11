#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>

CFeedEngine::CFeedEngine() : parser(*this)
	{
	iClient = CHttpClient::NewL(*this);
	iFeedListFile.Copy(KFeedsFileName);
	}

CFeedEngine::~CFeedEngine()
	{
	}

void CFeedEngine::Cancel() 
	{
	}
void CFeedEngine::GetFeed(TFeedInfo* feedInfo)
	{
	items.Reset();
	TBuf<100> privatePath;
	RFs rfs;
	rfs.Connect();
	rfs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(rfs, privatePath);
	
	int pos = feedInfo->url.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedInfo->url.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}
	//iFileName.Copy(privatePath);
	feedInfo->fileName.Copy(privatePath);
	iClient->GetFeed(feedInfo);

	RDebug::Print(_L("DownloadFeed END"));
	}

RArray <TShowInfo*>& CFeedEngine::GetItems() 
{
	return items;
}

RArray <TFeedInfo*>& CFeedEngine::GetFeeds() 
{
	return feeds;
}
void CFeedEngine::GetShow(TShowInfo *info)
	{
	TBuf<100> filePath;
	RFs rfs;
	rfs.Connect();
	filePath.Copy(iShowDir);
	BaflUtils::EnsurePathExistsL(rfs, filePath);
	
	int pos = info->url.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = info->url.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown"));
	}
	info->state = EDownloading;
	info->fileName.Copy(filePath);
	iClient->GetShow(info);
	}

void CFeedEngine::Item(TShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription: %S"), &(item->title), &(item->url), &(item->description));
	items.Append(item);
	}

void CFeedEngine::ConnectedCallback()
	{
	
	}

void CFeedEngine::ProgressCallback(int percent)
	{
	TBuf<100> printBuffer;
	printBuffer.Format(_L("Downloading... %d%%"), percent);
	User::InfoPrint(printBuffer);
	
	}

void CFeedEngine::ShowCompleteCallback(TShowInfo *info)
	{
	RDebug::Print(_L("File %S complete"), &info->fileName);
	TInt pos = downloadQueue.Find(info);
	
	if (pos != KErrNotFound) {
		RDebug::Print(_L("Removing from list..."));
		downloadQueue.Remove(pos);
	}
	DownloadNextShow();

	}

void CFeedEngine::FeedCompleteCallback(TFeedInfo *info)
	{
	RDebug::Print(_L("File to parse: %S"), &info->fileName);
	parser.ParseFeedL(info->fileName);
	
	}

void CFeedEngine::DisconnectedCallback()
	{
	
	}

void CFeedEngine::DownloadInfoCallback(int size)
	{
	RDebug::Print(_L("About to download %d bytes"), size);
	}

void CFeedEngine::LoadSettings()
	{
	RFs rfs;
	rfs.Connect();
	
	TBuf<100> configPath;
	rfs.PrivatePath(configPath);
	configPath.Append(KConfigFile);
	
	BaflUtils::EnsurePathExistsL(rfs, configPath);
	RDebug::Print(_L("Reading config from %S"), &configPath);
	RFile rfile;
	int error = rfile.Open(rfs, configPath,  EFileRead);
	
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
	
	}

void CFeedEngine::LoadFeeds()
	{
	RFs rfs;
	rfs.Connect();
	
	TBuf<100> configPath;
	configPath.Copy(iFeedListFile);
	RDebug::Print(_L("Reading feeds from %S"), &configPath);
	RFile rfile;
	int error = rfile.Open(rfs, configPath,  EFileRead);
	
	if (error != KErrNone) {
		RDebug::Print(_L("Failed to read feeds"));
		return;
	}

	
	TFileText tft;
	tft.Set(rfile);
	
	TBuf<1024> line;
	error = tft.Read(line);
	
	while (error == KErrNone) {
		RDebug::Print(_L("Line: %S"), &line);
		TFeedInfo *fi = new TFeedInfo;
		int pos = line.Locate('|');
		if (pos == KErrNotFound) {
			fi->title.Copy(line);
			fi->url.Copy(line);
		}else {
			fi->title.Copy(line.Left(pos));
			fi->url.Copy(line.Mid(pos+1));
			RDebug::Print(_L("Read title: '%S', url: '%S'"), &fi->title, &fi->url);
		}
		feeds.Append(fi);
		 error = tft.Read(line);
		}
	}
	
void CFeedEngine::SaveFeeds()
	{
	
	}


void CFeedEngine::ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files) {
	CDirScan *dirScan = CDirScan::NewLC(rfs);
	RDebug::Print(_L("Listing dir: %S"), &folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);
	
	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (int i=0;i<dirPtr->Count();i++) {
		const TEntry &entry = (*dirPtr)[i];
		if (entry.IsDir())  {
			/*TBuf<100> subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDir(rfs, subFolder, array);*/
		} else {
		if (entry.iName.Right(3).CompareF(_L("mp3")) == 0 ||
				entry.iName.Right(3).CompareF(_L("aac")) == 0 ||
				entry.iName.Right(3).CompareF(_L("wav")) == 0) {
			RDebug::Print(entry.iName);
			int id = files.Count();
			TBuf<100> fileName;
			fileName.Copy(folder);
			fileName.Append(entry.iName);
			TShowInfo *pID = new TShowInfo;
			pID->fileName.Copy(fileName);
			pID->title.Copy(entry.iName);
			files.Append(pID);
		}
		}
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

TShowInfoArray& CFeedEngine::GetPodcasts()
{
	return files;
}
void CFeedEngine::ListAllPodcasts()
	{
	RFs rfs;
	rfs.Connect();
	files.Reset();
	TBuf<100> podcastDir;
	podcastDir.Copy(KPodcastDir);

	ListDir(rfs, podcastDir, files);	
}

void CFeedEngine::AddDownload(TShowInfo *info)
	{
	downloadQueue.Append(info);

	if (downloadQueue.Count() == 1) {
		DownloadNextShow();
	}
	}

TShowInfoArray& CFeedEngine::GetDownloads()
	{
	return downloadQueue;
	}

void CFeedEngine::DownloadNextShow()
{
	if(downloadQueue.Count() > 0) {
		TShowInfo *info = downloadQueue[0];
		RDebug::Print(_L("Downloading %S"), &(info->title));
		GetShow(info);
	}
}