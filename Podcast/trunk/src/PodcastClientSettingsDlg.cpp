#include <eikedwin.h>
#include <e32hashtab.h>
#include <eikchlst.h>
#include <QikNumberEditor.h>
#include <eikchkbx.h>
#include <QikSelectFolderDlg.h>
#include <QikCommandManager.h>
#include <QikSlider.h>

#include "PodcastClient.hrh"
#include "PodcastClientSettingsDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SettingsEngine.h"


CPodcastClientIAPDlg::CPodcastClientIAPDlg(CPodcastModel& aPodcastModel, TInt& aSelectedIAP):iPodcastModel(aPodcastModel), iSelectedIAP(aSelectedIAP)
{
}

CPodcastClientIAPDlg::~CPodcastClientIAPDlg()
{
}

void CPodcastClientIAPDlg::PreLayoutDynInitL()
{
	iPodcastModel.UpdateIAPListL();
	iIAPListCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingIAPList));
	iIAPListCtrl->SetArrayL((MDesCArray*) NULL);
	iIAPListCtrl->SetArrayExternalOwnership(ETrue);
	iIAPListCtrl->SetArrayL(iPodcastModel.IAPNames());
	iIAPListCtrl->SetFocus(ETrue);
	
	TInt cnt = iPodcastModel.IAPIds().Count();
	for(TInt loop = 0;loop<cnt;loop++)
	{
		if(iPodcastModel.IAPIds()[loop].iIapId == iSelectedIAP)
		{
			iIAPListCtrl->SetCurrentItem(loop);
			break;
		}
		
	}
}


TBool CPodcastClientIAPDlg::OkToExitL(TInt /*aCommandId*/)
{
	iSelectedIAP = iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId;
	return ETrue;
}

CPodcastClientVolumeDlg::CPodcastClientVolumeDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientVolumeDlg::~CPodcastClientVolumeDlg()
{
}

void CPodcastClientVolumeDlg::PreLayoutDynInitL()
{
	iVolumeSlider =	 static_cast<CQikSlider*> (ControlOrNull(EPodcastSettingsVolume));
	iVolumeSlider->SetValue(iPodcastModel.SettingsEngine().Volume()/KVolumeDelta);
}

TBool CPodcastClientVolumeDlg::OkToExitL(TInt /*aCommandId*/)
{
	iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue()*KVolumeDelta);

	return ETrue;
}

void CPodcastClientVolumeDlg::HandleControlStateChangeL(TInt aControlId)
{
	CEikDialog::HandleControlStateChangeL(aControlId);

	if(EPodcastSettingsVolume == aControlId)
	{
		iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue()*KVolumeDelta);
	}
}

enum
{
	EUseDefaultAccount,
	EUseWLANOnly,
	EUseSpecifiedIAP
};

enum
{
	ENoAutoDownload,
	EFeedsOnly,
	EFeedsAndShows
};

CPodcastClientSettingsDlg::CPodcastClientSettingsDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientSettingsDlg::~CPodcastClientSettingsDlg()
{
}

void CPodcastClientSettingsDlg::PreLayoutDynInitL()
{
	// First init control pointers
	iShowBaseDirCtrl = static_cast<CEikEdwin*> (ControlOrNull(EPodcastSettingShowDir));
	iAutoDLCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingAutoDL));
	iUpdateIntervalCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingUpdateInterval));
//	iMaxSimDlsCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingMaxsimdls));
	iConnectionCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingConnection));
	iIAPListCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingIAPList));
	iIAPListCtrl->SetArrayL((MDesCArray*) NULL);
	iIAPListCtrl->SetArrayExternalOwnership(ETrue);
	iIAPListCtrl->SetArrayL(iPodcastModel.IAPNames());

	// Populate data
	iSelectedPathTemp = iPodcastModel.SettingsEngine().BaseDir();
	iShowBaseDirCtrl->SetTextL(&iSelectedPathTemp);

	iAutoDLCtrl->SetCurrentItem(iPodcastModel.SettingsEngine().DownloadAutomatically());
	iUpdateIntervalCtrl->SetValueL(iPodcastModel.SettingsEngine().UpdateFeedInterval());

	if (iPodcastModel.SettingsEngine().DownloadAutomatically() == 0) {
		SetLineDimmedNow(EPodcastSettingUpdateInterval, ETrue);
	} else {
		SetLineDimmedNow(EPodcastSettingUpdateInterval, EFalse);
	}
//	iMaxSimDlsCtrl->SetValueL(iPodcastModel.SettingsEngine().MaxSimultaneousDownloads());

	if(iPodcastModel.SettingsEngine().SpecificIAP() == -1)
	{
		iConnectionCtrl->SetCurrentItem(EUseWLANOnly);
		SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
		//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
	}
	else if(iPodcastModel.SettingsEngine().SpecificIAP() > 0)
	{
		iConnectionCtrl->SetCurrentItem(EUseSpecifiedIAP);
		//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
		SetLineDimmedNow(EPodcastSettingIAPList, EFalse);
		TInt cnt = iPodcastModel.IAPIds().Count();
		for(TInt loop = 0;loop<cnt;loop++)
		{
			if(iPodcastModel.IAPIds()[loop].iIapId == iPodcastModel.SettingsEngine().SpecificIAP())
			{
				iIAPListCtrl->SetCurrentItem(loop);
				break;
			}

		}
	}
	else
	{
		iConnectionCtrl->SetCurrentItem(EUseDefaultAccount);
		//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
		SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
	}
}


TBool CPodcastClientSettingsDlg::OkToExitL(TInt aCommandId)
{
	switch(aCommandId)
	{
	case EEikBidOk:
		{
			iShowBaseDirCtrl->GetText(iSelectedPathTemp);
			iPodcastModel.SettingsEngine().SetBaseDir(iSelectedPathTemp);
			iPodcastModel.SettingsEngine().SetDownloadAutomatically((TAutoDownloadSetting) iAutoDLCtrl->CurrentItem());
			iPodcastModel.SettingsEngine().SetUpdateFeedInterval(iUpdateIntervalCtrl->Value());
//			iPodcastModel.SettingsEngine().SetMaxSimultaneousDownloads(iMaxSimDlsCtrl->Value());
			/*if(iVolumeSlider != NULL)
			{
				iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue());
			}*/

			switch(iConnectionCtrl->CurrentItem())
			{
			case EUseDefaultAccount:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(0);
					iPodcastModel.SetIap(0);
				}break;
			case EUseWLANOnly:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(-1);
					iPodcastModel.SetIap(-1);
				}break;
			case EUseSpecifiedIAP:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId);
					iPodcastModel.SetIap(iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId);
				}
				break;
			}
			iPodcastModel.SettingsEngine().SaveSettingsL();
			
		}break;
	case EPodcastSelectShowDir:
		{
			TFileName newTemp = iSelectedPathTemp;
			if(CQikSelectFolderDlg::RunDlgLD(newTemp, &iSelectedPathTemp))
			{
				iSelectedPathTemp = newTemp;
				iShowBaseDirCtrl->SetTextL(&iSelectedPathTemp);
				iShowBaseDirCtrl->HandleTextChangedL();
			}
			return EFalse;
		}
	}
	return ETrue;
}

void CPodcastClientSettingsDlg::HandleControlStateChangeL(TInt aControlId)
{
	CEikDialog::HandleControlStateChangeL(aControlId);

	if(EPodcastSettingConnection == aControlId)
	{
		switch(iConnectionCtrl->CurrentItem())
		{
		case EUseSpecifiedIAP:
			{
			SetLineDimmedNow(EPodcastSettingIAPList, EFalse);
			//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
			}
			break;
		default:
			{
			SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
			//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
			}break;
		}
	} else if (EPodcastSettingAutoDL == aControlId) {
		switch(iAutoDLCtrl->CurrentItem())
		{
		case ENoAutoDownload:
			{
			SetLineDimmedNow(EPodcastSettingUpdateInterval, ETrue);
			//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
			}
			break;
		default:
			{
			SetLineDimmedNow(EPodcastSettingUpdateInterval, EFalse);
			//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
			}break;
		}
	}
}

void CPodcastClientSettingsDlg::LineChangedL(TInt aControlId)
{
	CEikDialog::LineChangedL(aControlId);

	CQikCommandManager& comMan = CQikCommandManager::Static();

	if(aControlId == EPodcastSettingShowDir)
	{
		comMan.SetAvailable(*this, EPodcastSelectShowDir, ETrue);
	}
	else
	{
		comMan.SetAvailable(*this, EPodcastSelectShowDir, EFalse);
	}

}

