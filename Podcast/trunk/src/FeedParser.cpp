#include "FeedParser.h"

CFeedParser::CFeedParser(MFeedParserCallbacks* aCallbacks) 
{
	iCallbacks = aCallbacks;
}

CFeedParser::~CFeedParser()
{
}


void CFeedParser::ParseL(TDesC &rss)
{

}