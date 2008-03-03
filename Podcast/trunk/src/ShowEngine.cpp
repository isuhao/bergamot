#include "ShowEngine.h"
#include "FeedEngine.h"
#include "FeedInfo.h"
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"
#include <e32hashtab.h>

CShowEngine::CShowEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
	iDownloadsSuspended = ETrue;
	iSelectOnlyUnplayed = ETrue;
	}

CShowEngine::~CShowEngine()
	{
	iShows.ResetAndDestroy();
	iShows.Close();
	iSelectedShows.Close();
	iShowsDownloading.Close();
	iFs.Close();
	delete iShowClient;
	iObservers.Close();
	delete iMetaDataReader;
	}

CShowEngine* CShowEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CShowEngine* self = new (ELeave) CShowEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CShowEngine::ConstructL()
	{
	iFs.Connect();
	iShowClient = CHttpClient::NewL(iPodcastModel, *this);
	iShowClient->SetResumeEnabled(ETrue);
	iMetaDataReader = new (ELeave) CMetaDataReader(*this);
	iMetaDataReader->ConstructL();

	// Try to load the database. Continue if we fail.
	TRAP_IGNORE(LoadShowsL());
	
	//CheckFiles();
	DownloadNextShow();
	
	// maybe this is a bad idea?
	if (iShowsDownloading.Count() == 0) 
		{
		iDownloadsSuspended = EFalse;
		}
	}

void CShowEngine::StopDownloads() 
	{
	RDebug::Print(_L("StopDownloads"));
	iDownloadsSuspended = ETrue;
	iShowClient->Stop();
	}

void CShowEngine::ResumeDownloads() 
	{
	RDebug::Print(_L("ResumeDownloads"));
	iDownloadsSuspended = EFalse;
	iDownloadErrors = 0;
	DownloadNextShow();
	}

TBool CShowEngine::DownloadsStopped()
	{
	return iDownloadsSuspended;
	}

void CShowEngine::RemoveDownload(TUint aUid) 
	{
	RDebug::Print(_L("CShowEngine::RemoveDownload\t Trying to remove download"));
	
	const TInt count = iShowsDownloading.Count();
	for (TInt i=0 ; i < count; i++) 
		{
		if (iShowsDownloading[i]->Uid() == aUid) 
			{
			if (iShowsDownloading[i]->DownloadState() == EDownloading) 
				{
				iShowClient->Stop();
				}
			
			iShowsDownloading[i]->SetDownloadState(ENotDownloaded);
			BaflUtils::DeleteFile(iFs, iShowsDownloading[i]->FileName());
			iShowsDownloading.Remove(i);
			RDebug::Print(_L("CShowEngine::RemoveDownload\tDownload removed."));
			
			NotifyShowDownloadUpdated(-1,-1,-1);
			NotifyDownloadQueueUpdated();
			DownloadNextShow();
			return;		
			}
		}
	RDebug::Print(_L("CShowEngine::RemoveDownload\tCould not find downloading show to remove"));
	}

void CShowEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}

void CShowEngine::Progress(CHttpClient* /*aHttpClient */, int aBytes, int aTotalBytes)
	{	
	TInt percent = -1;
	if (aTotalBytes > 0) 
		{
		percent = (TInt) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}
	NotifyShowDownloadUpdated(percent, aBytes, aTotalBytes);
	}

void CShowEngine::Disconnected(CHttpClient* /*aClient */)
	{
	}

void CShowEngine::DownloadInfo(CHttpClient* aHttpClient, int aTotalBytes)
	{
	RDebug::Print(_L("About to download %d bytes"), aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->SetShowSize(aTotalBytes);
		}
	}

void CShowEngine::GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed )
	{
	int showsCount = 0;
	int unplayedCount = 0;
	
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			showsCount++;
			if (iShows[i]->PlayState() == ENeverPlayed) {
				unplayedCount++;
			}
			}
	}
	int maxItems = iPodcastModel.SettingsEngine().MaxListItems();
	aNumShows = showsCount < maxItems ? showsCount : maxItems;
	aNumUnplayed = unplayedCount < maxItems ? unplayedCount : maxItems;
	}

void CShowEngine::GetShow(CShowInfo *info)
	{
	CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(info->FeedUid());
	if (feedInfo == NULL) {
		RDebug::Print(_L("Feed not found for this show!"));
		return;
	}
	
	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
	
	// create relative file name
	TFileName relPath;
	relPath.Copy(feedInfo->Title());
	relPath.Append(_L("\\"));

	TFileName fileName;
	iPodcastModel.FeedEngine().FileNameFromUrl(info->Url(), fileName);
	relPath.Append(fileName);
	iPodcastModel.FeedEngine().EnsureProperPathName(relPath);
	
	// complete file path is base dir + rel path
	filePath.Append(relPath);
	info->SetFileNameL(filePath);
	iShowClient->GetL(info->Url(), filePath);
	}

TBool CShowEngine::AddShow(CShowInfo *item) 
	{
	const TInt count = iShows.Count();
	for (TInt i=0; i<count; i++) 
		{
		if (iShows[i]->Url().Compare(item->Url()) == 0) 
			{
			return EFalse;
			}
		}
	iShows.Append(item);
	
	return ETrue;
	}

void CShowEngine::AddObserver(MShowEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CShowEngine::RemoveObserver(MShowEngineObserver *observer)
	{
	TInt index = iObservers.Find(observer);
	
	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CShowEngine::CompleteL(CHttpClient* /*aHttpClient*/, TBool aSuccessful)
	{
	RDebug::Print(_L("CShowEngine::Complete\tDownload of file: %S is complete"), &iShowDownloading->FileName());
	
	if (aSuccessful) 
		{
		iShowDownloading->SetDownloadState(EDownloaded);
		iShowsDownloading.Remove(0);

		SaveShows();
		NotifyShowDownloadUpdated(100,0,1);
		}
	else 
		{
		iDownloadErrors++;
		if (iDownloadErrors > 3) 
			{
			RDebug::Print(_L("Too many downloading errors, suspending downloads"));
			iDownloadsSuspended = ETrue;
			}
		}
	DownloadNextShow();
	}

CShowInfo* CShowEngine::ShowDownloading()
	{
		return iShowDownloading;
	}


CShowInfo* CShowEngine::GetShowByUidL(TUint aShowUid)
	{
	CShowInfo* infoToFind = CShowInfo::NewL();
	CleanupStack::PushL(infoToFind);
	infoToFind->SetUid(aShowUid);
	TIdentityRelation<CShowInfo> comparison(CompareShowsByUid);
	TInt index = iShows.Find(infoToFind, comparison);
	CleanupStack::PopAndDestroy(infoToFind);
	if(index != KErrNotFound)
		{
		return iShows[index];
		}

	return NULL;
	}

TBool CShowEngine::CompareShowsByUid(const CShowInfo &a, const CShowInfo &b)
{
	return a.Uid() == b.Uid();
}

void CShowEngine::LoadShowsL()
	{
	RDebug::Print(_L("CShowEngine::LoadShowsL\tLoad shows from database file"));
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KShowDB);
	iFs.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(iFs, privatePath)) 
		{
		RDebug::Print(_L("The show database does not exist"));	
		return;
		}
	
	CFileStore* store = NULL;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) 
		{
		User::Leave(error);
		}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	
	TInt version = instream.ReadInt32L();
	RDebug::Print(_L("CShowEngine::LoadShowsL\tVersion of database file = %d"), version);

	if (version != KShowInfoVersion) 
		{
		RDebug::Print(_L("CShowEngine::LoadShowsL\tWrong version, discarding"));
		User::Leave(KErrGeneral);
		return;
		}
	
	CShowInfo *readData;
	//TUint lastUid = 0;
	TInt count = instream.ReadInt32L();
	RDebug::Print(_L("CShowEngine::LoadShowsL\t%d Shows present in database"), count);
		
	for (TInt i=0 ; i < count ; i++) 
		{
		readData = CShowInfo::NewL();
		instream  >> *readData;
		
		//CFeedInfo *feedInfo = NULL;
		//if (readData->FeedUid() != lastUid) 
		//	{
		//	lastUid = readData->FeedUid();
		//	feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(readData->FeedUid());
		//	}
		
		// might be useful to keep these shows after all...
		/*if (feedInfo == NULL) {
			// if this show does not have a valid feed, we don't bother
			RDebug::Print(_L("Discarding show since it has no feed!"));
			continue;
		}*/
		//RDebug::Print(_L("error: %d"), error);
		TBool isAdded = AddShow(readData);
		
		if (isAdded == EFalse)
			{
			delete readData;
			readData = NULL;
			}
		else
			{
			if ((readData->DownloadState() == EQueued) || (readData->DownloadState() == EDownloading)) 
				{
				readData->SetDownloadState(EQueued);
				iShowsDownloading.Append(readData);
				}
			}
		}
	CleanupStack::PopAndDestroy(2); // instream and store
	}

void CShowEngine::SaveShows()
	{
	RDebug::Print(_L("void CShowEngine::SaveShows\tAttempt to store shows to db."));
	
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
	
	const TInt numberOfShows = iShows.Count();
	RDebug::Print(_L("CShowEngine::SaveShows\tDatabase has %d shows entries"), numberOfShows);
	
	//We need to purge the array for entries that should not be saved.
	RShowInfoArray tempArray;
	CleanupClosePushL(tempArray);
	
	// Move all non delete entries into the new array
	for (TInt j = 0; j < numberOfShows ; j++)
		{
		if (!iShows[j]->Delete())
			{
			tempArray.Append(iShows[j]);
			}
		}
	
	const TInt countTempArray = tempArray.Count();
	RDebug::Print(_L("CShowEngine::SaveShows\tDatabase has %d entries marked for delete"), numberOfShows - countTempArray);	
	RDebug::Print(_L("CShowEngine::SaveShows\tSaving %d show entries"), countTempArray);
	
	outstream.WriteInt32L(countTempArray);
	for (TInt i=0 ; i < countTempArray ; i++) 
		{
		outstream  << *iShows[i];
		}
	
	CleanupStack::PopAndDestroy(&tempArray);
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);	
	}

void CShowEngine::SelectAllShows()
	{
	iSelectedShows.Reset();
	
	const TInt count = iShows.Count();
	for (TInt i=0;i<count;i++)
		{
		iSelectedShows.Append(iShows[i]);
		}
	}

TInt CShowEngine::CompareShowsByDate(const CShowInfo &a, const CShowInfo &b)
	{
	if (a.PubDate() > b.PubDate()) 
		{
//		RDebug::Print(_L("Sorting %S less than %S"), &a.iTitle, &b.iTitle);
		return -1;
		} 
	else if (a.PubDate() == b.PubDate()) 
		{
//		RDebug::Print(_L("Sorting %S equal to %S"), &a.iTitle, &b.iTitle);
		return 0;
		}
	else 
		{
//		RDebug::Print(_L("Sorting %S greater than %S"), &a.iTitle, &b.iTitle);
		return 1;
		}
	}

void CShowEngine::PurgeShowsByFeed(TUint aFeedUid)
	{
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			//if (iShows[i]->DownloadState() == EDownloaded) {
			if (iShows[i]->FileName().Length() > 0) {
				BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
				iShows[i]->SetDownloadState(ENotDownloaded);
			}
			}
		}
	}

void CShowEngine::RemoveAllShowsByFeed(TUint aFeedUid)
	{
	const TInt count = iShows.Count();
	
	for (TInt i=count-1 ; i >= 0; i--)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			if (iShows[i]->FileName().Length() > 0) 
				{
				BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
				iShows[i]->SetDownloadState(ENotDownloaded);
				}
			CShowInfo* show = iShows[i];
			iShows.Remove(i);
			delete show;
			}
		}
	}
	


void CShowEngine::PurgePlayedShows()
	{
	for (int i=0;i<iShows.Count();i++)
	{
		if (iShows[i]->PlayState() == EPlayed) {
			BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
			iShows[i]->SetDownloadState(ENotDownloaded);
			// do we want this?
			//iShows[i]->iPlayState = ENeverPlayed;
		}
	}
}

void CShowEngine::SetSelectUnplayedOnly(TBool aOnlyUnplayed)
	{
	iSelectOnlyUnplayed = aOnlyUnplayed;
	}

TBool CShowEngine::SelectUnplayedOnly()
	{
	return iSelectOnlyUnplayed;
	}


void CShowEngine::PurgeOldShows()
	{
	// TODO: implement
	}

void CShowEngine::PurgeShow(TUint aShowUid)
	{
	const TInt count = iShows.Count();
	for (TInt i=0; i < count ; i++)
		{
		if (iShows[i]->Uid() == aShowUid) 
			{
			BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
			iShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	}

TUint CShowEngine::GetGrossSelectionLength()
	{
	int max = iPodcastModel.SettingsEngine().MaxListItems();
	return (iGrossSelectionLength < max ? iGrossSelectionLength : max);
	}

void CShowEngine::SelectShowsByFeed(TUint aFeedUid)
	{
	RDebug::Print(_L("SelectShowsByFeed: %u, unplayed only=%d"), aFeedUid, iSelectOnlyUnplayed);
	iSelectedShows.Reset();
	iGrossSelectionLength = 0;
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			iGrossSelectionLength++;
			if (!iSelectOnlyUnplayed || (iSelectOnlyUnplayed && iShows[i]->PlayState() == ENeverPlayed) ) {
				iSelectedShows.Append(iShows[i]);
			}
			}
		}
	
	// sort by date falling
	TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByDate);
	iSelectedShows.Sort(sortOrder);
	
	// now purge if more than limit
	int count = iSelectedShows.Count();
	while (count > iPodcastModel.SettingsEngine().MaxListItems()) {
		RDebug::Print(_L("Too many items, Removing"));
		//delete iSelectedShows[count-1];
		iSelectedShows[count-1]->SetDelete();
		iSelectedShows.Remove(count-1);
		count = iSelectedShows.Count();
	}
}

void CShowEngine::SelectNewShows()
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->PlayState() == ENeverPlayed)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	
	TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByDate);
	iSelectedShows.Sort(sortOrder);
	}

void CShowEngine::SelectShowsDownloaded()
	{
	CheckFiles();
	
	iSelectedShows.Reset();
	iGrossSelectionLength = 0;
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EDownloaded)
			{
			iGrossSelectionLength++;
			if (!iSelectOnlyUnplayed || (iSelectOnlyUnplayed && iShows[i]->PlayState() == ENeverPlayed) ) {
				iSelectedShows.Append(iShows[i]);
				}
			}
		}
	}


void CShowEngine::SelectShowsDownloading()
	{
	iSelectedShows.Reset();
	
	/*for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EDownloading)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}

	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EQueued)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}
*/
	const TInt count = iShowsDownloading.Count();
	for (TInt i=0 ; i < count ;i++) 
		{
		iSelectedShows.Append(iShowsDownloading[i]);
		}
	}

RShowInfoArray& CShowEngine::GetSelectedShows()
	{
	return iSelectedShows;
	}

void CShowEngine::AddDownload(CShowInfo *info)
	{
	info->SetDownloadState(EQueued);
	iShowsDownloading.Append(info);
	SaveShows();
	DownloadNextShow();
	}


void CShowEngine::DownloadNextShow()
	{
	// Check if we have anything in the download queue
	const TInt count = iShowsDownloading.Count();
	RDebug::Print(_L("CShowEngine::DownloadNextShow\tTrying to start new download"));
	RDebug::Print(_L("CShowEngine::DownloadNextShow\tShows in download queue %d"), count);
	
	if (count > 0)
		{
		if (iDownloadsSuspended)
			{
			RDebug::Print(_L("CShowEngine::DownloadNextShow\tDownload process is suspended, ABORTING"));
			return;
			}
		else if (iShowClient->IsActive())
			{
			RDebug::Print(_L("CShowEngine::DownloadNextShow\tDownload process is already active."));
			return;
			}
		else
			{
			// Inform the observers
			NotifyDownloadQueueUpdated();
			
			// Start the download
			CShowInfo *info = iShowsDownloading[0];
			RDebug::Print(_L("CShowEngine::DownloadNextShow\tDownloading: %S"), &(info->Title()));
			info->SetDownloadState(EDownloading);
			iShowDownloading = info;
			GetShow(info);
			}
		}
	else
		{
		iShowDownloading = NULL;
		RDebug::Print(_L("CShowEngine::DownloadNextShow\tNothing to download"));
		}
	}


void CShowEngine::NotifyDownloadQueueUpdated()
	{
	const TInt count = iObservers.Count();
	for (TInt i=0;i < count; i++) 
		{
		TRAP_IGNORE(iObservers[i]->DownloadQueueUpdated(1, iShowsDownloading.Count() -1));
		}	
	}


void CShowEngine::NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal)
	{
	const TInt count = iObservers.Count();
	for (TInt i=0; i < count ; i++) 
		{
		TRAP_IGNORE(iObservers[i]->ShowDownloadUpdatedL(aPercentOfCurrentDownload, aBytesOfCurrentDownload, aBytesTotal));
		}
	}

void CShowEngine::SetSelectionPlayed()
	{
	RDebug::Print(_L("CShowEngine::SetSelectionPlayed"));
	const TInt count = iSelectedShows.Count();
	for (TInt i=0 ; i < count ; i++)
		{
		//RDebug::Print(_L("Setting %d played"), iSelectedShows[i]->Uid());
		iSelectedShows[i]->SetPlayState(EPlayed);
		}
	SaveShows();
	}


void CShowEngine::ListDir(TFileName &folder) {
	CDirScan *dirScan = CDirScan::NewLC(iFs);
	//RDebug::Print(_L("Listing dir: %S"), &folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);
	
	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (int i=0;i<dirPtr->Count();i++) {
		const TEntry &entry = (TEntry) (*dirPtr)[i];
		if (entry.IsDir())  {
			TFileName subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDir(subFolder);
		} else {
		if (entry.iName.Right(3).CompareF(_L("mp3")) == 0 ||
			entry.iName.Right(3).CompareF(_L("aac")) == 0 ||
			entry.iName.Right(3).CompareF(_L("wav")) == 0) {
			TFileName fileName;
			fileName.Copy(entry.iName);
			TFileName pathName;
			pathName.Copy(folder);
			pathName.Append(entry.iName);

			TBool exists = EFalse;
			for (int i=0;i<iShows.Count();i++) {
				if (iShows[i]->FileName().Compare(pathName) == 0) {
					//RDebug::Print(_L("Already listed %S"), &pathName);
					exists = ETrue;
					break;
				}
			}
			
			if (exists) {
				continue;
			}
			
			//RDebug::Print(_L("We found a new file: %S"), &fileName);
			
			CShowInfo *info = CShowInfo::NewL();
			info->SetFileNameL(pathName);
			info->SetTitleL(fileName);
			info->SetDownloadState(EDownloaded);
			info->SetUid(DefaultHash::Des16(fileName));
			info->SetFeedUid(0);
			TEntry entry;
			iFs.Entry(pathName, entry);
			info->SetShowSize(entry.iSize);
			info->SetPubDate(entry.iModified);
			info->SetDelete();  			// so that we do not save the entry in the DB.
			
			iShows.Append(info);
			
			// submit the file for meta data reading
			iMetaDataReader->SubmitShow(info);
		}
		}
	}
	delete dirPtr;
	CleanupStack::PopAndDestroy(dirScan);
}

void CShowEngine::CheckFiles()
	{

	// check to see if any files were removed
	for (int i=0;i<iShows.Count();i++) {
		if(iShows[i]->DownloadState() == EDownloaded) {
			if(!BaflUtils::FileExists(iFs, iShows[i]->FileName())) {
				RDebug::Print(_L("%S was removed, delisting"), &iShows[i]->FileName());
				if (iShows[i]->FeedUid() == 0) {
					iShows[i]->SetDelete();
				} 
				iShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	}

	// check if any new files were added
	ListDir(iPodcastModel.SettingsEngine().BaseDir());	
}

void CShowEngine::ReadMetaData(CShowInfo *aShowInfo)
	{
	RDebug::Print(_L("Read %S"), &(aShowInfo->Title()));
	
	//for (int i=0;i<iObservers.Count();i++) {
	//	iObservers[i]->ShowListUpdated();
	//}
	}

void CShowEngine::ReadMetaDataComplete()
	{
	SaveShows();
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->ShowListUpdated();
	}
	}
