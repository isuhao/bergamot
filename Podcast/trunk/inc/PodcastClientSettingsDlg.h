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
	CEikCheckBox* iAutoDLCtrl;
	CQikNumberEditor* iUpdateIntervalCtrl;
	CQikNumberEditor* iMaxSimDlsCtrl;
	CEikChoiceList* iConnectionCtrl;
	CEikChoiceList* iIAPListCtrl;
	CQikSlider* iVolumeSlider;

	TFileName iSelectedPathTemp;
};

#endif