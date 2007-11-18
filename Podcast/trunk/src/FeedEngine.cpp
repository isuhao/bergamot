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
	CleanupStack::Pop();
	return self;
	}

void CFeedEngine::ConstructL()
	{
	iParser = new (ELeave) CFeedParser(*this);
	iFs.Connect();
	iShowClient = CHttpClient::NewL(*this);
	iFeedClient = CHttpClient::NewL(*this);
	LoadSettings();
    LoadFeeds();
    LoadUserFeeds();
    LoadShows();
	}

CFeedEngine::CFeedEngine()
	{
	iFeedListFile.Copy(KFeedsFileName);
	}

CFeedEngine::~CFeedEngine()
	{
	iShowsDownloading.Close();
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


TShowInfo* CFeedEngine::ShowDownloading()
{
	return iShowDownloading;
}

void CFeedEngine::UpdateFeed(TInt aFeedUid)
	{
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	TFeedInfo *feedInfo = GetFeedInfoByUid(aFeedUid);
	int pos = feedInfo->iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedInfo->iUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}

	feedInfo->iFileName.Copy(privatePath);
	RDebug::Print(_L("URL: %S, fileName: %S"), &feedInfo->iUrl, &feedInfo->iFileName);
	iFeedClient->GetFeed(feedInfo);
	RDebug::Print(_L("UpdateFeed END"));
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
	TFeedInfo *feedInfo = GetFeedInfoByUid(info->iFeedUid);
	buf.Copy(feedInfo->iUrl);
	ReplaceString(buf, _L("/"), _L("_"));
	ReplaceString(buf, _L(":"), _L("_"));
	ReplaceString(buf, _L("?"), _L("_"));
	buf.Trim();
	filePath.Append(buf);
	filePath.Append(_L("\\"));
	RDebug::Print(_L("directory to store in: %S"), &filePath);
	BaflUtils::EnsurePathExistsL(iFs, filePath);

	int pos = info->iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = info->iUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown.mp3"));
	}
	RDebug::Print(_L("filePath: %S"), &filePath);
	info->iFileName.Copy(filePath);
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
		if (iShows[i]->iUrl.Compare(item->iUrl) == 0) {
			return;
		}
	}
	iShows.Append(item);
	}

void CFeedEngine::AddFeed(TFeedInfo *item) {
	for (int i=0;i<iFeeds.Count();i++) {
		if (iFeeds[i]->iUrl.Compare(item->iUrl) == 0) {
			RDebug::Print(_L("Already have feed %S, discarding"), &item->iUrl);
			return;
		}
	}
	iFeeds.Append(item);
	}


void CFeedEngine::NewShowCallback(TShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %S"), &(item->iTitle), &(item->iUrl), item->description.Length(), &(item->feedUrl));
	CleanHtml(item->iDescription);
	item->iUid = DefaultHash::Des16(item->iUrl);
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

void CFeedEngine::ConnectedCallback(CHttpClient* aClient)
	{
	
	}

void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CFeedEngine::ProgressCallback(CHttpClient* aClient, int aPercent)
{	
	for (int i=0;i<iObservers.Count();i++) {
		if(aClient == iFeedClient)
		{
			iObservers[i]->FeedDownloadUpdatedL(aPercent);
		}
		else if(aClient == iShowClient)
		{
			iObservers[i]->ShowDownloadUpdatedL(aPercent);
		}
	}
}

void CFeedEngine::ShowCompleteCallback(TShowInfo *info)
	{
	RDebug::Print(_L("File %S complete"), &info->iFileName);
	info->iDownloadState = EDownloaded;

	SaveShows();

	for (int i=0;i<iObservers.Count();i++) {
			iObservers[i]->ShowDownloadUpdatedL(100);		
		}
	
	DownloadNextShow();
	}

void CFeedEngine::FeedCompleteCallback(TFeedInfo * aInfo)
	{
	RDebug::Print(_L("File to parse: %S"), &aInfo->iFileName);
	iParser->ParseFeedL(aInfo->iFileName, aInfo);
	aInfo->iUid = DefaultHash::Des16(aInfo->iUrl);
	SaveFeeds();
	SaveShows();

	}

void CFeedEngine::DisconnectedCallback(CHttpClient* aClient)
	{
	
	}

void CFeedEngine::DownloadInfoCallback(CHttpClient* aClient, int aSize)
	{
	RDebug::Print(_L("About to download %d bytes"), aSize);
	if(aClient == iShowClient && iShowDownloading != NULL) {
		iShowDownloading->iShowSize = aSize;
		}
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
		fi->iUrl.Copy(line);
		int pos = line.Locate('|');
		if (pos == KErrNotFound) {
			fi->iTitle.Copy(line);
			fi->iUrl.Copy(line);
		}else {
			fi->iTitle.Copy(line.Left(pos));
			fi->iUrl.Copy(line.Mid(pos+1));
			RDebug::Print(_L("Read title: '%S', url: '%S'"), &fi->iTitle, &fi->iUrl);
		}
		
		fi->iTitle.Trim();
		fi->iUrl.Trim();
		fi->iUid = DefaultHash::Des16(fi->iUrl);

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
			TBuf<100> subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDir(iFs, subFolder, iShows);
		} else {
		if (entry.iName.Right(3).CompareF(_L("mp3")) == 0 ||
			entry.iName.Right(3).CompareF(_L("aac")) == 0 ||
			entry.iName.Right(3).CompareF(_L("wav")) == 0) {
			TBool exists = EFalse;
			TFileName fileName;
			fileName.Copy(folder);
			fileName.Append(entry.iName);

			for (int i=0;i<iShows.Count();i++) {
				if (iShows[i]->iFileName.Compare(fileName) == 0) {
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
			pID->iFileName.Copy(fileName);
			pID->iTitle.Copy(entry.iName);		
			pID->iPosition = 0;
			pID->iDownloadState = EDownloaded;
			RDebug::Print(_L("Adding!"));
			AddShow(pID);
		}
		}
	}
	delete dirPtr;
	CleanupStack::Pop(dirScan);

}

TFeedInfo* CFeedEngine::GetFeedInfoByUid(TInt aFeedUid)
	{
	for (int i=0;i<iFeeds.Count();i++)
		{
		if (iFeeds[i]->iUid == aFeedUid)
			{
			return iFeeds[i];
			}
		}
	
	return NULL;
	}

void CFeedEngine::GetFeeds(TFeedInfoArray& array) 
{
	for (int i=0;i<iFeeds.Count();i++) {
		array.Append(iFeeds[i]);
	}
}

void CFeedEngine::SelectAllShows()
	{
	iSelectedShows.Reset();
	
	for (int i=0;i<iShows.Count();i++)
		{
		iSelectedShows.Append(iShows[i]);
		}
	}

void CFeedEngine::SelectShowsByFeed(TInt aFeedUid)
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iFeedUid == aFeedUid)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	}

void CFeedEngine::SelectShowsByDownloadState(TInt aDownloadState)
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		// TODO: remove ugly alpha 1 hack
		if (aDownloadState == EDownloading) {
			if (iShows[i]->iDownloadState == EDownloading ||
					iShows[i]->iDownloadState == EQueued	)
				{
				iSelectedShows.Append(iShows[i]);
				}
			
		} else {
			if (iShows[i]->iDownloadState == aDownloadState)
				{
				iSelectedShows.Append(iShows[i]);
				}
			}	
		}
	}

TShowInfoArray& CFeedEngine::GetSelectedShows()
	{
	return iSelectedShows;
	}

void CFeedEngine::ListAllFiles()
	{
	TBuf<100> podcastDir;
	podcastDir.Copy(KPodcastDir);
	/*
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->iDownloadState == EDownloaded) {
			if(BaflUtils::FileExists(iFs, iShows[i]->iFileName) == EFalse) {
				RDebug::Print(_L("%S was removed, marking"), &iShows[i]->iFileName);
				iShows[i]->ifiles.Remove(i);
			}
	}*/

	ListDir(iFs, podcastDir, iShows);
	SaveShows();
}

void CFeedEngine::AddDownload(TShowInfo *info)
	{
	RDebug::Print(_L("AddDownload START"));
	info->iDownloadState = EQueued;
	iShowsDownloading.Append(info);
	DownloadNextShow();
	}


void CFeedEngine::DownloadNextShow()
{
	if (iShowClient->IsActive()) {
		RDebug::Print(_L("Show client busy..."));
		return;
	}
	if (iShowsDownloading.Count() > 0) {
		TShowInfo *info = iShowsDownloading[0];
		iShowsDownloading.Remove(0);
		RDebug::Print(_L("Downloading %S"), &(info->iTitle));
		info->iDownloadState = EDownloading;
		iShowDownloading = info;
		GetShow(info);
	}
	else {
		iShowDownloading = NULL;
	}
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
