#ifndef FEEDPARSER_H_
#define FEEDPARSER_H_

#include "FeedParserCallbacks.h"

class CFeedParser
{
public:
	CFeedParser(MFeedParserCallbacks* aCallbacks);
	virtual ~CFeedParser();
	
public:
	void ParseL(TDesC &rss);
	
private:
	MFeedParserCallbacks* iCallbacks;
};

#endif
