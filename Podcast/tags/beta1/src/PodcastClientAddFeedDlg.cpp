#include <eikedwin.h>
#include <e32hashtab.h>

#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include <PodcastClient.rsg>

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
	 iEditFeed = EFalse;
}

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel, CFeedInfo* aFeedInfo):iPodcastModel(aPodcastModel), iFeedInfo(aFeedInfo)
{
	iEditFeed = ETrue;
}

CPodcastClientAddFeedDlg::~CPodcastClientAddFeedDlg()
{
}

_LIT(KURLPrefix, "http://");
void CPodcastClientAddFeedDlg::PreLayoutDynInitL()
{
		CEikEdwin* edwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));

		if(iEditFeed)
		{
			edwin->SetTextL(&iFeedInfo->Url());
		}
		else
		{
			edwin->SetTextL(&KURLPrefix());
		}
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
		TBuf<1024> buffer;
		edwin->GetText(buffer);
	

		if(iEditFeed)
		{
			iFeedInfo->SetUrl(buffer);			
		}
		else
		{
			CFeedInfo* newFeedInfo = new (ELeave) CFeedInfo;
			iFeedInfo = newFeedInfo;
			iFeedInfo->SetUrl(buffer);
			iFeedInfo->SetTitle(newFeedInfo->Url());
			iPodcastModel.FeedEngine().AddFeed(newFeedInfo);
		}
		/*
		if(iEikonEnv->QueryWinL(R_PODCAST_UPDATE_NEW_FEED_TITLE, R_PODCAST_UPDATE_NEW_FEED_PROMPT))
			{
				iPodcastModel.FeedEngine().UpdateFeed(iFeedInfo->Uid());
			}
		*/
	}

	return ETrue;
}
