#include "ShowEngine.h"
#include "FeedEngine.h"
#include "FeedInfo.h"
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"

CShowEngine::CShowEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
{
}

CShowEngine::~CShowEngine()
	{
	iShowsDownloading.Close();
	iShows.ResetAndDestroy();
	iShows.Close();
	iFs.Close();
	delete iShowClient;
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
	iShowClient = CHttpClient::NewL(*this);
	LoadShows();
	iLinearOrder = new TLinearOrder<TShowInfo>(CShowEngine::CompareShowsByDate);
	ListAllFiles();
	}

void CShowEngine::StopDownloads() 
	{
	RDebug::Print(_L("StopDownloads"));
	}

void CShowEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}

void CShowEngine::Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes)
{	
	int percent = -1;
	if (aTotalBytes != -1) {
		percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
	}
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->ShowDownloadUpdatedL(percent, aBytes, aTotalBytes);
	}
}

void CShowEngine::Disconnected(CHttpClient* aClient)
	{
	
	}

void CShowEngine::DownloadInfo(CHttpClient* aHttpClient, int aTotalBytes)
	{
	RDebug::Print(_L("About to download %d bytes"), aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->iShowSize = aTotalBytes;
		}
	}


void CShowEngine::GetShow(TShowInfo *info)
	{
	TFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(info->iFeedUid);
	TFileName filePath;
	iPodcastModel.FeedEngine().GetFeedDir(feedInfo, filePath);
	TFileName fileName;
	MakeFileNameFromUrl(info->iUrl, fileName);
	
	filePath.Append(fileName);

	RDebug::Print(_L("filePath: %S"), &filePath);
	info->iFileName.Copy(filePath);
	iShowClient->GetL(info->iUrl, info->iFileName);
	}

void CShowEngine::AddShow(TShowInfo *item) {
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->iUrl.Compare(item->iUrl) == 0) {
			return;
		}
	}
	iShows.Append(item);
	}

void CShowEngine::MakeFileNameFromUrl(TDesC &aUrl, TFileName &fileName)
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


void CShowEngine::AddObserver(MShowEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CShowEngine::Complete(CHttpClient *aHttpClient, TBool aSuccessful)
	{
	RDebug::Print(_L("File %S complete"), &iShowDownloading->iFileName);
	
	if (aSuccessful) {
		iShowDownloading->iDownloadState = EDownloaded;
	}

	SaveShows();
	for (int i=0;i<iObservers.Count();i++) {
			iObservers[i]->ShowDownloadUpdatedL(100,0,1);		
		}
	DownloadNextShow();
	}

TShowInfo* CShowEngine::ShowDownloading()
	{
		return iShowDownloading;
	}

void CShowEngine::LoadShows()
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
		
		if (readData->iDownloadState == EQueued) {
			iShowsDownloading.Append(readData);
		} else if (readData->iDownloadState == EDownloaded) {
			if (!BaflUtils::FileExists(iFs, readData->iFileName)) {
				readData->iDownloadState = ENotDownloaded;
			}
		}
	}
	exit_point:
	CleanupStack::PopAndDestroy(); // instream
	
	CleanupStack::PopAndDestroy(store);	
	}

void CShowEngine::SaveShows()
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

void CShowEngine::ListDir(RFs &rfs, TDesC &folder, TShowInfoArray &files) {
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


void CShowEngine::SelectAllShows()
	{
	iSelectedShows.Reset();
	
	for (int i=0;i<iShows.Count();i++)
		{
		iSelectedShows.Append(iShows[i]);
		}
	}

TInt CShowEngine::CompareShowsByDate(const TShowInfo &a, const TShowInfo &b)
	{
		if (a.iPubDate > b.iPubDate) {
//			RDebug::Print(_L("Sorting %S less than %S"), &a.iTitle, &b.iTitle);
			return -1;
		} else if (a.iPubDate == b.iPubDate) {
//			RDebug::Print(_L("Sorting %S equal to %S"), &a.iTitle, &b.iTitle);
			return 0;
		} else {
//			RDebug::Print(_L("Sorting %S greater than %S"), &a.iTitle, &b.iTitle);
			return 1;
		}
	}

void CShowEngine::SelectShowsByFeed(TInt aFeedUid)
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iFeedUid == aFeedUid)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	
	iSelectedShows.Sort(*iLinearOrder);
	}

void CShowEngine::SelectNewShows()
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iPlayState == ENeverPlayed)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	
	iSelectedShows.Sort(*iLinearOrder);
	}

void CShowEngine::SelectShowsByDownloadState(TInt aDownloadState)
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iDownloadState == aDownloadState)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	}


void CShowEngine::SelectShowsDownloading()
	{
	iSelectedShows.Reset();
	
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iDownloadState == EDownloading)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}

	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->iDownloadState == EQueued)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}

	}

TShowInfoArray& CShowEngine::GetSelectedShows()
	{
	return iSelectedShows;
	}

void CShowEngine::ListAllFiles()
	{
	TBuf<100> podcastDir;
	podcastDir.Copy(KPodcastDir);
	TBool changed = EFalse;
	RDebug::Print(_L("Checking if files still exist..."));
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->iDownloadState == EDownloaded) {
			if(BaflUtils::FileExists(iFs, iShows[i]->iFileName) == EFalse) {
				RDebug::Print(_L("%S was removed, marking"), &iShows[i]->iFileName);
				iShows[i]->iDownloadState = ENotDownloaded;
				changed = ETrue;
			}
		}
	}
	RDebug::Print(_L("Checking complete"));
	//ListDir(iFs, podcastDir, iShows);
	if (changed) {
		SaveShows();
	}
}

void CShowEngine::AddDownload(TShowInfo *info)
	{
	RDebug::Print(_L("AddDownload START"));
	info->iDownloadState = EQueued;
	iShowsDownloading.Append(info);
	DownloadNextShow();
	}


void CShowEngine::DownloadNextShow()
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
