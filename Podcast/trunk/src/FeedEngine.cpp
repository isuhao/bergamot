#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>

CFeedEngine::CFeedEngine() : parser(*this)
	{
	iClient = CHttpClient::NewL(*this);
	}

CFeedEngine::~CFeedEngine()
	{
	}

void CFeedEngine::GetFeed(TFeedInfo& feedInfo)
	{
	items.Reset();
	TBuf<100> privatePath;
	RFs rfs;
	rfs.Connect();
	rfs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(rfs, privatePath);
	
	int pos = feedInfo.iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedInfo.iUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}
	iFileName.Copy(privatePath);
	iClient->SetUrl(feedInfo.iUrl);
	iClient->SetSaveFileName(privatePath);
	iClient->StartClientL();
//	TFileName fn;
//	fn.Copy(_L("C:\\Private\\2000fb05\\tech5.xml"));
//	parser.ParseFeedL(private);
	RDebug::Print(_L("DownloadFeed END"));
	}

RArray <TPodcastInfo*>& CFeedEngine::GetItems() 
{
	return items;
}

void CFeedEngine::GetPodcast(TPodcastInfo &info)
	{
	TBuf<100> filePath;
	RFs rfs;
	rfs.Connect();
	filePath.Copy(KPodcastDirectory);
	BaflUtils::EnsurePathExistsL(rfs, filePath);
	
	int pos = info.iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = info.iUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown"));
	}
	iClient->SetSaveFileName(filePath);
	iClient->SetUrl(info.iUrl);
	iClient->StartClientL();
	}

void CFeedEngine::Item(TPodcastInfo *item)
	{
	RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription: %S"), &(item->iTitle), &(item->iUrl), &(item->iDescription));
	items.Append(item);
	}

void CFeedEngine::ConnectedCallback()
	{
	
	}

void CFeedEngine::FileCompleteCallback(TFileName &fileName)
	{
	RDebug::Print(_L("File to parse: %S"), &fileName);
	parser.ParseFeedL(fileName);
	
	}
void CFeedEngine::TransactionFinishedCallback()
	{
	}

void CFeedEngine::DisconnectedCallback()
	{
	
	}
