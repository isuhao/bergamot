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
	void OpenFileL(const TDesC& aFileName);
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

	TSoundEngineState State();
	void SetObserver(MSoundEngineObserver* aObserver);

	void SetVolume(TUint aVolume);
	const TFileName& LastFileName();
protected:
	CSoundEngine(CPodcastModel& aPodcastModel);
	void ConstructL();
	void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
private:
    CMdaAudioPlayerUtility *iPlayer;
	CPodcastModel& iPodcastModel;
	TSoundEngineState iState;
	MSoundEngineObserver* iObserver;
	TFileName	iLastOpenedFileName;
};

#endif // SOUND_ENGINE_H

