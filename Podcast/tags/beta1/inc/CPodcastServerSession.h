#ifndef CPODCASTSERVERSESSION_H_
#define CPODCASTSERVERSESSION_H_

#include "PodcastServer.h"
#include <e32base.h>

enum TPodcastServerMessage {
	ETest
};

class CPodcastServerSession : public CSession2
{
public:
	void ServiceL(const RMessage2& aMessage);
	void PanicClient(const RMessage2& aMessage,TInt aPanic) const;
private:
	const CPodcastServer *iServer;
};

#endif
