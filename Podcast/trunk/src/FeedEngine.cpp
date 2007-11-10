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

void CFeedEngine::DownloadFeed(TDesC &feedUrl)
	{
	iClient->SetUrl(feedUrl);

	TBuf<100> privatePath;
	RFs rfs;
	rfs.Connect();
	rfs.PrivatePath(privatePath);
	RDebug::Print(_L("PrivatePath: %S"), &privatePath);
	BaflUtils::EnsurePathExistsL(rfs, privatePath);
	
	int pos = feedUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) {
		TPtrC str = feedUrl.Mid(pos+1);
		RDebug::Print(_L("Separated filename: %S"), &str);
		privatePath.Append(str);
	} else {
		privatePath.Append(_L("unknown"));
	}
	iFileName.Copy(privatePath);
	iClient->SetSaveFileName(privatePath);
//	iClient->StartClientL();
	TFileName fn;
	fn.Copy(_L("C:\\Private\\2000fb05\\tech5.xml"));
	parser.ParseFeedL(fn);
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
