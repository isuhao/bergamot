#ifndef METADATAREADER_H_
#define METADATAREADER_H_

#include <e32base.h>
#include <mdaaudiosampleplayer.h>
#include "ShowInfo.h"
#include "MetaDataReaderObserver.h"

class CCnvCharacterSetConverter;
#include "id3tag.h"

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
	void ConvertToUniCodeL(TDes& aDestBuffer, TDes8& aInputBuffer, enum id3_field_textencoding aEncoding);
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
	TBuf<1024> iStringBuffer2;
	CAsyncCallBack* iParseNextShowCallBack;
	CCnvCharacterSetConverter* iCharConverter;
	TInt iLastConverterCharset;
};

#endif /*METADATAREADER_H_*/
