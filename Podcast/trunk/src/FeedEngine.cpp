#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>

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
	LoadSettings();
    LoadFeeds();
    LoadMetaData();
}

CFeedEngine::CFeedEngine() : parser(*this)
	{
	showClient = CHttpClient::NewL(*this);
	feedClient = CHttpClient::NewL(*this);
	iFeedListFile.Copy(KFeedsFileName);
    iPlayer = CMdaAudioPlayerUtility::NewL(*this);
    RSemaphore semaphore;
    semaphore.CreateGlobal(KMetaDataSemaphoreName, 0);
	}

CFeedEngine::~CFeedEngine()
	{
	shows.Close();
	feeds.Close();
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
	//iFileName.Copy(privatePath);
	feedInfo->fileName.Copy(privatePath);
	feedClient->GetFeed(feedInfo);

	RDebug::Print(_L("GetFeed END"));
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
		filePath.Append(_L("unknown.mp3"));
	}
	info->state = EDownloading;
	info->fileName.Copy(filePath);
	iDownloading = ETrue;
	showClient->GetShow(info);
	}

void CFeedEngine::Item(TShowInfo *item)
	{
	RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %S"), &(item->title), &(item->url), item->description.Length(), &(item->feedTitle));
	CleanHtml(item->description);
	AddShow(item);
	}

void CFeedEngine::AddShow(TShowInfo *item) {
	shows.Append(item);
}

void CFeedEngine::ParsingComplete()
	{
	
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
	info->state = EDownloaded;
	SaveMetaData();
	iDownloading = EFalse;
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

	int version = instream.ReadInt32L();
	RDebug::Print(_L("Read version: %d"), version);

	if (version != KMetaDataFileVersion) {
		RDebug::Print(_L("Wrong version, discarding"));
		goto exit_point;
	}
	
	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	TShowInfo *readData;
	
	for (int i=0;i<count;i++) {
		readData = new TShowInfo;
		TRAP(error, instream  >> *readData);
		RDebug::Print(_L("error: %d"), error);
		AddShow(readData);
	}
	exit_point:
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
	outstream.WriteInt32L(KMetaDataFileVersion);
	RDebug::Print(_L("Saving %d items"), shows.Count());
	outstream.WriteInt32L(shows.Count());
	for (int i=0;i<shows.Count();i++) {
		RDebug::Print(_L("Storing show %i"), i);
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
			
			RDebug::Print(entry.iName);
			TShowInfo *pID = new TShowInfo;
			pID->fileName.Copy(fileName);
			pID->title.Copy(entry.iName);		
			pID->position = 0;
			pID->state = EStateless;
			pID->iShowDownloaded = ETrue;
			//LoadMetaDataFromFile(pID);
			files.Append(pID);
		}
		}
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

RArray <TFeedInfo*>& CFeedEngine::GetFeeds() 
{
	return feeds;
}

TShowInfoArray* CFeedEngine::GetShowsByFeed(TDesC& feedTitle)
{
	RDebug::Print(_L("GetShowsByFeed: %S"), &feedTitle);
	TShowInfoArray *array = new TShowInfoArray;
	RDebug::Print(_L("Searching through %d shows"), shows.Count());
	for (int i=0;i<shows.Count();i++) {
		RDebug::Print(_L("Comparing '%S' to '%S'"), &(shows[i]->feedTitle), &feedTitle);
		if (shows[i]->feedTitle.Compare(feedTitle) == 0) {
			array->Append(shows[i]);
		}
	
	}
	RDebug::Print(_L("GetShowsByFeed returning %d shows"), array->Count());
	return array;
}

TShowInfoArray* CFeedEngine::GetShowsDownloading()
{
	TShowInfoArray *array = new TShowInfoArray;

	for (int i=0;i<shows.Count();i++) {
		if (shows[i]->state == EDownloading) {
			array->Append(shows[i]);
		}
	}
	return array;

}

TShowInfoArray* CFeedEngine::GetAllShows()
{
	return &shows;
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
	info->state = EDownloading;
	DownloadNextShow();
	}


void CFeedEngine::DownloadNextShow()
{
	if (iDownloading) {
		return;
	}
	
	TShowInfoArray* array = GetShowsDownloading();
	if(array->Count() > 0) {
		TShowInfo *info = (*array)[0];
		RDebug::Print(_L("Downloading %S"), &(info->title));
		info->state = EDownloading;
		GetShow(info);
	}
	delete array;
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

void CFeedEngine::MapcPlayComplete(TInt aError)
	{
	
	}

void CFeedEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration)
	{
	int numEntries = 0;
	int error = iPlayer->GetNumberOfMetaDataEntries(numEntries);
	if (error != KErrNone) {
		return;
	}
	
	RDebug::Print(_L("Found %d meta data entries"), numEntries);
	/*
	 * Entry: Name: year, Value:
	Entry: Name: title, Value:
	Entry: Name: album, Value:
	Entry: Name: genre, Value:
	Entry: Name: comment, Value:
	Entry: Name: artist, Value:
	Entry: Name: duration, Value:
	Entry: Name: num-tracks, Value:
	Entry: Name: track-info/audio/format, Value:
	Entry: Name: track-info/bit-rate, Value:
	Entry: Name: track-info/sample-rate, Value:
	Entry: Name: track-info/audio/channels, Value:
	Entry: Name: graphic;format=APIC;index=0, Value:*/
	/*	
	for (int i=0;i<numEntries;i++) {
		CMMFMetaDataEntry* entry = iPlayer->GetMetaDataEntryL(i);
		RDebug::Print(_L("Entry: Name: %S, Value: "), &entry->Name()); //, &entry->Value());
		if (entry->Name().CompareF(_L("title")) == 0) {
			metadataFile->title.Copy(entry->Value());
		}
	}
    iPlayer->Close();*/
    /*RSemaphore s;
	error = s.OpenGlobal(KMetaDataSemaphoreName);
	if (error == KErrNone) {
		RDebug::Print(_L("Signalling semaphore"));
		s.Signal();
	}
	s.Close();*/
	
	}


void CFeedEngine::LoadMetaDataFromFile(TShowInfo *info) {
	/*metadataFile = info;
	TRAPD(error, iPlayer->OpenFileL(info->fileName));
    if (error != KErrNone) {
		RDebug::Print(_L("Error: %d"), error);
		return;
    }*/
    /*RSemaphore s;
	error = s.OpenGlobal(KMetaDataSemaphoreName);
	if (error == KErrNone) {
		RDebug::Print(_L("Waiting for semaphore"));
		s.Wait();
	}
    RDebug::Print(_L("Done reading meta data"));
    s.Close();*/
}
