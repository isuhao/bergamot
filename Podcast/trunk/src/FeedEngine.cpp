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
	iFeedClient = CHttpClient::NewL(iPodcastModel, *this);
	iFeedTimer.ConstructL();
	
	RunFeedTimer();
	
    if (!LoadFeeds()) {
    	ImportFeedsL(iPodcastModel.SettingsEngine().DefaultFeedsFileName());
    }
	}

CFeedEngine::CFeedEngine(CPodcastModel& aPodcastModel) : iFeedTimer(this), iPodcastModel(aPodcastModel)
	{
	}

CFeedEngine::~CFeedEngine()
	{
	iObservers.Close();
	
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
	if (iFeedsUpdating.Count() > 0) {
		RDebug::Print(_L("Cancelling update"));
		iFeedClient->Stop();
		iFeedsUpdating.Reset();
		return;
	}
	
	for (int i=0;i<iFeeds.Count();i++) {
		iFeedsUpdating.Append(iFeeds[i]);
	}
	
	UpdateNextFeed();
	
	}

void CFeedEngine::UpdateNextFeed()
	{
	RDebug::Print(_L("UpdateNextFeed"));
	if (iFeedsUpdating.Count() > 0) {
		CFeedInfo *info = iFeedsUpdating[0];
		iFeedsUpdating.Remove(0);
		//RDebug::Print(_L("** UpdateNextFeed: %S, ID: %u"), &(info->Url()), info->Uid());
		TRAPD(error, UpdateFeedL(info->Uid()));
		
		if (error != KErrNone) {
			RDebug::Print(_L("Error while updating all feeds"));
			//iFeedsUpdating.Reset();
		}
	} else {
		for (int i=0;i<iObservers.Count();i++) {
			iObservers[i]->FeedUpdateComplete();
		}
	}
	}

void CFeedEngine::UpdateFeedL(TUint aFeedUid)
	{
	iClientState = EFeed;
	iActiveFeed = GetFeedInfoByUid(aFeedUid);
	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().PrivatePath());
	filePath.Append(_L("feed.xml"));
	iUpdatingFeedFileName.Copy(filePath);
	iFeedClient->GetL(iActiveFeed->Url(), iUpdatingFeedFileName, iPodcastModel.SettingsEngine().SpecificIAP());
	RDebug::Print(_L("Update done"));
	}

void CFeedEngine::NewShow(CShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %d"), &(item->Title()), &(item->Url()), item->Description().Length(), item->FeedUid());
	TBuf<2048> description;
	description.Copy(item->Description());
	CleanHtml(description);
	//RDebug::Print(_L("New show has feed ID: %u"), item->FeedUid());
	item->SetDescriptionL(description);
	//RDebug::Print(_L("Description: %S"), &description);
	
	iPodcastModel.ShowEngine().AddShow(item);
	}

void CFeedEngine::GetFeedImageL(CFeedInfo *aFeedInfo)
	{
	RDebug::Print(_L("GetFeedImage"));
	iClientState = EImage;
	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
	
	// create relative file name
	TFileName relPath;
	relPath.Copy(aFeedInfo->Title());
	relPath.Append(_L("\\"));

	TFileName fileName;
	FileNameFromUrl(aFeedInfo->ImageUrl(), fileName);
	relPath.Append(fileName);
	EnsureProperPathName(relPath);
	
	// complete file path is base dir + rel path
	filePath.Append(relPath);
	aFeedInfo->SetImageFileNameL(filePath);
	iFeedClient->GetL(aFeedInfo->ImageUrl(), filePath, ETrue);
	}

void CFeedEngine::FileNameFromUrl(TDesC &aUrl, TFileName &aFileName)
	{
	int pos = aUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {	
		TPtrC str = aUrl.Mid(pos+1);
		pos = str.Locate('?');
		if (pos != KErrNotFound) {			
			aFileName.Copy(str.Left(pos));
		} else {
			aFileName.Copy(str);
		}
	}
	RDebug::Print(_L("FileNameFromUrl in: %S, out: %S"), &aUrl, &aFileName);
	}

void CFeedEngine::EnsureProperPathName(TFileName &aPath)
	{
	ReplaceString(aPath, _L("/"), _L("\\"));
	ReplaceString(aPath, _L(":"), _L("_"));
	ReplaceString(aPath, _L("?"), _L("_"));
	//buf.Append(_L("\\"));
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

void CFeedEngine::AddFeed(CFeedInfo *item) {
	for (int i=0;i<iFeeds.Count();i++) {
		if (iFeeds[i]->Uid() == item->Uid()) {
			RDebug::Print(_L("Already have feed %S, discarding"), &item->Url());
			return;
		}
	}

	iFeeds.Append(item);
	}

void CFeedEngine::RemoveFeed(TUint aUid) 
	{
	for (int i=0;i<iFeeds.Count();i++) 
		{
		if (iFeeds[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().RemoveAllShowsByFeed(aUid);
			iPodcastModel.ShowEngine().SaveShows();
					
			CFeedInfo* feedToRemove = iFeeds[i];
			
			//delete the image file if it exists
			if ( (feedToRemove->ImageFileName().Length() >0) && BaflUtils::FileExists(iFs, feedToRemove->ImageFileName() ))
				{
				iFs.Delete(feedToRemove->ImageFileName());
				}
				
			//delete the folder. It has the same name as the title.
			TFileName filePath;
			filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
			filePath.Append(feedToRemove->Title());
			filePath.Append(_L("\\"));
			iFs.RmDir(filePath);

			iFeeds.Remove(i);
			delete feedToRemove;
			
			RDebug::Print(_L("Removed feed"));
			SaveFeeds();
			return;
		}
	}
}

void CFeedEngine::ParsingComplete(CFeedInfo *item)
	{
	TBuf<1024> title;
	title.Copy(item->Title());
	CleanHtml(title);
	item->SetTitleL(title);
	//RDebug::Print(_L("feed image url: %S"), &item->ImageUrl());
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->FeedInfoUpdated(item);
//		iObservers[i]->ShowListUpdated();
	}
	}


void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CFeedEngine::RemoveObserver(MFeedEngineObserver *observer)
	{
	TInt index = iObservers.Find(observer);
	
	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CFeedEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}

void CFeedEngine::Progress(CHttpClient* /*aHttpClient*/, int /*aBytes*/, int /*aTotalBytes*/)
{	
	/*if (iClientState == EFeed) {
		int percent = -1;
		if (aTotalBytes != -1) {
			percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}

	}*/
}

void CFeedEngine::Complete(CHttpClient* /*aClient*/, TBool aSuccessful)
{
	RDebug::Print(_L("Complete, aSuccessful=%d"), aSuccessful);
	if (iClientState == EFeed) {
	
		iParser->ParseFeedL(iUpdatingFeedFileName, iActiveFeed, iPodcastModel.SettingsEngine().MaxListItems());
		
		if (iActiveFeed->ImageFileName().Length() == 0 || !BaflUtils::FileExists(iFs,iActiveFeed->ImageFileName())) {
			TRAPD(error, GetFeedImageL(iActiveFeed));
		}

		TTime time;
		time.HomeTime();
		iActiveFeed->SetLastUpdated(time);
		SaveFeeds();
		
		iPodcastModel.ShowEngine().SaveShows();
		
		UpdateNextFeed();
	}
}

void CFeedEngine::Disconnected(CHttpClient* /*aClient*/)
	{
	
	}

void CFeedEngine::DownloadInfo(CHttpClient* /*aHttpClient */, int /*aTotalBytes*/)
	{
	/*RDebug::Print(_L("About to download %d bytes"), aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->iShowSize = aTotalBytes;
		}*/
	}

void CFeedEngine::ImportFeedsL(TFileName &aFile)
	{
	RDebug::Print(_L("Importing default feeds from %S"), &aFile);
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
		if (line.Locate('#') == 0) {
			error = tft.Read(line);
			continue;
		}
		CFeedInfo* fi = CFeedInfo::NewLC();
		line.Trim();
		int pos = line.Locate('|');
		if (pos != -1) {
			TPtrC title = line.Left(pos);
			TPtrC url = line.Mid(pos+1);
			//RDebug::Print(_L("url: %S"), &url);
			//RDebug::Print(_L("title: %S"), &title);
			fi->SetUrlL(url);
			fi->SetTitleL(title);
		} else {
			fi->SetUrlL(line);
			fi->SetTitleL(line);
		}
		CleanupStack::Pop(fi);
		
		AddFeed(fi);
		error = tft.Read(line);
		}
	rfile.Close();
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
		CleanupStack::PopAndDestroy(2); // instream and store
		return EFalse;
	}
	
	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	CFeedInfo *readData;
	
	for (int i=0;i<count;i++) {
		readData = CFeedInfo::NewL();
		TRAP(error, instream  >> *readData);
		//RDebug::Print(_L("error: %d"), error);
		iFeeds.Append(readData);
	}
	CleanupStack::PopAndDestroy(2); // instream and store
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


CFeedInfo* CFeedEngine::GetFeedInfoByUid(TUint aFeedUid)
	{
	for (int i=0;i<iFeeds.Count();i++)
		{
		if (iFeeds[i]->Uid() == aFeedUid)
			{
			return iFeeds[i];
			}
		}
	
	return NULL;
	}

void CFeedEngine::GetFeeds(RFeedInfoArray& array) 
{
	for (int i=0;i<iFeeds.Count();i++) {
		array.Append(iFeeds[i]);
	}
	
	TLinearOrder<CFeedInfo> order(CFeedEngine::CompareFeedsByTitle);
	array.Sort(order);
}


void CFeedEngine::CleanHtml(TDes &str)
{
	ReplaceString(str, _L("\n"), _L(""));

//	RDebug::Print(_L("CleanHtml %d, %S"), str.Length(), &str);
	int startPos = str.Locate('<');
	int endPos = str.Locate('>');
	//RDebug::Print(_L("length: %d, startPos: %d, endPos: %d"), str.Length(), startPos, endPos);
	TBuf<2048> tmp;
	while (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		//RDebug::Print(_L("Cleaning out %S"), &str.Mid(startPos, endPos-startPos+1));
		tmp.Copy(str.Left(startPos));
		TPtrC ptr=str.Mid(startPos, endPos-startPos+1);
		if (ptr.CompareF(_L("<br>"))== 0) {
			tmp.Append('\r');
			tmp.Append('\n');
		} else if (ptr.CompareF(_L("<p>")) == 0) {
			tmp.Append('\r');
			tmp.Append('\n');
			tmp.Append('\r');
			tmp.Append('\n');
		}
		
		if (str.Length() > endPos+1) {
			tmp.Append(str.Mid(endPos+1));
		}
		
		str.Copy(tmp);
		startPos = str.Locate('<');
		endPos = str.Locate('>');
	}
	
	str.Trim();
	if(str.Locate('&') != KErrNotFound) {
		ReplaceString(str, _L("&amp;"), _L(""));
		ReplaceString(str, _L("&quot;"), _L(""));
		ReplaceString(str, _L("&nbsp;"), _L(""));
		ReplaceString(str, _L("&copy;"), _L("(c)"));
	}

}

TInt CFeedEngine::CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b)
	{
		//RDebug::Print(_L("Comparing %S to %S"), &a.Title(), &b.Title());
		return a.Title().CompareF(b.Title());
	}
