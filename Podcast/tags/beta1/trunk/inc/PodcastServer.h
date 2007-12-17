#ifndef PODCASTSERVER_H_
#define PODCASTSERVER_H_

#include <e32base.h>
#include "PodcastData.h"

const TUint KDefaultHeapSize=0x10000;
const TUint KDefaultMessageSlots=4;

_LIT(KConfigFileName, "config.dat");
_LIT(KPodcastServerName, "PodcastServer");

enum TPodcastServerPanic
{
	EBadRequest,
	EGetTimer,
	ESetTimer,
	EMainSchedulerError,
	ECreateTrapCleanup,
	EStartServer
};

class CPodcastServer : public CServer2
{
public:
	CPodcastServer(CActive::TPriority aActiveObjectPriority);
	CSession2 * NewSessionL(const TVersion& aVersion,
	const RMessage2& aMessage) const;
	static TInt ThreadFunction(TAny *aParam);
	static void PanicServer(TPodcastServerPanic aPanic);
public:
	// methods

public:
	void LoadSettingsL();
	void SaveSettings();
	
private:
	void RunL();
	RArray<CPodcastData> timerArray;
};

#endif
