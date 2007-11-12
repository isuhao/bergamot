#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32cmn.h>
#include <e32std.h>

const int KTitleLength=256;
const int KUrlLength=1024;
const int KDescriptionLength=1024;

enum TShowState {
	EStateless,
	EPlaying,
	EDownloading
};

class TShowInfo {
public:
	TBuf<KTitleLength> title;
	TBuf<KUrlLength> url;
	TBuf<KDescriptionLength> description;
	TFileName fileName;
	TTimeIntervalMicroSeconds position;
	TShowState state;
	TBool playing;
};

typedef RArray<TShowInfo*> TShowInfoArray;

#endif
