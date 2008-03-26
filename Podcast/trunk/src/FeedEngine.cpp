#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include "PodcastModel.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include <e32hashtab.h>
#include "OpmlParser.h"

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
	
    TFileName importFile = iPodcastModel.SettingsEngine().DefaultFeedsFileName();
    if (!LoadFeeds() && BaflUtils::FileExists(iFs, importFile)) {
    	ImportFeedsL(importFile);
    }

    importFile = iPodcastModel.SettingsEngine().ImportFeedsFileName();
    if (BaflUtils::FileExists(iFs, importFile)) {
    	ImportFeedsL(importFile);
    }

	LoadBooksL();
	}

CFeedEngine::CFeedEngine(CPodcastModel& aPodcastModel) : iFeedTimer(this), iPodcastModel(aPodcastModel)
	{
	iClientState = ENotUpdating;
	}

CFeedEngine::~CFeedEngine()
	{
	iObservers.Close();
	
	iFeedsUpdating.Close();
	iSortedFeeds.ResetAndDestroy();
	iSortedFeeds.Close();
	iSortedBooks.ResetAndDestroy();
	iSortedBooks.Close();
	iFs.Close();
	delete iParser;
	delete iFeedClient;
	}

/**
 * Returns the current internal state of the feed engine4
 */
TClientState CFeedEngine::ClientState()
	{
	return iClientState;
	}


/**
 * Returns the current updating client UID if clientstate is != ENotUpdateing
 * @return TUint
 */
TUint CFeedEngine::ActiveClientUid()
	{
		if(iActiveFeed != NULL)
			{
			return iActiveFeed->Uid();
			}
		return 0;	
	}

void CFeedEngine::RunFeedTimer()
	{
	iFeedTimer.Cancel();

	if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdateAtTime) {
		iFeedTimer.SetPeriod(24*60);
		iFeedTimer.SetSyncTime(iPodcastModel.SettingsEngine().UpdateFeedTime());
	} else if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdatePeriodically){
		int interval = iPodcastModel.SettingsEngine().UpdateFeedInterval();
	
		if (interval != 0) {
			iFeedTimer.SetPeriod(interval);
			iFeedTimer.RunPeriodically();
		}
	}
	
	}

void CFeedEngine::UpdateAllFeedsL()
	{
	if (iFeedsUpdating.Count() > 0) {
		RDebug::Print(_L("Cancelling update"));
		iFeedClient->Stop();
		iFeedsUpdating.Reset();
		return;
	}

	TInt cnt = iSortedFeeds.Count();
	for (int i=0;i<cnt;i++) {
		iFeedsUpdating.Append(iSortedFeeds[i]);
	}

	UpdateNextFeedL();
	}

void CFeedEngine::CancelUpdateAllFeedsL()
	{
	if(iClientState != ENotUpdating)
	{
		iFeedsUpdating.Reset();
		iFeedClient->Stop();
	}
}

void CFeedEngine::UpdateNextFeedL()
	{

	RDebug::Print(_L("UpdateNextFeed. %d feeds left to update"), iFeedsUpdating.Count());
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
		iClientState = ENotUpdating;
		for (TInt i=0;i<iObservers.Count();i++) 
		{
			TRAP_IGNORE(iObservers[i]->FeedUpdateAllCompleteL());
		}
	}
}

void CFeedEngine::UpdateFeedL(TUint aFeedUid)
	{
	iClientState = EUpdatingFeed;
	iActiveFeed = GetFeedInfoByUid(aFeedUid);

	for (TInt i=0;i<iObservers.Count();i++) {
			TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), 0));
		}

	TFileName filePath;
	filePath.Copy (iPodcastModel.SettingsEngine().PrivatePath ());
	TBuf<20> feedUidNum;
	feedUidNum.Format(_L("%lu"), aFeedUid);
	filePath.Append(feedUidNum);
	filePath.Append(_L(".xml"));
	//filePath.Append(_L("feed.xml"));
	iUpdatingFeedFileName.Copy(filePath);
	iFeedClient->GetL(iActiveFeed->Url(), iUpdatingFeedFileName, iPodcastModel.SettingsEngine().SpecificIAP());	

	RDebug::Print(_L("Update done"));
	}

TBool CFeedEngine::NewShow(CShowInfo *item)
	{
	//RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %d"), &(item->Title()), &(item->Url()), item->Description().Length(), item->FeedUid());
	TBuf<2048> description;
	description.Copy(item->Description());
	CleanHtml(description);
	//RDebug::Print(_L("New show has feed ID: %u"), item->FeedUid());
	item->SetDescriptionL(description);
	//RDebug::Print(_L("Description: %S"), &description);
	
	TBool isShowAdded = iPodcastModel.ShowEngine().AddShow(item);
	
	if (isShowAdded && iPodcastModel.SettingsEngine().DownloadAutomatically()) 
		{
		iPodcastModel.ShowEngine().AddDownload(item);
		}
	return isShowAdded;
	}

void CFeedEngine::GetFeedImageL(CFeedInfo *aFeedInfo)
	{
	RDebug::Print(_L("GetFeedImage"));
	iClientState = EUpdatingImage;
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

void CFeedEngine::FileNameFromUrl(const TDesC& aUrl, TFileName &aFileName)
	{
	TInt pos = aUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) 
		{	
		TPtrC str = aUrl.Mid(pos+1);
		pos = str.Locate('?');
		if (pos != KErrNotFound) 
			{
			aFileName.Copy(str.Left(pos));
			} 
		else 
			{
			aFileName.Copy(str);
			}
		}
	RDebug::Print(_L("FileNameFromUrl in: %S, out: %S"), &aUrl, &aFileName);
	}

void CFeedEngine::EnsureProperPathName(TFileName &aPath)
	{
	ReplaceString(aPath, _L("/"), _L("_")); // better not to add \\ in case we have multiple /
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

TBool CFeedEngine::AddFeed(CFeedInfo *aItem) 
	{
	RDebug::Print(_L("CFeedEngine::AddFeed, title=%S, URL=%S"), &aItem->Title(), &aItem->Url());
	for (TInt i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aItem->Uid()) 
			{
			RDebug::Print(_L("Already have feed %S, discarding"), &aItem->Url());
			delete aItem;
			aItem = NULL;
			return EFalse;
			}
		}
	
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
	iSortedFeeds.InsertInOrder(aItem, sortOrder);

	// Save the feeds into DB
	SaveFeeds();
	return ETrue;
	}

void CFeedEngine::RemoveFeed(TUint aUid) 
	{
	for (int i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().RemoveAllShowsByFeed(aUid);
					
			CFeedInfo* feedToRemove = iSortedFeeds[i];
			
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

			iSortedFeeds.Remove(i);
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

void CFeedEngine::Progress(CHttpClient* /*aHttpClient*/, TInt aBytes, TInt aTotalBytes)
	{	
	
	TInt percent = 0;
	if (aTotalBytes > 0)
		{
		percent = (aBytes*100)/aTotalBytes;		
		}
	
	for (TInt i=0;i<iObservers.Count();i++) 
		{
		TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), percent));
		}
	/*if (iClientState == EFeed) {
		int percent = -1;
		if (aTotalBytes != -1) {
			percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}

	}*/
}

void CFeedEngine::CompleteL(CHttpClient* /*aClient*/, TBool aSuccessful)
	{
	RDebug::Print(_L("Complete, aSuccessful=%d"), aSuccessful);
	if (iClientState == EUpdatingFeed) 
		{
		// Parse the feed. We need to trap this call since it could leave and then
		// the whole update chain will be broken
		TRAPD(parserErr, iParser->ParseFeedL(iUpdatingFeedFileName, iActiveFeed, iPodcastModel.SettingsEngine().MaxListItems()));
		if(parserErr)
			{
			// we do not need to any special action on this error.
			RDebug::Print(_L("CFeedEngine::Complete()\t Failed to parse feed. Leave with error code=%d"), parserErr);
			}
			
		// delete the downloaded XML file as it is no longer needed
		BaflUtils::DeleteFile(iFs,iUpdatingFeedFileName);

		// change client state
		iClientState = ENotUpdating;

		TTime time;
		time.HomeTime();
		iActiveFeed->SetLastUpdated(time);
		SaveFeeds();
		iPodcastModel.ShowEngine().SaveShows();

		// if the feed has specified a image url. download it if we dont already have it
		if ( iActiveFeed->ImageUrl().Length() > 0 && ((iActiveFeed->ImageFileName().Length() == 0) || (!BaflUtils::FileExists(iFs,iActiveFeed->ImageFileName()) )))
 			{
			TRAPD(error, GetFeedImageL(iActiveFeed));
			if (error)
				{
				// we have failed in a very early stage to fetch the image.
				// continue with next Feed update
				NotifyFeedUpdateComplete();
				UpdateNextFeedL();
				}
			}
		else
			{
			// we do not have an image file
			NotifyFeedUpdateComplete();
			UpdateNextFeedL();		
			}

		// we will wait until the image has been downloaded to start the next feed update.
		}
	else	// iClientState == EUpdatingImage
		{
		// change client state to not updating
		iClientState = ENotUpdating;
		
		NotifyFeedUpdateComplete();
		UpdateNextFeedL();
		}
	}

void CFeedEngine::NotifyFeedUpdateComplete()
	{
	for (TInt i=0;i<iObservers.Count();i++) 
		{
		TRAP_IGNORE(iObservers[i]->FeedUpdateCompleteL(iActiveFeed->Uid()));
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

void CFeedEngine::ImportFeedsL(const TDesC& aFile)
	{
	TFileName opmlPath;
	opmlPath.Copy(aFile);
	COpmlParser opmlParser(*this);
	
	opmlParser.ParseOpmlL(opmlPath);
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
	
	CFileStore* store = NULL;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		CleanupStack::Pop(store);
		return EFalse;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	TInt version = instream.ReadInt32L();
	RDebug::Print(_L("Read version: %d"), version);

	//if (version != KFeedInfoVersion) {
	//	RDebug::Print(_L("Wrong version, discarding"));
	//	CleanupStack::PopAndDestroy(2); // instream and store
	//	return EFalse;
	//}
	
	TInt count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	CFeedInfo *readData;
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
	for (TInt i=0;i<count;i++) {
		readData = CFeedInfo::NewL(version);
		TRAP(error, instream  >> *readData);
		//RDebug::Print(_L("error: %d"), error);
		iSortedFeeds.InsertInOrder(readData, sortOrder);
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
	RDebug::Print(_L("Saving %d feeds"), iSortedFeeds.Count());
	outstream.WriteInt32L(iSortedFeeds.Count());
	for (TInt i=0;i<iSortedFeeds.Count();i++) {
//		RDebug::Print(_L("Storing feed %i"), i);
		outstream  << *iSortedFeeds[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);		
	}


CFeedInfo* CFeedEngine::GetFeedInfoByUid(TUint aFeedUid)
	{
	TInt cnt = iSortedFeeds.Count();
	for (TInt i=0;i<cnt;i++)
		{
		if (iSortedFeeds[i]->Uid() == aFeedUid)
			{
			return iSortedFeeds[i];
			}
		}
	
	return NULL;
	}

const RFeedInfoArray& CFeedEngine::GetSortedFeeds() const 
{
	return iSortedFeeds;
}

void CFeedEngine::AddBookL(const TDesC& aBookTitle, CDesCArrayFlat* aFileNameArray)
	{
	
	if(aFileNameArray && aFileNameArray->Count() > 0)
		{
	
		TParsePtrC parser (aFileNameArray->MdcaPoint(0));
		CFeedInfo* item = CFeedInfo::NewLC();
		
		TFileName tempUrl;
		tempUrl.Copy(parser.DriveAndPath());
		tempUrl.Append(aBookTitle);
		item->SetUrlL(tempUrl);
		item->SetIsBookFeed();

		for (TInt i=0;i<iSortedBooks.Count();i++) 
			{
			if (iSortedBooks[i]->Uid() == item->Uid()) 
				{
				RDebug::Print(_L("Already have book %S, discarding"), &item->Url());
				CleanupStack::PopAndDestroy(item);
				return;
				}
			}
		item->SetTitleL(aBookTitle);
		item->SetDescriptionL(aBookTitle);
		TTime time;
		time.HomeTime();
		item->SetLastUpdated(time);
	
		TInt cnt = aFileNameArray->Count();
		CShowInfo* showInfo = NULL;
		TEntry fileInfo;
		for(TInt loop = 0;loop<cnt;loop++)
		{
			showInfo = CShowInfo::NewL();
			CleanupStack::PushL(showInfo);
			showInfo->SetTitleL(aBookTitle);		
			showInfo->SetDescriptionL(aBookTitle);	
			showInfo->SetDownloadState(EDownloaded);
			showInfo->SetUrlL(aFileNameArray->MdcaPoint(loop));
			showInfo->SetFeedUid(item->Uid());
			showInfo->SetPubDate(time);
			showInfo->SetTrackNo(loop+1);
			showInfo->SetShowType(EAudioBook);

			showInfo->SetFileNameL(aFileNameArray->MdcaPoint(loop));

			if(iFs.Entry(aFileNameArray->MdcaPoint(loop), fileInfo) == KErrNone)
			{
				showInfo->SetShowSize(fileInfo.iSize);
			}

			showInfo->SetPlayState(ENeverPlayed);
			iPodcastModel.ShowEngine().AddShow(showInfo);			
			iPodcastModel.ShowEngine().MetaDataReader().SubmitShow(showInfo);
			
			CleanupStack::Pop(showInfo);
		}
		
		TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
		iSortedBooks.InsertInOrder(item, sortOrder);
		CleanupStack::Pop(item);
		// Save the feeds into DB
		SaveBooksL();
		}
	}

void CFeedEngine::RemoveBookL(TUint aUid)
	{
		for (int i=0;i<iSortedBooks.Count();i++) 
		{
		if (iSortedBooks[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().RemoveAllShowsByFeed(aUid);
					
			CFeedInfo* feedToRemove = iSortedBooks[i];					
				
			//delete the folder. It has the same name as the title.
			TFileName filePath;
			filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
			filePath.Append(feedToRemove->Title());
			filePath.Append(_L("\\"));
			iFs.RmDir(filePath);

			iSortedBooks.Remove(i);
			delete feedToRemove;
			
			SaveBooksL();
			return;
			}
		}
	}

TBool CFeedEngine::LoadBooksL()
	{
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KBookDB);
	iFs.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(iFs, privatePath)) {
		RDebug::Print(_L("No books DB file"));	
		return EFalse;
	}
	
	CFileStore* store = NULL;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		CleanupStack::Pop(store);
		return EFalse;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	TInt version = instream.ReadInt32L();

	if (version != KFeedInfoVersion) {
		CleanupStack::PopAndDestroy(2); // instream and store
		return EFalse;
	}
	
	TInt count = instream.ReadInt32L();
	CFeedInfo *readData;
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
	for (TInt i=0;i<count;i++) {
		readData = CFeedInfo::NewL();
		TRAP(error, instream  >> *readData);
		readData->SetIsBookFeed();
		iSortedBooks.InsertInOrder(readData, sortOrder);
	}
	CleanupStack::PopAndDestroy(2); // instream and store
	return ETrue;
	}

void CFeedEngine::SaveBooksL()
	{
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KBookDB);
	
	iFs.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(iFs, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	outstream.WriteInt32L(KFeedInfoVersion);
	outstream.WriteInt32L(iSortedBooks.Count());
	for (TInt i=0;i<iSortedBooks.Count();i++) {
		outstream  << *iSortedBooks[i];
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);		
	}



const RFeedInfoArray& CFeedEngine::GetSortedBooks() const 
{
	return iSortedBooks;
}

void CFeedEngine::CleanHtml(TDes &str)
{
	ReplaceString(str, _L("\n"), _L(""));

//	RDebug::Print(_L("CleanHtml %d, %S"), str.Length(), &str);
	TInt startPos = str.Locate('<');
	TInt endPos = str.Locate('>');
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