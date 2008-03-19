#include <eikedwin.h>

#include "PodcastClient.hrh"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "ShowEngine.h"
#include <PodcastClient.rsg>

const TInt KMaxTextBuffer = 1024;
_LIT(KURLPrefix, "http://");
_LIT(KItpcPrefix, "itpc://");

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
	iEditFeed = EFalse;
	}

CPodcastClientAddFeedDlg::CPodcastClientAddFeedDlg(CPodcastModel& aPodcastModel, CFeedInfo* aFeedInfo)	
	: iPodcastModel(aPodcastModel), iFeedInfo(aFeedInfo)
	{
	iEditFeed = ETrue;
	}

CPodcastClientAddFeedDlg::~CPodcastClientAddFeedDlg()
	{
	}

void CPodcastClientAddFeedDlg::PreLayoutDynInitL()
	{
	CEikEdwin* edwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));
	
	if (!edwin)
		return;
	
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
	// why are we doing this?
	if(aCommandId == EEikBidYes)
		{
		return ETrue;
		}

	TBool exitDialog = ETrue;

	CEikEdwin* urlEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgUrl));
	CEikEdwin* titleEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastAddEditFeedDlgTitle));

	// url is always present so access that
	HBufC* url = urlEdwin->GetTextInHBufL();
	CleanupStack::PushL(url);
	TPtr urlPtr = url->Des();

	// Some pod links are written in format itpc://mylink.net/podcast.xml
	// Symbian HTTP stack does not like itpc:// 
	// Try to use a HTTP instead.
	TInt p = urlPtr.Find(KItpcPrefix);
	if (p >= 0)
		{
		urlPtr.Delete(p, KItpcPrefix().Length());
		}

	// The URL must start with http://, otherwise the HTTP stack fails.
	TInt pos = urlPtr.Find(KURLPrefix);
	if (pos == KErrNotFound)
		{
		HBufC* newUrl = HBufC::NewL(url->Length() + KURLPrefix().Length());
		TPtr ptr = newUrl->Des();
		ptr.Append(KURLPrefix());
		ptr.Append(*url);
		
		// replace the url buffer
		CleanupStack::PopAndDestroy(url);
		url = newUrl;
		CleanupStack::PushL(url);
		}

	// check which mode we are in.
	if (iEditFeed == EFalse)
		{
		// we are creating a new feed
		CFeedInfo* newFeedInfo = CFeedInfo::NewL();
		CleanupStack::PushL(newFeedInfo);
		newFeedInfo->SetUrlL(*url);
		newFeedInfo->SetTitleL(newFeedInfo->Url());
		CleanupStack::Pop(newFeedInfo);
		
		TBool added = iPodcastModel.FeedEngine().AddFeed(newFeedInfo); // takes ownership
		if (!added)
			{
			CEikonEnv::Static()->InfoWinL(_L("Information"), _L("A feed to this URL already exists") );
			exitDialog = EFalse;
			}
		}
	else
		{
		// editing an existing feed
		
		// we need to check if the URL has changed
		if(iFeedInfo->Url().Compare(*url) != 0)
			{
			// Ask the user if it is OK to remove all shows
			if ( CEikonEnv::Static()->QueryWinL(_L("Remove all shows?"), _L("All shows in this feed will be deleted. Continue?") ))
				{
				
				//----- HACK ---- //
				CFeedInfo* temp = CFeedInfo::NewL();
				temp->SetUrlL(*url);
				TBool added = iPodcastModel.FeedEngine().AddFeed(temp);
				if (added)
					{
					// The Feed URL did not exist
					// Remove the temp entry so that the correct entry could be changed
					iPodcastModel.FeedEngine().RemoveFeed(temp->Uid());	
					
					// user has accepted that shows will be deleted
					iPodcastModel.ShowEngine().RemoveAllShowsByFeed(iFeedInfo->Uid());

					// update URL
					iFeedInfo->SetUrlL(*url);	
				
					// Update the title
					HBufC* title = titleEdwin->GetTextInHBufL();
					CleanupStack::PushL(title);		
							
					if (iFeedInfo->Url().Compare(*title) != 0)
						{
						iFeedInfo->SetTitleL(*title);
						iFeedInfo->SetCustomTitle();	
						}
					CleanupStack::PopAndDestroy(title);
					exitDialog = ETrue;	
					}
				else
					{
					// the feed existed. Object deleted in AddFeed.	
					CEikonEnv::Static()->InfoWinL(_L("Information"), _L("A feed to this URL already exists") );
					exitDialog = EFalse;
					}
				// --- END HACK ---
				}
			else
				{
				// abort
				exitDialog = EFalse;
				}
			}
		}
		
	CleanupStack::PopAndDestroy(url);
	return exitDialog;
	}