#include "PodcastDownloader.h"
#include <Uri8.h>

CPodcastDownloader::CPodcastDownloader() {
	iSession.OpenL();
}

void CPodcastDownloader::Test()
	{	
	RDebug::Print(_L("Test START"));
	TUriParser8 uri; 
    uri.Parse(_L8("http://www.example.com"));
    RHTTPTransaction trans = iSession.OpenTransactionL(uri, *this);
	
    //RHTTPHeaders hdr = trans.Request().GetHeaderCollection();
    trans.SubmitL();
    CActiveScheduler::Start();
	iSession.Close();
	RDebug::Print(_L("Test END"));
	}

void CPodcastDownloader::MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent &aEvent)
	{
	RDebug::Print(_L("MHFRunL"));
	
	}

TInt CPodcastDownloader::MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent &aEvent)
	{
	RDebug::Print(_L("MHFRunError"));	
	}
