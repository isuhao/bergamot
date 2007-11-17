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
	iFs.Connect();
	LoadSettings();
    LoadFeeds();
    LoadUserFeeds();
    LoadShows();
	}

CFeedEngine::CFeedEngine()
	{
	iShowClient = CHttpClient::NewL(*this);
	iFeedClient = CHttpClient::NewL(*this);
	iFeedListFile.Copy(KFeedsFileName);
	}

CFeedEngine::~CFeedEngine()
	{
	iShows.ResetAndDestroy();
	iShows.Close();
	iFeeds.ResetAndDestroy();
	iFeeds.Close();
	iFs.Close();
	delete iParser;
	delete iFeedClient;
	delete iShowClient;
	}

void CFeedEngine::StopDownloads() 
	{
	RDebug::Print(_L("StopDownloads"));
	}

void CFeedEngine::GetFeed(TFeedInfo* feedInfo)
	{
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	
	int pos = feedInfo->url.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedInfo->url.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}

	feedInfo->fileName.Copy(privatePath);
	RDebug::Print(_L("URL: %S, fileName: %S"), &feedInfo->url, &feedInfo->fileName);
	iFeedClient->GetFeed(feedInfo);
	RDebug::Print(_L("GetFeed END"));
	}

void CFeedEngine::GetShow(TShowInfo *info)
	{
	TBuf<100> filePath;
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
	BaflUtils::EnsurePathExistsL(iFs, filePath);

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
	iShowClient->GetShow(info);
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

void CFeedEngine::AddShow(TShowInfo *item) {
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->url.Compare(item->url) == 0) {
			return;
		}
	}
	iShows.Append(item);
	}

void CFeedEngine::AddFeed(TFeedInfo *item) {
	for (int i=0;i<iFeeds.Count();i++) {
		if (iFeeds[i]->url.Compare(item->url) == 0) {
			RDebug::Print(_L("Already have feed %S, discarding"), &item->url);
			return;
		}
	}
	iFeeds.Append(item);
	}


void CFeedEngine::NewShowCallback(TShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %S"), &(item->title), &(item->url), item->description.Length(), &(item->feedUrl));
	CleanHtml(item->description);
	item->uid = DefaultHash::Des16(item->url);
	//RDebug::Print(_L("Setting UID to %d"), item->uid);
	AddShow(item);
	}

void CFeedEngine::ParsingCompleteCallback(TFeedInfo *item)
	{
	RDebug::Print(_L("ParsingCompleteCallback"));
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->FeedInfoUpdated(*item);
		iObservers[i]->ShowListUpdated();
	}
	}

void CFeedEngine::ConnectedCallback()
	{
	
	}

void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CFeedEngine::ProgressCallback(int percent)
	{
	TBuf<100> printBuffer;
	if (percent == -1) {
		printBuffer.Format(_L("Downloading..."));
	
	} else {
		printBuffer.Format(_L("Downloading... %d%%"), percent);
	}
	User::InfoPrint(printBuffer);
	
	}

void CFeedEngine::ShowCompleteCallback(TShowInfo *info)
	{
	RDebug::Print(_L("File %S complete"), &info->fileName);
	info->downloadState = EDownloaded;
	SaveShows();
	DownloadNextShow();
	}

void CFeedEngine::FeedCompleteCallback(TFeedInfo * aInfo)
	{
	RDebug::Print(_L("File to parse: %S"), &aInfo->fileName);
	iParser->ParseFeedL(aInfo->fileName, aInfo);
	aInfo->iUid = DefaultHash::Des16(aInfo->url);
	SaveFeeds();
	SaveShows();

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

void CFeedEngine::LoadUserFeeds()
	{
	
	TBuf<100> configPath;
	configPath.Copy(iFeedListFile);
	RDebug::Print(_L("Reading feeds from %S"), &configPath);
	RFile rfile;
	int error = rfile.Open(iFs, configPath,  EFileRead);
	
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

		AddFeed(fi);
		error = tft.Read(line);
		}
	}
	
void CFeedEngine::LoadFeeds()
	{
	RDebug::Print(_L("LoadFeeds"));
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KFeedDB);
	iFs.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(iFs, privatePath)) {
		RDebug::Print(_L("No feed DB file"));	
		return;
	}
	
	CFileStore* store;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		CleanupStack::Pop(store);
		return;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	int version = instream.ReadInt32L();
	RDebug::Print(_L("Read version: %d"), version);

	if (version != KFeedInfoVersion) {
		RDebug::Print(_L("Wrong version, discarding"));
		goto exit_point;
	}
	
	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	TFeedInfo *readData;
	
	for (int i=0;i<count;i++) {
		readData = new TFeedInfo;
		TRAP(error, instream  >> *readData);
		//RDebug::Print(_L("error: %d"), error);
		iFeeds.Append(readData);
	}
	exit_point:
	CleanupStack::PopAndDestroy(); // instream
	
	CleanupStack::PopAndDestroy(store);	
	}

void CFeedEngine::SaveFeeds()
	{
	RDebug::Print(_L("SaveFeeds"));
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KFeedDB);
	
	RDebug::Print(_L("File: %S"), &privatePath);
	iFs.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(iFs, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	outstream.WriteInt32L(KFeedInfoVersion);
	RDebug::Print(_L("Saving %d feeds"), iFeeds.Count());
	outstream.WriteInt32L(iFeeds.Count());
	for (int i=0;i<iFeeds.Count();i++) {
//		RDebug::Print(_L("Storing feed %i"), i);
		outstream  << *iFeeds[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);		
	}

void CFeedEngine::LoadShows()
	{
	RDebug::Print(_L("LoadShows"));
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KShowDB);
	iFs.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(iFs, privatePath)) {
		RDebug::Print(_L("No show DB file"));	
		return;
	}
	
	CFileStore* store;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
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
		TRAPD(error, instream  >> *readData);
		//RDebug::Print(_L("error: %d"), error);
		AddShow(readData);
	}
	exit_point:
	CleanupStack::PopAndDestroy(); // instream
	
	CleanupStack::PopAndDestroy(store);	
	}

void CFeedEngine::SaveShows()
	{
	RDebug::Print(_L("SaveShows"));
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KShowDB);
	
	RDebug::Print(_L("File: %S"), &privatePath);
	iFs.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(iFs, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	outstream.WriteInt32L(KShowInfoVersion);
	RDebug::Print(_L("Saving %d shows"), iShows.Count());
	outstream.WriteInt32L(iShows.Count());
	for (int i=0;i<iShows.Count();i++) {
//		RDebug::Print(_L("Storing show %i"), i);
		outstream  << *iShows[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);	
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
	for (int i=0;i<iFeeds.Count();i++) {
		array.Append(iFeeds[i]);
	}
}

void CFeedEngine::GetShowsByFeed(TFeedInfo *info, TShowInfoArray& array)
{
	RDebug::Print(_L("GetShowsByFeed: %S"), &info->title);

	
	for (int i=0;i<iShows.Count();i++) {
		//RDebug::Print(_L("Comparing '%S' to '%S'"), &(iShows[i]->feedUrl), &info->url);
		if (iShows[i]->feedUrl.Compare(info->url) == 0) {
			array.Append(iShows[i]);
		}
	
	}
	RDebug::Print(_L("GetShowsByFeed returning %d out of %d shows"), array.Count(), iShows.Count());
}

void CFeedEngine::GetShowsDownloading(TShowInfoArray& array)
{
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->downloadState == EQueued) {
			array.Append(iShows[i]);
		}
	}

}

void CFeedEngine::GetShowsDownloaded(TShowInfoArray& array)
	{
		for (int i=0;i<iShows.Count();i++) {
			if (iShows[i]->downloadState == EDownloaded) {
				array.Append(iShows[i]);
			}
		}

	}

void CFeedEngine::GetAllShows(TShowInfoArray &array)
{
	for (int i=0;i<iShows.Count();i++) {
		array.Append(iShows[i]);
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
	if (iShowClient->IsActive()) {
		RDebug::Print(_L("Show client busy..."));
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
