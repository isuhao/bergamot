#include <eikedwin.h>
#include <e32hashtab.h>

#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientAddFeedDlg::~CPodcastClientAddFeedDlg()
{
}

TBool CPodcastClientAddFeedDlg::OkToExitL(TInt aCommandId)
{
	if(aCommandId == EEikBidYes)
	{
		return ETrue;
	}
	CEikEdwin* edwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));

	if(edwin != NULL)
	{
		edwin->GetText(iFeedInfo.iUrl);
		if(iFeedInfo.iUrl.Length() <= iFeedInfo.iTitle.MaxLength())
		{
			iFeedInfo.iTitle = iFeedInfo.iUrl;
		}
		else
		{
			iFeedInfo.iTitle = iFeedInfo.iUrl.Left(iFeedInfo.iTitle.MaxLength());
		}

		iFeedInfo.iUid = DefaultHash::Des16(iFeedInfo.iUrl);
	}

	TFeedInfo* newFeedInfo = new (ELeave) TFeedInfo;
	
	*newFeedInfo = iFeedInfo;

	iPodcastModel.FeedEngine().AddFeed(newFeedInfo);
	iPodcastModel.FeedEngine().UpdateFeed(newFeedInfo->iUid);

	return ETrue;
}
