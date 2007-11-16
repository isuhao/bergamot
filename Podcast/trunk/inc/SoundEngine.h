#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H
#include <e32base.h>
#include <mdaaudiosampleplayer.h>

class CPodcastModel;


enum TSoundEngineState
{
	ESoundEngineNotInitialized,
	ESoundEnginePlaying,
	ESoundEnginePaused,
	ESoundEngineStopped
};


/**
 * This class handles all playback and audio associated resources that the application/client needs
 */
class CSoundEngine:public CBase, public MMdaAudioPlayerCallback
{
public:
	static CSoundEngine* NewL(CPodcastModel& aPodcastModel);
	~CSoundEngine();
	void OpenFileL(TDesC& aFileName);
	void Play();
	void Stop();
	TTimeIntervalMicroSeconds Position();
	void SetPosition(TUint aPos);

	/**
	 * Returns the playtime/duration in seconds 
	 * @return Value in a TUint
	 */
	TUint PlayTime();
	void Pause();

	TUint Volume();
	void SetVolume(TUint aVolume);

	TSoundEngineState State();
protected:
	CSoundEngine(CPodcastModel& aPodcastModel);
	void ConstructL();
	void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
private:
    CMdaAudioPlayerUtility *iPlayer;
	CPodcastModel& iPodcastModel;
	TInt iVolume;
	TSoundEngineState iState;	
};

#endif // SOUND_ENGINE_H

