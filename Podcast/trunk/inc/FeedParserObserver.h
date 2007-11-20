#ifndef FEEDPARSEROBSERVER_H_
#define FEEDPARSEROBSERVER_H_

#include "ShowInfo.h"
#include "FeedInfo.h"

class MFeedParserObserver
{
public:
	virtual void NewShow(TShowInfo *item) = 0;
	virtual void ParsingComplete(TFeedInfo *item) = 0;
};

#endif
