#ifndef SYNCSETTING_H
#define SYNCSETTING_H

#include <aknsettingitemlist.h>
#include "RSyncServerSession.h"

class CSyncSetting: public CAknEnumeratedTextPopupSettingItem 
{ 
public:
	CSyncSetting(TInt aResourceId, TInt aData, RSyncServerSession &aSession);
    ~CSyncSetting();

    void EditItemL(TBool aCalledFromMenu);
	void HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType);

	void CompleteConstructionL();
	
	TInt HandleSetting();
protected:
    TInt iStatusIndex;
    RSyncServerSession &iSession;
};

#endif