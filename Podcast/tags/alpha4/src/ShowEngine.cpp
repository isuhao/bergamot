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
	iLinearOrder = new TLinearOrder<CShowInfo>(CShowEngine::CompareShowsByDate);
	}

void CShowEngine::StopDownloads() 
	{
	RDebug::Print(_L("StopDownloads"));
	iShowClient->Stop();
	}

void CShowEngine::ResumeDownloads() 
	{
	RDebug::Print(_L("ResumeDownloads"));
	DownloadNextShow();
	}

void CShowEngine::RemoveDownload(TInt aUid) 
	{
	RDebug::Print(_L("RemoveFromDownloadQueue"));
	for (int i=0;i<iShowsDownloading.Count();i++) {
		if (iShowsDownloading[i]->Uid() == aUid) {
			iShowsDownloading.Remove(i);
			for (int j=0;j<iObservers.Count();j++) {
				iObservers[j]->DownloadQueueUpdated(1, iShowsDownloading.Count()-1);
			}
			return;
		}
	}
	RDebug::Print(_L("Could not find downloading show to remove"));
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
		iShowDownloading->SetShowSize(aTotalBytes);
		}
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
	info->SetFileName(filePath);
	iShowClient->GetL(info->Url(), filePath);
	}

void CShowEngine::AddShow(CShowInfo *item) {
	for (int i=0;i<iShows.Count();i++) {
		if (iShows[i]->Url().Compare(item->Url()) == 0) {
			return;
		}
	}
	
	iShows.Append(item);

	if (!iSuppressAutoDownload && iPodcastModel.SettingsEngine().DownloadAutomatically()) {
		AddDownload(item);
	}
	}

void CShowEngine::AddObserver(MShowEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CShowEngine::Complete(CHttpClient *aHttpClient, TBool aSuccessful)
	{
	RDebug::Print(_L("File %S complete"), &iShowDownloading->FileName());
	
	if (aSuccessful) {
		iShowDownloading->SetDownloadState(EDownloaded);
	}

	SaveShows();
	for (int i=0;i<iObservers.Count();i++) {
			iObservers[i]->ShowDownloadUpdatedL(100,0,1);		
		}
	DownloadNextShow();
	}

CShowInfo* CShowEngine::ShowDownloading()
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
		CleanupStack::PopAndDestroy(2); // instream and store
		return;
	}
	
	int count = instream.ReadInt32L();
	RDebug::Print(_L("Read count: %d"), count);
	CShowInfo *readData;
	
	iSuppressAutoDownload = ETrue;
	CFeedInfo *feedInfo = NULL;
	int lastUid = -1;
	for (int i=0;i<count;i++) {
		readData = CShowInfo::NewL();
		TRAPD(error, instream  >> *readData);
		
		if (readData->FeedUid() != lastUid) {
			lastUid = readData->FeedUid();
			feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(readData->FeedUid());
		}
		
		// if this show does not have a valid feed, we don't bother
		if (feedInfo == NULL) {
			RDebug::Print(_L("Discarding show since it has no feed!"));
			continue;
		}
		//RDebug::Print(_L("error: %d"), error);
		AddShow(readData);
		
		if (readData->DownloadState() == EQueued) {
			AddDownload(readData);
		} else if (readData->DownloadState() == EDownloaded) {
			if (!BaflUtils::FileExists(iFs, readData->FileName())) {
				readData->SetDownloadState(ENotDownloaded);
			}
		}
	}

	iSuppressAutoDownload = EFalse;
	CleanupStack::PopAndDestroy(2); // instream and store
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

void CShowEngine::SelectAllShows()
	{
	iSelectedShows.Reset();
	
	for (int i=0;i<iShows.Count();i++)
		{
		AppendToSelection(iShows[i]);
		}
	}

TInt CShowEngine::CompareShowsByDate(const CShowInfo &a, const CShowInfo &b)
	{
		if (a.PubDate() > b.PubDate()) {
//			RDebug::Print(_L("Sorting %S less than %S"), &a.iTitle, &b.iTitle);
			return -1;
		} else if (a.PubDate() == b.PubDate()) {
//			RDebug::Print(_L("Sorting %S equal to %S"), &a.iTitle, &b.iTitle);
			return 0;
		} else {
//			RDebug::Print(_L("Sorting %S greater than %S"), &a.iTitle, &b.iTitle);
			return 1;
		}
	}

void CShowEngine::PurgeShowsByFeed(TInt aFeedUid)
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

void CShowEngine::PurgeOldShows()
	{
	// TODO: implement
	}

void CShowEngine::PurgeShow(TInt aShowUid)
	{
	for (int i=0;i<iShows.Count();i++)
		{
			if (iShows[i]->Uid() == aShowUid) {
				BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
				iShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	}

void CShowEngine::SelectShowsByFeed(TUint aFeedUid)
	{
	RDebug::Print(_L("SelectShowsByFeed: %u"), aFeedUid);
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			AppendToSelection(iShows[i]);
			}
		}
	
	iSelectedShows.Sort(*iLinearOrder);
	}

void CShowEngine::SelectNewShows()
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->PlayState() == ENeverPlayed)
			{
			AppendToSelection(iShows[i]);
			}
		}
	
	iSelectedShows.Sort(*iLinearOrder);
	}

void CShowEngine::SelectShowsByDownloadState(TInt aDownloadState)
	{
	iSelectedShows.Reset();
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == aDownloadState)
			{
			AppendToSelection(iShows[i]);
			}
		}
	}


void CShowEngine::SelectShowsDownloading()
	{
	iSelectedShows.Reset();
	
	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EDownloading)
				{
				AppendToSelection(iShows[i]);
				}
		}

	for (int i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EQueued)
				{
				AppendToSelection(iShows[i]);
				}
		}

	}

void CShowEngine::AppendToSelection(CShowInfo *aInfo)
	{
	if (iSelectedShows.Count() > iPodcastModel.SettingsEngine().MaxListItems()) {
		return;
	}
	
	iSelectedShows.Append(aInfo);
	}

CShowInfoArray& CShowEngine::GetSelectedShows()
	{
	return iSelectedShows;
	}

void CShowEngine::AddDownload(CShowInfo *info)
	{
	info->SetDownloadState(EQueued);
	iShowsDownloading.Append(info);
	DownloadNextShow();
	}


void CShowEngine::DownloadNextShow()
{
	if (iShowClient->IsActive()) {
		//RDebug::Print(_L("Show client busy..."));
		return;
	}

	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->DownloadQueueUpdated(1, iShowsDownloading.Count() -1);
	}
	
	if (iShowsDownloading.Count() > 0) {
		CShowInfo *info = iShowsDownloading[0];
		iShowsDownloading.Remove(0);
		RDebug::Print(_L("Downloading %S"), &(info->Title()));
		info->SetDownloadState(EDownloading);
		iShowDownloading = info;
		GetShow(info);
	}
	else {
		iShowDownloading = NULL;
	}
}
