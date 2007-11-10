#ifndef PODCASTINFO_H_
#define PODCASTINFO_H_

#include <e32cmn.h>

const int KTitleLength = 256;
const int KUrlLength = 256;
const int KDescriptionLength = 2048;


class TPodcastInfo {
public:
	TBuf<KTitleLength> iTitle;
	TBuf<KUrlLength> iUrl;
	TBuf<KDescriptionLength> iDescription;
	TFileName iFile;
};

#endif