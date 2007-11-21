#include <eikedwin.h>
#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"

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
	}

	return EFalse;
}
