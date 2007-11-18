#ifndef FEEDPARSERCALLBACKS_H_
#define FEEDPARSERCALLBACKS_H_

#include "ShowInfo.h"
#include "FeedInfo.h"

class MFeedParserCallbacks
{
public:
	virtual void NewShowCallback(TShowInfo *item) = 0;
	virtual void ParsingCompleteCallback(TFeedInfo *item) = 0;
};

#endif
