#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KTitleLength=256;
const int KUrlLength=1024;
const int KDescriptionLength=2048;

enum TShowState {
	EStateless,
	EDownloading,
	EDownloaded,
	EPlaying
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
	TBuf<KTitleLength> feedTitle;
	TFileName fileName;
	TTimeIntervalMicroSeconds position;
	TShowState state;
	TBool played;
	TBool playing;
	TBool iShowDownloaded;
};


typedef RArray<TShowInfo*> TShowInfoArray;

#endif
