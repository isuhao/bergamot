#include <eikedwin.h>
#include <e32hashtab.h>

#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include <PodcastClient.rsg>
const TInt KMaxTextBuffer = 1024;
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
		static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgTitle))->SetTextL(&iFeedInfo->Title());
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

	CEikEdwin* urlEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));
	CEikEdwin* titleEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgTitle));

	HBufC* title = NULL;

	if(titleEdwin != NULL)
		{
		title = titleEdwin->GetTextInHBufL();
		}
	
	CleanupStack::PushL(title);

	if(urlEdwin != NULL)
		{
		TBuf<KMaxTextBuffer> buffer;
		urlEdwin->GetText(buffer);	

		if(iEditFeed)
			{
			iFeedInfo->SetUrlL(buffer);	
			if(title && title->Length()>0)
				{
				iFeedInfo->SetTitleL(*title);
				}
			}
		else
			{
			CFeedInfo* newFeedInfo = CFeedInfo::NewL();
			iFeedInfo = newFeedInfo;
			iFeedInfo->SetUrlL(buffer);

			if(title && title->Length()>0)
				{
				iFeedInfo->SetTitleL(*title);
				}
			else
				{
				iFeedInfo->SetTitleL(newFeedInfo->Url());
				}
			iPodcastModel.FeedEngine().AddFeed(newFeedInfo);
			}
	
		/*
		if(iEikonEnv->QueryWinL(R_PODCAST_UPDATE_NEW_FEED_TITLE, R_PODCAST_UPDATE_NEW_FEED_PROMPT))
			{
				iPodcastModel.FeedEngine().UpdateFeed(iFeedInfo->Uid());
			}
		*/
	}

	CleanupStack::PopAndDestroy(title);
	return ETrue;
}
