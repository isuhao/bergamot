#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include "PodcastModel.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include <e32hashtab.h>

CFeedEngine* CFeedEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CFeedEngine* self = new (ELeave) CFeedEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CFeedEngine::ConstructL()
	{
	iParser = new (ELeave) CFeedParser(*this);
	iFs.Connect();
	iFeedClient = CHttpClient::NewL(*this);
	iFeedTimer.ConstructL();
	
	RunFeedTimer();
	
    if (!LoadFeeds()) {
    	ImportFeeds(iPodcastModel.SettingsEngine().DefaultFeedsFileName());
    }
	}

CFeedEngine::CFeedEngine(CPodcastModel& aPodcastModel) : iFeedTimer(this), iPodcastModel(aPodcastModel)
	{
	}

CFeedEngine::~CFeedEngine()
	{
	iFeeds.ResetAndDestroy();
	iFeeds.Close();
	iFs.Close();
	delete iParser;
	delete iFeedClient;
	}

void CFeedEngine::RunFeedTimer()
	{
	int interval = iPodcastModel.SettingsEngine().UpdateFeedInterval();
	iFeedTimer.Cancel();

	if (interval != 0) {
		iFeedTimer.SetPeriod(interval);
		iFeedTimer.RunPeriodically();
	}
	}

void CFeedEngine::UpdateAllFeeds()
	{
	iFeedsUpdating.Reset();
	for (int i=0;i<iFeeds.Count();i++) {
		iFeedsUpdating.Append(iFeeds[i]);
	}
	
	UpdateNextFeed();
	}

void CFeedEngine::UpdateNextFeed()
	{
	if (iFeedsUpdating.Count() > 0) {
		TFeedInfo *info = iFeedsUpdating[0];
		iFeedsUpdating.Remove(0);
		RDebug::Print(_L("** Updating %S"), &(info->iUrl));
		UpdateFeed(info->iUid);
	}
	}

void CFeedEngine::UpdateFeed(TInt aFeedUid)
	{
	iClientState = EFeed;
	TFeedInfo *feedInfo = GetFeedInfoByUid(aFeedUid);
	iActiveFeed = feedInfo;
	TFileName filePath;
	GetFeedDir(feedInfo, filePath);

	BaflUtils::EnsurePathExistsL(iFs, filePath);
	
	int pos = feedInfo->iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedInfo->iUrl.Mid(pos+1);
		//RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown"));
	}

	feedInfo->iFileName.Copy(filePath);
	//RDebug::Print(_L("URL: %S, fileName: %S"), &feedInfo->iUrl, &feedInfo->iFileName);
	iFeedClient->GetL(feedInfo->iUrl, feedInfo->iFileName);
	RDebug::Print(_L("UpdateFeed END"));
	}

void CFeedEngine::GetFeedDir(TFeedInfo *aFeedInfo, TFileName &aDir)
	{
	TBuf<100> filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().ShowDir());
	if (filePath[filePath.Length()-1] != '\\') {
		filePath.Append(_L("\\"));
	}
	// fix filename
	TBuf<1024> buf;
	buf.Copy(aFeedInfo->iUrl);
	ReplaceString(buf, _L("/"), _L("_"));
	ReplaceString(buf, _L(":"), _L("_"));
	ReplaceString(buf, _L("?"), _L("_"));
	buf.Trim();
	filePath.Append(buf);
	filePath.Append(_L("\\"));
	RDebug::Print(_L("directory to store in: %S"), &filePath);
	BaflUtils::EnsurePathExistsL(iFs, filePath);
	aDir.Copy(filePath);
	
	}

void CFeedEngine::MakeFileNameFromUrl(TDesC &aUrl, TFileName &fileName)
	{
	int pos = aUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {	
		TPtrC str = aUrl.Mid(pos+1);
		pos = str.Locate('?');
		if (pos != KErrNotFound) {			
			fileName.Copy(str.Left(pos));
			RDebug::Print(_L("fileName %S"), &(fileName));
		} else {
			RDebug::Print(_L("Not Found"));
			fileName.Copy(str);
		}
		
		} 
		
	}

void CFeedEngine::NewShow(CShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %S"), &(item->iTitle), &(item->iUrl), item->description.Length(), &(item->feedUrl));
	//CleanHtml(item->iDescription);
	
	//RDebug::Print(_L("Setting UID to %d"), item->uid);
	iPodcastModel.ShowEngine().AddShow(item);
	}

void CFeedEngine::GetFeedImage(TFeedInfo *aFeedInfo)
	{
	RDebug::Print(_L("GetFeedImage"));
	iClientState = EImage;
	TFileName filePath;
	GetFeedDir(aFeedInfo, filePath);
	
	TFileName fileName;
	MakeFileNameFromUrl(aFeedInfo->iImageUrl, fileName);
	filePath.Append(fileName);
	aFeedInfo->iImageFileName.Copy(filePath);
	RDebug::Print(_L("image file: %S"), &(aFeedInfo->iImageFileName));
	
	iFeedClient->GetL(aFeedInfo->iImageUrl, aFeedInfo->iImageFileName);
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

void CFeedEngine::AddFeed(TFeedInfo *item) {
	for (int i=0;i<iFeeds.Count();i++) {
		if (iFeeds[i]->iUid == item->iUid) {
			RDebug::Print(_L("Already have feed %S, discarding"), &item->iUrl);
			return;
		}
	}

	iFeeds.Append(item);
	}

void CFeedEngine::RemoveFeed(TInt aUid) {
	for (int i=0;i<iFeeds.Count();i++) {
		if (iFeeds[i]->iUid == aUid) {
			iPodcastModel.ShowEngine().PurgeShowsByFeed(aUid);
			iFeeds.Remove(i);
			RDebug::Print(_L("Removed feed"));
			return;
		}
	}
}

void CFeedEngine::ParsingComplete(TFeedInfo *item)
	{
	RDebug::Print(_L("ParsingCompleteCallback"));
	RDebug::Print(_L("feed image url: %S"), &item->iImageUrl);
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->FeedInfoUpdated(*item);
//		iObservers[i]->ShowListUpdated();
	}
	}


void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CFeedEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}


void CFeedEngine::Progress(CHttpClient* /*aHttpClient*/, int aBytes, int aTotalBytes)
{	
	if (iClientState == EFeed) {
		int percent = -1;
		if (aTotalBytes != -1) {
			percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}
		for (int i=0;i<iObservers.Count();i++) {
			iObservers[i]->FeedDownloadUpdatedL(percent);
		}
	}
}

void CFeedEngine::Complete(CHttpClient* /*aClient*/, TBool aSuccessful)
{
	if (iClientState == EFeed) {
		RDebug::Print(_L("File to parse: %S"), &iActiveFeed->iFileName);
		iParser->ParseFeedL(iActiveFeed->iFileName, iActiveFeed);
		iActiveFeed->iUid = DefaultHash::Des16(iActiveFeed->iUrl);
		iActiveFeed->iImageUrl.Trim();
		
		TFileName fileName;
		MakeFileNameFromUrl(iActiveFeed->iImageUrl, fileName);
		
		if (!BaflUtils::FileExists(iFs, fileName) && iActiveFeed->iImageUrl.Length() > 0) {
			GetFeedImage(iActiveFeed);
		}
		
		SaveFeeds();
		iPodcastModel.ShowEngine().SaveShows();
		
		UpdateNextFeed();
	}
}

void CFeedEngine::Disconnected(CHttpClient* aClient)
	{
	
	}

void CFeedEngine::DownloadInfo(CHttpClient* aHttpClient, int aTotalBytes)
	{
	/*RDebug::Print(_L("About to download %d bytes"), aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->iShowSize = aTotalBytes;
		}*/
	}

void CFeedEngine::ImportFeeds(TFileName &aFile)
	{
	
	TFileName configPath;
	configPath.Copy(aFile);
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
	
TBool CFeedEngine::LoadFeeds()
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
		return EFalse;
	}
	
	CFileStore* store;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		CleanupStack::Pop(store);
		return EFalse;
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
	return ETrue;
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
	
	TLinearOrder<TFeedInfo> order(CFeedEngine::CompareFeedsByTitle);
	array.Sort(order);
}


void CFeedEngine::CleanHtml(TDes &str)
{
//	RDebug::Print(_L("CleanHtml %d, %S"), str.Length(), &str);
	int startPos = str.Locate('<');
	int endPos = str.Locate('>');
	//RDebug::Print(_L("length: %d, startPos: %d, endPos: %d"), str.Length(), startPos, endPos);
	TBuf<2048> tmp;
	while (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		//RDebug::Print(_L("Cleaning out %S"), &str.Mid(startPos, endPos-startPos+1));
		tmp.Copy(str.Left(startPos));
		TPtrC ptr=str.Mid(startPos, endPos-startPos+1);
		if (ptr.CompareF(_L("<br>"))) {
			//RDebug::Print(_L("byter ut %S mot \\n"), &ptr);
			tmp.Append(_L("\n"));
		}
		
		if (str.Length() > endPos+1) {
			tmp.Append(str.Mid(endPos+1));
		}
		str.Copy(tmp);
		startPos = str.Locate('<');
		endPos = str.Locate('>');
	}
	
	str.Trim();
}

TInt CFeedEngine::CompareFeedsByTitle(const TFeedInfo &a, const TFeedInfo &b)
	{
		//RDebug::Print(_L("Comparing %S to %S"), &a.iTitle, &b.iTitle);
		return a.iTitle.CompareF(b.iTitle);
	}
