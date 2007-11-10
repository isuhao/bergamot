#ifndef PODCASTITEM_H_
#define PODCASTITEM_H_

#include <e32cmn.h>

class TPodcastItem {
public:
	TDesC iTitle;
	TDesC iUrl;
	TDesC iDescription;
};

#endif