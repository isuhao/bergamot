#ifndef PODCASTCLIENTVIEW_H
#define PODCASTCLIENTVIEW_H

#include <QikViewBase.h>
#include <QikScrollableContainer.h>
#include "PodcastClient.hrh"
#include "S32FILE.H"
#include "e32debug.h"
#include <e32cmn.h>
#include "RPodcastServerSession.h"
#include "PodcastClientGlobals.h"
#include <MdaAudioSamplePlayer.h>

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

private:
	RPodcastServerSession serverSession;
	CQikScrollableContainer* iContainer;
    CMdaAudioPlayerUtility *iPlayer;
	};
#endif
