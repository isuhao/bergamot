#include "RemoteControlListener.h"
#include <e32debug.h>
#include "PodcastModel.h"
#include "SoundEngine.h"

CRemoteControlListener* CRemoteControlListener::NewL(CPodcastModel &aModel)
	{
	CRemoteControlListener* mylistener = new (ELeave) CRemoteControlListener(aModel);
	CleanupStack::PushL(mylistener);
    mylistener->ConstructL();
    CleanupStack::Pop(mylistener);
    return mylistener;
	}

CRemoteControlListener::CRemoteControlListener(CPodcastModel &aModel) : iModel(aModel)
	{
	delete iSelector;
	}

CRemoteControlListener::~CRemoteControlListener()
	{
	delete iSelector;
	}
	
void CRemoteControlListener::MrccatoPlay(TRemConCoreApiPlaybackSpeed aSpeed, TRemConCoreApiButtonAction aButtonAct)
	{
	RDebug::Print(_L("Play Command received"));
	iModel.SoundEngine().Play();
	}

void CRemoteControlListener::MrccatoTuneFunction()
	{
	RDebug::Print(_L("Tuner Command received"));
	}

void CRemoteControlListener::MrccatoSelectDiskFunction()
	{
	RDebug::Print(_L("Select disc function received"));
	}
	
void CRemoteControlListener::MrccatoSelectAvInputFunction(TUint8 aAvInputSignalNumber, TRemConCoreApiButtonAction aButtonAct)
	{
	RDebug::Print(_L("Select AV input function received"));
	}
	
void CRemoteControlListener::MrccatoSelectAudioInputFunction(TUint8 aAudioInputSignalNumber, TRemConCoreApiButtonAction aButtonAct)
	{
	RDebug::Print(_L("Select audio input function received"));
	}
	
void CRemoteControlListener::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct)
	{
	switch(aOperationId)
		{
		case ERemConCoreApiSelect:
			RDebug::Print(_L("ERemConCoreApiSelect"));
			break;
	/** Up. */
		case ERemConCoreApiUp:
			RDebug::Print(_L("ERemConCoreApiUp"));
			break;
	/** Down. */
		case ERemConCoreApiDown:
			RDebug::Print(_L("ERemConCoreApiDown"));
			break;
	/** Left. */
		case ERemConCoreApiLeft:
			RDebug::Print(_L("ERemConCoreApiLeft"));
			break;
	/** Right. */
		case ERemConCoreApiRight:
			RDebug::Print(_L("ERemConCoreApiRight"));
			break;
	/** RightUp. */
		case ERemConCoreApiRightUp:
			RDebug::Print(_L("ERemConCoreApiRightUp"));
			break;
	/** RightDown. */
		case ERemConCoreApiRightDown:
			RDebug::Print(_L("ERemConCoreApiRightDown"));
			break; 				
	/** LeftUp. */
		case ERemConCoreApiLeftUp:					
			RDebug::Print(_L("ERemConCoreApiLeftUp"));
			break;
	/** LeftDown. */
		case ERemConCoreApiLeftDown:					
			RDebug::Print(_L("ERemConCoreApiLeftDown"));
			break;
	/** RootMenu. */
		case ERemConCoreApiRootMenu:				
			RDebug::Print(_L("ERemConCoreApiRootMenu"));
			break;
	/** SetupMenu. */
		case ERemConCoreApiSetupMenu: 				
			RDebug::Print(_L("ERemConCoreApiSetupMenu"));
			break;
	/** ContentsMenu. */
		case ERemConCoreApiContentsMenu:				
			RDebug::Print(_L("ERemConCoreApiContentsMenu"));
			break;
	/** FavoriteMenu. */
		case ERemConCoreApiFavoriteMenu:				
			RDebug::Print(_L("ERemConCoreApiFavoriteMenu"));
			break;
	/** Exit. */
		case ERemConCoreApiExit:						
			RDebug::Print(_L("ERemConCoreApiExit"));
			break;
	/** 0. */
		case ERemConCoreApi0:
			RDebug::Print(_L("ERemConCoreApi0"));
			break; 						
	/** 1. */
		case ERemConCoreApi1:
			RDebug::Print(_L("ERemConCoreApi1"));
			break; 					
	/** 2. */
		case ERemConCoreApi2: 						
			RDebug::Print(_L("ERemConCoreApi2"));
			break;
	/** 3. */
		case ERemConCoreApi3: 						
			RDebug::Print(_L("ERemConCoreApi3"));
			break;
	/** 4. */
		case ERemConCoreApi4: 						
			RDebug::Print(_L("ERemConCoreApi4"));
			break;
	/** 5. */
		case ERemConCoreApi5:
			RDebug::Print(_L("ERemConCoreApi5"));
			break; 					
	/** 6. */
		case ERemConCoreApi6:
			RDebug::Print(_L("ERemConCoreApi6"));
			break; 						
	/** 7. */
		case ERemConCoreApi7: 
			RDebug::Print(_L("ERemConCoreApi7"));
			break;					
	/** 8. */
		case ERemConCoreApi8:
			RDebug::Print(_L("ERemConCoreApi8"));
			break; 					
	/** 9. */
		case ERemConCoreApi9: 
			RDebug::Print(_L("ERemConCoreApi9"));
			break;					
	/** Dot. */
		case ERemConCoreApiDot:
			RDebug::Print(_L("ERemConCoreApiDot"));
			break;					
	/** Enter. */
		case ERemConCoreApiEnter:
			RDebug::Print(_L("ERemConCoreApiEnter"));
			break; 					
	/** Clear. */
		case ERemConCoreApiClear: 
			RDebug::Print(_L("ERemConCoreApiClear"));
			break;				
	/** ChannelUp. */
		case ERemConCoreApiChannelUp:
			RDebug::Print(_L("ERemConCoreApiChannelUp"));
			break; 				
	/** ChannelDown. */
		case ERemConCoreApiChannelDown:
			RDebug::Print(_L("ERemConCoreApiChannelDown"));
			break;				
	/** PreviousChannel. */
		case ERemConCoreApiPreviousChannel:
			RDebug::Print(_L("ERemConCoreApiPreviousChannel"));
			break;		
	/** SoundSelect. */
		case ERemConCoreApiSoundSelect:
			RDebug::Print(_L("ERemConCoreApiSoundSelect"));
			break;				
	/** InputSelect. */
		case ERemConCoreApiInputSelect:	
			RDebug::Print(_L("ERemConCoreApiInputSelect"));
			break;		
	/** DisplayInformation. */
		case ERemConCoreApiDisplayInformation:	
			RDebug::Print(_L("ERemConCoreApiDisplayInformation"));
			break;
	/** Help. */
		case ERemConCoreApiHelp:
			RDebug::Print(_L("ERemConCoreApiHelp"));
			break;					
	/** PageUp. */
		case ERemConCoreApiPageUp:
			RDebug::Print(_L("ERemConCoreApiPageUp"));
			break;				
	/** PageDown. */
		case ERemConCoreApiPageDown:				
			RDebug::Print(_L("ERemConCoreApiPageDown"));
			break;
	/** Power. */
		case ERemConCoreApiPower: 
			RDebug::Print(_L("ERemConCoreApiPower"));
			break;				
	/** VolumeUp. */
		case ERemConCoreApiVolumeUp:					
			RDebug::Print(_L("ERemConCoreApiVolumeUp"));
			break;
	/** VolumeDown. */
		case ERemConCoreApiVolumeDown:
			RDebug::Print(_L("ERemConCoreApiVolumeDown"));
			break;				
	/** Mute. */
		case ERemConCoreApiMute:
			RDebug::Print(_L("ERemConCoreApiMute"));
			break;						
	/** Play. */
		case ERemConCoreApiPlay:
			RDebug::Print(_L("ERemConCoreApiPlay"));
			iModel.SoundEngine().Play();
			break;						
	/** Stop. */
		case ERemConCoreApiStop:
			RDebug::Print(_L("ERemConCoreApiStop"));
			iModel.SoundEngine().Stop();
			break;						
	/** Pause. */
		case ERemConCoreApiPause: 
			RDebug::Print(_L("ERemConCoreApiPause"));
			iModel.SoundEngine().Pause();
			break;					
	/** Record. */
		case ERemConCoreApiRecord:
			RDebug::Print(_L("ERemConCoreApiRecord"));
			break;					
	/** Rewind. */
		case ERemConCoreApiRewind:
			RDebug::Print(_L("ERemConCoreApiRewind"));
			break;					
	/** FastForward. */
		case ERemConCoreApiFastForward:				
			RDebug::Print(_L("ERemConCoreApiFastForward"));
			break;
	/** Eject. */
		case ERemConCoreApiEject: 					
			RDebug::Print(_L("ERemConCoreApiEject"));
			break;
	/** Forward. */
		case ERemConCoreApiForward:					
			RDebug::Print(_L("ERemConCoreApiForward"));
			break;
	/** Backward. */
		case ERemConCoreApiBackward:					
			RDebug::Print(_L("ERemConCoreApiBackward"));
			break;
	/** Angle. */
		case ERemConCoreApiAngle: 					
			RDebug::Print(_L("ERemConCoreApiAngle"));
			break;
	/** Subpicture. */
		case ERemConCoreApiSubpicture:				
			RDebug::Print(_L("ERemConCoreApiSubpicture"));
			break;
	/** PausePlayFunction. */
		case ERemConCoreApiPausePlayFunction: 		
			RDebug::Print(_L("ERemConCoreApiPausePlayFunction"));
			break;
	/** RestoreVolumeFunction. */
		case ERemConCoreApiRestoreVolumeFunction: 	
			RDebug::Print(_L("ERemConCoreApiRestoreVolumeFunction"));
			break;
	/** TuneFunction. */
		case ERemConCoreApiTuneFunction:				
			RDebug::Print(_L("ERemConCoreApiTuneFunctio"));
			break;
	/** SelectDiskFunction. */
		case ERemConCoreApiSelectDiskFunction:
			RDebug::Print(_L("ERemConCoreApiSelectDiskFunction"));
			break;		
	/** SelectAvInputFunction. */
		case ERemConCoreApiSelectAvInputFunction:
			RDebug::Print(_L("ERemConCoreApiSelectAvInputFunction"));
			break; 	
	/** SelectAudioInputFunction. */
		case ERemConCoreApiSelectAudioInputFunction:
			RDebug::Print(_L("ERemConCoreApiSelectAudioInputFunction"));
			break;	
	/** F1. */
		case ERemConCoreApiF1:
			if (iModel.SoundEngine().State() == ESoundEnginePlaying) {
				iModel.SoundEngine().Pause();
			} else if (iModel.SoundEngine().State() == ESoundEnginePaused) {
				iModel.SoundEngine().Play();
			}
			RDebug::Print(_L("ERemConCoreApiF1"));
			break;							
	/** F2. */
		case ERemConCoreApiF2:
			RDebug::Print(_L("ERemConCoreApiF2"));
			break;							
	/** F3. */
		case ERemConCoreApiF3:
			RDebug::Print(_L("ERemConCoreApiF3"));
			break;							
	/** F4. */
		case ERemConCoreApiF4:
			RDebug::Print(_L("ERemConCoreApiF4"));
			break;							
	/** F5. */
		case ERemConCoreApiF5:
			RDebug::Print(_L("ERemConCoreApiF5"));
			break;
		
		default:
			RDebug::Print(_L("Unknown"));
			break;						
		}
	
	}

void CRemoteControlListener::ConstructL()
	{
	// RemConInterface Selector
	iSelector = CRemConInterfaceSelector::NewL();
	
	// to be able to receive remote commands
	iTarget = CRemConCoreApiTarget::NewL(*iSelector,*this);
	
	//Opens a target session to RemCon.
	iSelector->OpenTargetL();
	}


