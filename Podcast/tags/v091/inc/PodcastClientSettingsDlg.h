#ifndef PODCASTCLIENTSETTINGSDLGH
#define PODCASTCLIENTSETTINGSDLGH
#include <eikdialg.h>

#include "FeedInfo.h"
class CPodcastModel;
class CEikEdwin;
class CEikCheckBox;
class CEikChoiceList;
class CQikNumberEditor;
class CQikSlider;


/**
 * Dialog (old style) used to do volumd
 */
class CPodcastClientVolumeDlg: public CEikDialog
{
public:
	CPodcastClientVolumeDlg(CPodcastModel& aPodcastModel);
	~CPodcastClientVolumeDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	void PreLayoutDynInitL();
	
	void HandleControlStateChangeL(TInt aControlId);
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;
	CQikSlider* iVolumeSlider;
};


/**
 * Dialog (old style) used to do volumd
 */
class CPodcastClientIAPDlg: public CEikDialog
{
public:
	CPodcastClientIAPDlg(CPodcastModel& aPodcastModel, TInt& aSelectedIAP);
	~CPodcastClientIAPDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	void PreLayoutDynInitL();
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;
	CEikChoiceList* iIAPListCtrl;
	TInt& iSelectedIAP;
};



/**
 * Dialog (old style) used to do settings
 */
class CPodcastClientSettingsDlg: public CEikDialog
{
public:
	CPodcastClientSettingsDlg(CPodcastModel& aPodcastModel);
	~CPodcastClientSettingsDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	 void PreLayoutDynInitL();
	
	 void HandleControlStateChangeL(TInt aControlId);
	 void LineChangedL(TInt aControlId);
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;

	CEikEdwin* iShowBaseDirCtrl;
	CEikChoiceList* iAutoDLCtrl;
	CQikNumberEditor* iUpdateIntervalCtrl;
//	CQikNumberEditor* iMaxSimDlsCtrl;
	CEikChoiceList* iConnectionCtrl;
	CEikChoiceList* iIAPListCtrl;

	TFileName iSelectedPathTemp;
};

#endif
