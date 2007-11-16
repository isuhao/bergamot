#include <QikContent.h>
#include <PodcastClient.mbg>

#include "PodcastClientBaseView.h"
#include "PodcastModel.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientBaseView* CPodcastClientBaseView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientBaseView* self = new (ELeave) CPodcastClientBaseView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientBaseView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID);
	}

CPodcastClientBaseView::CPodcastClientBaseView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
}

CPodcastClientBaseView::~CPodcastClientBaseView()
{
}

void CPodcastClientBaseView::UpdateListboxItemsL()
{
	iListbox->RemoveAllItemsL();
	
	MQikListBoxModel& model(iListbox->Model());
	// Informs the list box model that there will be an update of the data.
	// Notify the list box model that changes will be made after this point.
	// Observe that a cleanupitem has been put on the cleanupstack and 
	// will be removed by ModelEndUpdateL. This means that you have to 
	// balance the cleanupstack.
	// When you act directly on the model you always need to encapsulate 
	// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
	model.ModelBeginUpdateLC();
		
	MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
	CleanupClosePushL(*listBoxData);
	listBoxData->AddTextL(_L("Player"), EQikListBoxSlotText1);
	
	listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
	CleanupClosePushL(*listBoxData);
	listBoxData->AddTextL(_L("Shows"), EQikListBoxSlotText1);
	
	listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
	CleanupClosePushL(*listBoxData);
	listBoxData->AddTextL(_L("Feeds"), EQikListBoxSlotText1);
	CQikContent* content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientFeeds_40x40, EMbmPodcastclientFeeds_40x40m);
	CleanupStack::PushL(content);
	listBoxData->AddIconL(content, EQikListBoxSlotLeftMediumIcon1);
	CleanupStack::Pop(content);
	CleanupStack::PopAndDestroy(3);
	
	
	// Informs that the update of the list box model has ended
	model.ModelEndUpdateL();
}


void CPodcastClientBaseView::HandleListBoxEventL(CQikListBox * aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	default:
		break;
	case EEventHighlightMoved:
		break;
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			TUid newview = TUid::Uid(0);
			switch(aItemIndex)
			{
			case EBaseViewPlayer:
				{
					newview = KUidPodcastPlayViewID;
				}break;
			case EBaseViewShows:
				{
					newview = KUidPodcastShowsViewID;
				}break;
			case EBaseViewFeeds:
				{
					newview = KUidPodcastFeedViewID;
				}break;
			default:
				break;
			}
			if(newview.iUid != 0)
			{
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, newview);
				iQikAppUi.ActivateViewL(viewId);
			}
			
		}break;
	}
	
}
