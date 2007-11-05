#ifndef PODCASTDATA_H_
#define PODCASTDATA_H_

#include <s32strm.h>
#include "PodcastServerTimer.h"

enum TPodcastPeriod
{
	ENever,
	EFiveMinutes,
	EOneHour,
	EFourHours,
	ETwentyFourHours,
	ENoPeriod = -1
};

class CPodcastData {
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TPodcastPeriod period;
	CPodcastServerTimer* timer;
	TDes *fileName;
};

#endif
