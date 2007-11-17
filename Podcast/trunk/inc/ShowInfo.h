#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KShowInfoVersion = 2;

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
	TBuf<KTitleLength> title;
	TBuf<KUrlLength> url;
	TBuf<KDescriptionLength> description;
	TBuf<KUrlLength> feedUrl;
	TFileName fileName;
	TTimeIntervalMicroSeconds position;
	TPlayState playState;
	TDownloadState downloadState;
	TInt uid;
};


typedef RPointerArray<TShowInfo> TShowInfoArray;

#endif
