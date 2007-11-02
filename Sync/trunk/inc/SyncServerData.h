#ifndef KITCHENSYNCDATA_H_
#define KITCHENSYNCDATA_H_

#include <S32STRM.H>
#include "KitchenSyncTimer.h"

enum TKitchenSyncPeriod
{
	ENever,
	EFiveMinutes,
	EOneHour,
	EFourHours,
	ETwentyFourHours,
	ENoPeriod = -1
};

class CKitchenSyncData {
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TSmlProfileId profileId;
	TKitchenSyncPeriod period;
	CKitchenSyncTimer* timer;
};

#endif
