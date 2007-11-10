#ifndef FEEDPARSERCALLBACKS_H_
#define FEEDPARSERCALLBACKS_H_

#include "PodcastItem.h"

class MFeedParserCallbacks
{
public:
	virtual void Item(TPodcastItem *item) = 0;
};

#endif
