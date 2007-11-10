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
	TBuf<100> privatePath;
	RFs rfs;
	rfs.Connect();
	rfs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(rfs, privatePath);
	
	int pos = feedInfo.iUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}
	iFileName.Copy(privatePath);
	iClient->SetUrl(feedInfo.iUrl);
	iClient->SetSaveFileName(privatePath);
//	iClient->StartClientL();
	TFileName fn;
	fn.Copy(_L("C:\\Private\\2000fb05\\tech5.xml"));
	parser.ParseFeedL(fn);
	RDebug::Print(_L("DownloadFeed END"));
	}

void CFeedEngine::GetPodcast(TDesC &fileUrl)
	{
	TBuf<100> filePath;
	RFs rfs;
	rfs.Connect();
	filePath.Copy(KPodcastDirectory);
	BaflUtils::EnsurePathExistsL(rfs, filePath);
	
	int pos = fileUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = fileUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		filePath.Append(str);
	} else {
		filePath.Append(_L("unknown"));
	}
	iClient->SetSaveFileName(filePath);
	iClient->SetUrl(fileUrl);
	iClient->StartClientL();
	}

void CFeedEngine::Item(TPodcastItem *item)
	{
	RDebug::Print(_L("\nTitle: %S\nURL: %S\nDescription: %S"), &(item->iTitle), &(item->iUrl), &(item->iDescription));
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
