#ifndef PODCASTPLAYVIEW_H
#define PODCASTPLAYVIEW_H

#include <qikviewbase.h>
#include <qikscrollablecontainer.h>
#include "PodcastClient.hrh"
#include "s32file.h"
#include "e32debug.h"
#include <e32cmn.h>

#include "PodcastClientGlobals.h"
#include "HttpClient.h"
#include "FeedEngine.h"

#include <coecobs.h>
#include <mqiklistboxobserver.h>
class CEikLabel;
class CEikEdwin;
class CPodcastModel;
class CQikSlider;

class CPodcastClientPlayView : public CQikViewBase
	{
public:
	static CPodcastClientPlayView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientPlayView();
	TVwsViewId ViewId()const;
	void HandleCommandL(CQikCommand& aCommand);
  protected: 
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	void UpdateViewL();
private:
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
	CPodcastClientPlayView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void ConstructL();
private:
	CPodcastModel& iPodcastModel;
	CQikSlider* iProgress;
//	CEikLabel* iTitleLabel;
	CEikLabel* iTimeLabel;
	CEikEdwin* iInformationEdwin;
	CQikSlider* iVolumeSlider;
	};
#endif // PODCASTPLAYVIEW_H
