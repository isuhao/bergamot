#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KFeedTitleLength=256;
const int KFeedUrlLength=1024;
const int KFeedDescriptionLength=2048;

const int KFeedInfoVersion = 2;

class TFeedInfo
{
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);
public:
	TBuf<KFeedUrlLength> iUrl;
	TBuf<KFeedTitleLength> iTitle;
	TBuf<KFeedDescriptionLength> iDescription;
	TFileName iFileName;
	TBuf<KFeedUrlLength> iImageUrl;
	TBuf<KFeedUrlLength> iLink;
	TTime iPubDate;
	TFileName iImageFileName;
	TDateTime iLastChecked;
	TInt iUid;
};

typedef RPointerArray<TFeedInfo> TFeedInfoArray;

#endif
