#ifndef METADATAREADER_H_
#define METADATAREADER_H_

#include <e32base.h>
#include <mdaaudiosampleplayer.h>
#include "ShowInfo.h"
#include "MetaDataReaderObserver.h"

class CMetaDataReader : public CBase, public MMdaAudioPlayerCallback
{
public:
	CMetaDataReader(MMetaDataReaderObserver &aObserver);
	void SubmitShow(CShowInfo *aShowInfo);
	virtual ~CMetaDataReader();
	void ConstructL();
	
protected:
	void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
	static TInt ParseNextShowL(TAny* aMetaDataReader);
private:
	void ParseNextShow();
	
private:
	CMdaAudioPlayerUtility *iPlayer;
	RShowInfoArray iShowsToParse;
	CShowInfo *iShow;
	MMetaDataReaderObserver &iObserver;
	TBuf8<256> iTempFileName;
	TBuf8<1024> iTempDataBuffer;
	TBuf<1024> iStringBuffer;
	CAsyncCallBack* iParseNextShowCallBack;
};

#endif /*METADATAREADER_H_*/
