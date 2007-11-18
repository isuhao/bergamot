#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KShowInfoVersion = 3;

const int KTitleLength=256;
const int KUrlLength=1024;
const int KDescriptionLength=2048;

enum TPlayState {
	ENeverPlayed,
	EPlayed,
	EPlaying
};

enum TDownloadState {
	ENotDownloaded,
	EQueued,
	EDownloading,
	EDownloaded
};

class TShowInfo {
public:
TShowInfo();
void ExternalizeL(RWriteStream& aStream) const;
void InternalizeL(RReadStream& aStream);

public:
	TBuf<KTitleLength> iTitle;
	TBuf<KUrlLength> iUrl;
	TBuf<KDescriptionLength> iDescription;
	TInt iFeedUid;
	TFileName iFileName;
	TTimeIntervalMicroSeconds iPosition;
	TPlayState iPlayState;
	TDownloadState iDownloadState;
	TInt iUid;
	TUint32 iShowSize;
};


typedef RPointerArray<TShowInfo> TShowInfoArray;

#endif
