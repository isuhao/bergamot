#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <S32FILE.h>

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
	CleanHtml(item->description);
	
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
	info->state = EStateless;
	if (pos != KErrNotFound) {
		RDebug::Print(_L("Removing from list..."));
		downloadQueue.Remove(pos);
	}
	SaveMetaData();
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


void CFeedEngine::LoadMetaData()
	{
	RDebug::Print(_L("LoadMetaData"));
	RFs fsSession;
	int error = fsSession.Connect(); 
	RDebug::Print(_L("RFs error: %d"), error);
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	privatePath.Append(KMetaDataFile);
	fsSession.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(fsSession, privatePath)) {
		RDebug::Print(_L("No metadata file"));	
		return;
	}
	
	CFileStore* store;
	TRAP(error, store = CDirectFileStore::OpenL(fsSession,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		fsSession.Close();
		CleanupStack::Pop(store);
		return;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	for (int i=0;i<count;i++) {
		TShowInfo *readData = new TShowInfo;
		instream  >> *readData;	
		files.Append(readData);
	}
	RDebug::Print(_L("Read all metadata"));
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);	
	}

void CFeedEngine::SaveMetaData()
	{
	RDebug::Print(_L("SaveMetaData"));
	RFs fsSession;
	fsSession.Connect(); 
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	privatePath.Append(KMetaDataFile);
	
	RDebug::Print(_L("File: %S"), &privatePath);
	fsSession.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(fsSession, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	outstream.WriteInt32L(files.Count());
	for (int i=0;i<files.Count();i++) {
		RDebug::Print(_L("Storing account %i"), i);
		outstream  << files[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	fsSession.Close();
	
	CleanupStack::Pop(store);	
	
	}


void CFeedEngine::ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files) {
	CDirScan *dirScan = CDirScan::NewLC(rfs);
	RDebug::Print(_L("Listing dir: %S"), &folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);
	
	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (int i=0;i<dirPtr->Count();i++) {
		const TEntry &entry = (TEntry) (*dirPtr)[i];
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
			TBool exists = EFalse;
			TFileName fileName;
			fileName.Copy(folder);
			fileName.Append(entry.iName);

			for (int i=0;i<files.Count();i++) {
				if (files[i]->fileName.Compare(fileName) == 0) {
					RDebug::Print(_L("Already listed %S"), &fileName);
					exists = ETrue;
					break;
				}
			}
			
			if (exists) {
				continue;
			}
			
			RDebug::Print(entry.iName);
			TShowInfo *pID = new TShowInfo;
			pID->fileName.Copy(fileName);
			pID->title.Copy(entry.iName);
			pID->playing = EFalse;
			pID->position = 0;
			pID->state = EStateless;
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
//	files.Reset();
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
		info->state = EDownloading;
		GetShow(info);
	}
}


void CFeedEngine::CleanHtml(TDes &str)
{
	RDebug::Print(_L("CleanHtml %d, %S"), str.Length(), &str);
	int startPos = str.Locate('<');
	int endPos = str.Locate('>');
	RDebug::Print(_L("startPos: %d, endPos: %d"), startPos, endPos);
	if (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		RDebug::Print(_L("Cleaning out %S"), &str.Mid(startPos, endPos-startPos+1));
		TBuf<1000> tmp;
		tmp.Copy(str.Left(startPos));
		tmp.Append(str.Mid(endPos+1));
		str = tmp;
		CleanHtml(str);
	}
	
	
}
