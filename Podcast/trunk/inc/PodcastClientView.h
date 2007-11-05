#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <qikviewbase.h>
#include <qikscrollablecontainer.h>
#include "PodcastClient.hrh"
#include "s32file.h"
#include "e32debug.h"
#include <e32cmn.h>
#include "RPodcastServerSession.h"
#include "PodcastClientGlobals.h"
#include <mdaaudiosampleplayer.h>

class CPodcastClientView : public CQikViewBase,  public MMdaAudioPlayerCallback
	{
public:
	static CPodcastClientView* NewLC(CQikAppUi& aAppUi);
	~CPodcastClientView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
    void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
protected: 
	void ViewConstructL();
	
private:
	CPodcastClientView(CQikAppUi& aAppUi);
	void ConstructL();
	void HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType);
	void ListAllPodcastsL();
	void ListDirL(RFs &rfs, TDesC &folder);
	void CreatePodcastListItem(CQikScrollableContainer* container, int id, TPtrC fileName);

private:
	RPodcastServerSession serverSession;
	CQikScrollableContainer* iContainer;
    CMdaAudioPlayerUtility *iPlayer;
    int iControlIdCount;
	};
#endif
