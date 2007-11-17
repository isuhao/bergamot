#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

const int KFeedTitleLength=256;
const int KFeedUrlLength=1024;
const int KFeedDescriptionLength=2048;

class TFeedInfo
{
public:
	TBuf<KFeedUrlLength> url;
	TBuf<KFeedTitleLength> title;
	TBuf<KFeedDescriptionLength> description;
	TFileName fileName;
	TDateTime lastChecked;
	TInt uid;
	TInt iCategoryHandle;
};

typedef RPointerArray<TFeedInfo> TFeedInfoArray;

#endif
