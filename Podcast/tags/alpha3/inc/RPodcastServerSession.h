#ifndef RPODCASTSERVERSESSION_H_
#define RPODCASTSERVERSESSION_H_

#include <e32std.h>
#include "CPodcastServerSession.h"

_LIT(KPodcastServerExe, "\\sys\\bin\\PodcastServer.exe");
_LIT(KPodcastServerSemaphore, "KPodcastServerSemaphore");

class RPodcastServerSession : public RSessionBase
{
public:
	TInt Connect();
	void SetTimer(TInt podcastId, TPodcastPeriod period);
	TPodcastPeriod GetTimer(TInt podcastId);
	
private:
	RThread iServerThread;
};

#endif
