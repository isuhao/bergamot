#ifndef PODCASTAPPUI_H
#define PODCASTAPPUI_H

#include <aknappui.h>
#include "PodcastContainer.h"

class CPodcastAppUi : public CAknAppUi
    {
    public: 
        void ConstructL();
       ~CPodcastAppUi();
        
    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        void HandleCommandL(TInt aCommand);
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);
        virtual void HandleResourceChangeL( TInt aType );            

    private:
    	CPodcastContainer *iAppContainer;
    };

#endif
