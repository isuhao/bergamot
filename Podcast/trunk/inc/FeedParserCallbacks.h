#ifndef FEEDPARSERCALLBACKS_H_
#define FEEDPARSERCALLBACKS_H_

#include "ShowInfo.h"

class MFeedParserCallbacks
{
public:
	virtual void NewShowCallback(TShowInfo *item) = 0;
	virtual void ParsingCompleteCallback() = 0;
};

#endif
