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


CPodcastClientVolumeDlg::CPodcastClientVolumeDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientVolumeDlg::~CPodcastClientVolumeDlg()
{
}

void CPodcastClientVolumeDlg::PreLayoutDynInitL()
{
	iVolumeSlider =	 static_cast<CQikSlider*> (ControlOrNull(EPodcastSettingsVolume));
	iVolumeSlider->SetValue(iPodcastModel.SettingsEngine().Volume()*KVolumeDelta);
}

TBool CPodcastClientVolumeDlg::OkToExitL(TInt aCommandId)
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
	iAutoDLCtrl = static_cast<CEikCheckBox*> (ControlOrNull(EPodcastSettingAutoDL));
	iUpdateIntervalCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingUpdateInterval));
//	iMaxSimDlsCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingMaxsimdls));
	iConnectionCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingConnection));
	iIAPListCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingIAPList));
	iVolumeSlider =	 static_cast<CQikSlider*> (ControlOrNull(EPodcastSettingsVolume));

	// Populate data
	iSelectedPathTemp = iPodcastModel.SettingsEngine().BaseDir();
	iShowBaseDirCtrl->SetTextL(&iSelectedPathTemp);

	iAutoDLCtrl->SetState(iPodcastModel.SettingsEngine().DownloadAutomatically() ? CEikButtonBase::ESet : CEikButtonBase::EClear);
	iUpdateIntervalCtrl->SetValueL(iPodcastModel.SettingsEngine().UpdateFeedInterval());
//	iMaxSimDlsCtrl->SetValueL(iPodcastModel.SettingsEngine().MaxSimultaneousDownloads());
	iVolumeSlider->SetValue(iPodcastModel.SettingsEngine().Volume());

	if(iPodcastModel.SettingsEngine().DownloadOnlyOnWLAN())
	{
		iConnectionCtrl->SetCurrentItem(EUseWLANOnly);
		MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
	}
	else if(iPodcastModel.SettingsEngine().SpecificIAP() > 0)
	{
		iConnectionCtrl->SetCurrentItem(EUseSpecifiedIAP);
		MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
	}
	else
	{
		iConnectionCtrl->SetCurrentItem(EUseDefaultAccount);
		MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
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
			iPodcastModel.SettingsEngine().SetDownloadAutomatically(iAutoDLCtrl->State() == CEikButtonBase::ESet ? ETrue : EFalse);
			iPodcastModel.SettingsEngine().SetUpdateFeedInterval(iUpdateIntervalCtrl->Value());
//			iPodcastModel.SettingsEngine().SetMaxSimultaneousDownloads(iMaxSimDlsCtrl->Value());
			iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue());

			switch(iConnectionCtrl->CurrentItem())
			{
			case EUseDefaultAccount:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(0);
					iPodcastModel.SettingsEngine().SetDownloadOnlyOnWLAN(EFalse);
				}break;
			case EUseWLANOnly:
				{
					iPodcastModel.SettingsEngine().SetDownloadOnlyOnWLAN(ETrue);
				}break;
			case EUseSpecifiedIAP:
				{
					iPodcastModel.SettingsEngine().SetDownloadOnlyOnWLAN(EFalse);
					// Now also set a listed IAP 
				}
				break;
			}
			
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

	if(EPodcastSettingsVolume == aControlId)
	{
		iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue());
	}

	if(EPodcastSettingConnection == aControlId)
	{
		switch(iConnectionCtrl->CurrentItem())
		{
		case EUseSpecifiedIAP:
			{
			MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
			}
			break;
		default:
			{
			MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
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

