#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

class TFeedInfo
{
public:
	TBuf<KUrlLength> url;
	TBuf<KTitleLength> title;
	TBuf<KDescriptionLength> description;
};

typedef RArray<TFeedInfo*> TFeedInfoArray;

#endif
