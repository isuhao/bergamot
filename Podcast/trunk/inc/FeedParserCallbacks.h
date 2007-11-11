#ifndef FEEDPARSERCALLBACKS_H_
#define FEEDPARSERCALLBACKS_H_

#include "ShowInfo.h"

class MFeedParserCallbacks
{
public:
	virtual void Item(TShowInfo *item) = 0;
};

#endif
