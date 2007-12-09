#ifndef PODCASTCLIENTADDFEEDDLGH
#define PODCASTCLIENTADDFEEDDLGH
#include <eikdialg.h>

#include "FeedInfo.h"
class CPodcastModel;

/**
 * Dialog (old style) used to input a new Feed url, test this and give use chance to edit if checking fails
 */
class CPodcastClientAddFeedDlg: public CEikDialog
{
public:
	CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel);
	~CPodcastClientAddFeedDlg();
	TBool OkToExitL(TInt aCommandId);
	 void PreLayoutDynInitL();
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;
	CFeedInfo iFeedInfo;
};

#endif
