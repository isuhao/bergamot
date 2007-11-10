#ifndef __PODCASTDOWNLOADER_H__
#define __PODCASTDOWNLOADER_H__

#include <http\mhttptransactioncallback.h>
#include <http\rhttpsession.h>

class CPodcastDownloader : public MHTTPTransactionCallback
{
public:
	CPodcastDownloader();
	void Test();

public:
	void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent &aEvent);
	TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent &aEvent);

private:
	RHTTPSession iSession;
};
#endif