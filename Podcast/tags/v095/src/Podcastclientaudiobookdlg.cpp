#include <eikedwin.h>
#include <eiklabel.h>

#include "PodcastClient.hrh"
#include "PodcastClientAudioBookDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"

const TInt KNumberOfFilesMaxLength = 4;

CPodcastClientAudioBookDlg::CPodcastClientAudioBookDlg(CPodcastModel& aPodcastModel, CDesCArrayFlat& aSelectedFileNames):iPodcastModel(aPodcastModel),iSelectedFilenames(aSelectedFileNames)
	{
	}

CPodcastClientAudioBookDlg::~CPodcastClientAudioBookDlg()
	{
	}

TBool CPodcastClientAudioBookDlg::OkToExitL(TInt aCommandId)
	{
	if(aCommandId == EEikBidOk)
		{
			CEikEdwin* titleEdwin = static_cast<CEikEdwin*>(ControlOrNull(EPodcastNewAudioBookTitle));
			HBufC* title = titleEdwin->GetTextInHBufL();
			CleanupStack::PushL(title);		
			iPodcastModel.FeedEngine().AddBookL(*title, &iSelectedFilenames);
		
			CleanupStack::PopAndDestroy(title);
		}

	return ETrue;
	}

void CPodcastClientAudioBookDlg::PreLayoutDynInitL()
	{
	 CEikLabel* label = static_cast<CEikLabel*>(ControlOrNull(EPodcastNewAudioBookLabel));
	 HBufC* buf = HBufC::NewLC(label->Text().Length()+KNumberOfFilesMaxLength);
	 buf->Des().Format(label->Text(), iSelectedFilenames.Count());
	 label->SetText(*buf);
	 CleanupStack::PopAndDestroy(buf);
	}