#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>

CFeedEngine::CFeedEngine()
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
	
	iClient->SetSaveFileName(privatePath);
	iClient->StartClientL();
	}

void CFeedEngine::ConnectedCallback()
	{
	
	}

void CFeedEngine::TransactionFinishedCallback()
	{
	
	}

void CFeedEngine::DisconnectedCallback()
	{
	
	}
