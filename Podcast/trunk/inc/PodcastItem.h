#ifndef PODCASTITEM_H_
#define PODCASTITEM_H_

#include <e32cmn.h>

const int KTitleLength = 256;
const int KUrlLength = 256;
const int KDescriptionLength = 2048;


class TPodcastItem {
public:
	TBuf<KTitleLength> iTitle;
	TBuf<KUrlLength> iUrl;
	TBuf<KDescriptionLength> iDescription;
};

#endif