#include <e32debug.h>
#include "TelephonyListener.h"
#include "PodcastModel.h"
#include "SoundEngine.h"

CTelephonyListener::CTelephonyListener(CPodcastModel &aPodcastModel) : 
	iPodcastModel(aPodcastModel), CActive(EPriorityStandard),
	iLineStatusPckg(iLineStatus)
	{
	iLineStatus.iStatus = CTelephony::EStatusUnknown;
	}

CTelephonyListener::~CTelephonyListener()
	{
	Cancel();
	delete iTelephony;
	}

void CTelephonyListener::DoCancel()
	{
	iTelephony->CancelAsync(CTelephony::EVoiceLineStatusChangeCancel);
	}

CTelephonyListener* CTelephonyListener::NewL(CPodcastModel &aPodcastModel)
	{
	CTelephonyListener* self = new(ELeave) CTelephonyListener(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CTelephonyListener::ConstructL()
	{
	RDebug::Print(_L("CTelephonyListener::ConstructL"));
	iTelephony = CTelephony::NewL();
	CActiveScheduler::Add(this);

	}

void CTelephonyListener::StartL()
	{
	iTelephony->NotifyChange(iStatus, CTelephony::EVoiceLineStatusChange, iLineStatusPckg);
	SetActive();
	}

void CTelephonyListener::RunL()
	{
	RDebug::Print(_L("CTelephonyListener::RunL: iStatus: %u"),iLineStatus.iStatus );

	// seems to be too late to do this at EStatusAnswering...
	iPodcastModel.SoundEngine().Pause();

	switch (iLineStatus.iStatus) {
	case CTelephony::EStatusRinging:
		RDebug::Print(_L("CTelephony::EStatusRinging"));
		break;
	case CTelephony::EStatusDialling:
		RDebug::Print(_L("CTelephony::EStatusDialling"));
		break;
	case CTelephony::EStatusAnswering:
		RDebug::Print(_L("CTelephony::EStatusAnswering"));
		break;
	case CTelephony::EStatusDisconnecting:
		RDebug::Print(_L("CTelephony::EStatusDisconnecting"));
		break;
	case CTelephony::EStatusConnecting:
		RDebug::Print(_L("CTelephony::EStatusConnecting:"));
		break;
	case CTelephony::EStatusConnected:
		RDebug::Print(_L("CTelephony::EStatusConnected"));
		break;
	default:
		RDebug::Print(_L("CTelephony:: Other (unhandled)"));
		break;
	};

	iTelephony->NotifyChange(iStatus, CTelephony::EVoiceLineStatusChange, iLineStatusPckg);
	SetActive();
	}
