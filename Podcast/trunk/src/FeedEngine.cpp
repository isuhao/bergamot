#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <e32hashtab.h>

CFeedEngine* CFeedEngine::NewL()
	{
	CFeedEngine* self = new (ELeave) CFeedEngine;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CFeedEngine::ConstructL()
	{
	iParser = new (ELeave) CFeedParser(*this);
	LoadSettings();
    LoadFeeds();
    LoadMetaData();
	}

CFeedEngine::CFeedEngine()
	{
	showClient = CHttpClient::NewL(*this);
	feedClient = CHttpClient::NewL(*this);
	iDownloading = EFalse;
	iFeedListFile.Copy(KFeedsFileName);
	}

CFeedEngine::~CFeedEngine()
	{
	shows.Close();
	feeds.Close();
	delete iParser;
	}

void CFeedEngine::Cancel() 
	{
	}

void CFeedEngine::GetFeed(TFeedInfo* feedInfo)
	{
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

	feedInfo->fileName.Copy(privatePath);
	
	feedClient->GetFeed(feedInfo);
	feedInfo->iUid = DefaultHash::Des16(feedInfo->url);
	SaveMetaData();
	RDebug::Print(_L("GetFeed END"));
	}

void CFeedEngine::GetShow(TShowInfo *info)
	{
	TBuf<100> filePath;
	RFs rfs;
	rfs.Connect();
	filePath.Copy(iShowDir);
	if (filePath[filePath.Length()-1] != '\\') {
		RDebug::Print(_L("%S has no terminating backslash"), &iShowDir);
		filePath.Append(_L("\\"));
	}
	// fix filename
	TBuf<1024> buf;
	buf.Copy(info->feedUrl);
	ReplaceString(buf, _L("/"), _L("_"));
	ReplaceString(buf, _L(":"), _L("_"));
	ReplaceString(buf, _L("?"), _L("_"));
	buf.Trim();
	filePath.Append(buf);
	filePath.Append(_L("\\"));
	RDebug::Print(_L("directory to store in: %S"), &filePath);
	BaflUtils::EnsurePathExistsL(rfs, filePath);

	int pos = info->url.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = info->url.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown.mp3"));
	}
	RDebug::Print(_L("filePath: %S"), &filePath);
	info->fileName.Copy(filePath);
	iDownloading = ETrue;
	showClient->GetShow(info);
	}

void CFeedEngine::ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement )
	{
		TInt pos=aString.Find(aStringToReplace);
		while (pos>=0)
		{
			aString.Replace(pos,aStringToReplace.Length(),aReplacement);
			pos=aString.Find(aStringToReplace);
		}
		
	}

void CFeedEngine::Item(TShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %S"), &(item->title), &(item->url), item->description.Length(), &(item->feedUrl));
	CleanHtml(item->description);
	item->uid = DefaultHash::Des16(item->url);
	AddShow(item);
	}

void CFeedEngine::AddShow(TShowInfo *item) {
	for (int i=0;i<shows.Count();i++) {
		if (shows[i]->url.Compare(item->url) == 0) {
			return;
		}
	}
	shows.Append(item);
}

void CFeedEngine::ParsingComplete()
	{
	RDebug::Print(_L("Triggering ShowListUpdated"));
	for (int i=0;i<observers.Count();i++) {
		observers[i]->FeedInfoUpdated(iParser->ActiveFeed());
		observers[i]->ShowListUpdated();
	}
	}

void CFeedEngine::ConnectedCallback()
	{
	
	}

void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	observers.Append(observer);
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
	info->downloadState = EDownloaded;
	SaveMetaData();
	iDownloading = EFalse;
	DownloadNextShow();
	}

void CFeedEngine::FeedCompleteCallback(TFeedInfo * aInfo)
	{
	RDebug::Print(_L("File to parse: %S"), &aInfo->fileName);
	iParser->ParseFeedL(aInfo->fileName, aInfo);
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
	
	rfile.Close();
	rfs.Close();
	
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
		line.Trim();
		fi->iUid = KErrNotFound;
		fi->url.Copy(line);
		int pos = line.Locate('|');
		if (pos == KErrNotFound) {
			fi->title.Copy(line);
			fi->url.Copy(line);
		}else {
			fi->title.Copy(line.Left(pos));
			fi->url.Copy(line.Mid(pos+1));
			RDebug::Print(_L("Read title: '%S', url: '%S'"), &fi->title, &fi->url);
		}
		
		fi->title.Trim();
		fi->url.Trim();
		fi->iUid = DefaultHash::Des16(fi->url);

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

	int version = instream.ReadInt32L();
	RDebug::Print(_L("Read version: %d"), version);

	if (version != KShowInfoVersion) {
		RDebug::Print(_L("Wrong version, discarding"));
		goto exit_point;
	}
	
	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	TShowInfo *readData;
	
	for (int i=0;i<count;i++) {
		readData = new TShowInfo;
		TRAP(error, instream  >> *readData);
		//RDebug::Print(_L("error: %d"), error);
		AddShow(readData);
	}
	exit_point:
	CleanupStack::PopAndDestroy(); // instream
	
	// check which shows have been downloaded
	/*for (int i=0;i<shows.Count();i++) {
		if (shows[i]->fileName.Length() > 0 && BaflUtils::FileExists(fsSession,shows[i]->fileName)) {
			RDebug::Print(_L("File %S exists"), &(shows[i]->fileName));
			shows[i]->iShowDownloaded = ETrue;
		} else {
			shows[i]->iShowDownloaded = EFalse;
		}
	}*/
	
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
	outstream.WriteInt32L(KShowInfoVersion);
	RDebug::Print(_L("Saving %d items"), shows.Count());
	outstream.WriteInt32L(shows.Count());
	for (int i=0;i<shows.Count();i++) {
//		RDebug::Print(_L("Storing show %i"), i);
		outstream  << *shows[i];
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
			
			/*RDebug::Print(entry.iName);
			TShowInfo *pID = new TShowInfo;
			pID->fileName.Copy(fileName);
			pID->title.Copy(entry.iName);		
			pID->position = 0;
			pID->state = EStateless;
			pID->iShowDownloaded = ETrue;
			//LoadMetaDataFromFile(pID);
			files.Append(pID);*/
		}
		}
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

void CFeedEngine::GetFeeds(TFeedInfoArray& array) 
{
	for (int i=0;i<feeds.Count();i++) {
		array.Append(feeds[i]);
	}
}

void CFeedEngine::GetShowsByFeed(TFeedInfo *info, TShowInfoArray& array)
{
	RDebug::Print(_L("GetShowsByFeed: %S"), &info->title);

	
	for (int i=0;i<shows.Count();i++) {
		//RDebug::Print(_L("Comparing '%S' to '%S'"), &(shows[i]->feedUrl), &info->url);
		if (shows[i]->feedUrl.Compare(info->url) == 0) {
			array.Append(shows[i]);
		}
	
	}
	RDebug::Print(_L("GetShowsByFeed returning %d out of %d shows"), array.Count(), shows.Count());
}

void CFeedEngine::GetShowsDownloading(TShowInfoArray& array)
{
	for (int i=0;i<shows.Count();i++) {
		if (shows[i]->downloadState == EQueued) {
			array.Append(shows[i]);
		}
	}

}

void CFeedEngine::GetAllShows(TShowInfoArray &array)
{
	for (int i=0;i<shows.Count();i++) {
		array.Append(shows[i]);
	}
}

void CFeedEngine::ListAllFiles()
	{
/*	RFs rfs;
	rfs.Connect();
	TBuf<100> podcastDir;
	podcastDir.Copy(KPodcastDir);
	
	for (int i=0;i<shows.Count();i++) {
		if(BaflUtils::FileExists(rfs, files[i]->fileName) == EFalse) {
			RDebug::Print(_L("%S was removed, delisting"), &files[i]->fileName);
			files.Remove(i);
		}
	}

	ListDir(rfs, podcastDir, files);	*/
}

void CFeedEngine::AddDownload(TShowInfo *info)
	{
	RDebug::Print(_L("AddDownload START"));
	info->downloadState = EQueued;
	DownloadNextShow();
	}


void CFeedEngine::DownloadNextShow()
{
	if (iDownloading) {
		RDebug::Print(_L("Already downloading"));
		return;
	}
	
	TShowInfoArray array;
	GetShowsDownloading(array);
	if(array.Count() > 0) {
		TShowInfo *info = array[0];
		RDebug::Print(_L("Downloading %S"), &(info->title));
		info->downloadState = EDownloading;
		GetShow(info);
	}
	array.Close();
}

void CFeedEngine::CleanHtml(TDes &str)
{
//	RDebug::Print(_L("CleanHtml %d, %S"), str.Length(), &str);
	int startPos = str.Locate('<');
	int endPos = str.Locate('>');
	//RDebug::Print(_L("length: %d, startPos: %d, endPos: %d"), str.Length(), startPos, endPos);
	TBuf<KDescriptionLength> tmp;
	while (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		//RDebug::Print(_L("Cleaning out %S"), &str.Mid(startPos, endPos-startPos+1));
		tmp.Copy(str.Left(startPos));
		if (str.Length() > endPos+1) {
			tmp.Append(str.Mid(endPos+1));
		}
		str.Copy(tmp);
		startPos = str.Locate('<');
		endPos = str.Locate('>');
	}
}