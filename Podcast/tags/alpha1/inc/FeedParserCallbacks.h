#ifndef FEEDPARSERCALLBACKS_H_
#define FEEDPARSERCALLBACKS_H_

#include "PodcastInfo.h"

class MFeedParserCallbacks
{
public:
	virtual void Item(TPodcastInfo *item) = 0;
};

#endif
