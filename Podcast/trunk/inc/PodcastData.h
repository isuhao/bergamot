#ifndef PODCASTDATA_H_
#define PODCASTDATA_H_

#include <S32STRM.H>
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
};

#endif
