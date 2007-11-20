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

class MSoundEngineObserver
{
public:
	virtual void PlaybackInitializedL() = 0;

	virtual void PlaybackStartedL() = 0;
	virtual void PlaybackStoppedL() = 0;
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
	void SetObserver(MSoundEngineObserver* aObserver);
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
	MSoundEngineObserver* iObserver;
};

#endif // SOUND_ENGINE_H

