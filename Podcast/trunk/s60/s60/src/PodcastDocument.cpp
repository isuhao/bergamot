#include "PodcastDocument.h"
#include "PodcastAppui.h"

CPodcastDocument::CPodcastDocument( CEikApplication& aApp )
    : CAknDocument( aApp )    
    {
    }

CPodcastDocument::~CPodcastDocument()
    {
    }

void CPodcastDocument::ConstructL()
    {
    }

CPodcastDocument* CPodcastDocument::NewL(CEikApplication& aApp )
    {
    CPodcastDocument* self = new (ELeave) CPodcastDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }
    
CEikAppUi* CPodcastDocument::CreateAppUiL()
    {
    return new (ELeave) CPodcastAppUi;
    }
