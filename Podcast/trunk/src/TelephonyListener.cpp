#include <e32debug.h>
#include "TelephonyListener.h"
#include "PodcastModel.h"
#include "SoundEngine.h"

CTelephonyListener::CTelephonyListener(CPodcastModel &aPodcastModel) : 
	CActive(EPriorityStandard), iPodcastModel(aPodcastModel), 
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

	switch (iLineStatus.iStatus) {
	case CTelephony::EStatusRinging:
	case CTelephony::EStatusConnecting:
	case CTelephony::EStatusConnected:
	case CTelephony::EStatusDialling:
	case CTelephony::EStatusAnswering:
		if (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying) {
			iPaused = ETrue;
			iPodcastModel.SoundEngine().Pause(ETrue);
		}
		break;
	case CTelephony::EStatusDisconnecting:
	case CTelephony::EStatusIdle:
		if (iPaused) {
			iPaused = EFalse;
			User::After(1000000*KHangupInProcessTimeout);
			iPodcastModel.SoundEngine().Play();
		}
		break;

	default:
		RDebug::Print(_L("CTelephony:: Other (unhandled)"));
		break;
	};

	iTelephony->NotifyChange(iStatus, CTelephony::EVoiceLineStatusChange, iLineStatusPckg);
	SetActive();
	}
