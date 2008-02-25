#include "SyncServerData.h"
#include <e32debug.h>

void CSyncServerData::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteInt32L(profileId);
	aStream.WriteInt32L(period);
	RDebug::Print(_L("Wrote %d, %d"), profileId, period);
}

void CSyncServerData::InternalizeL(RReadStream& aStream) {
	profileId = aStream.ReadInt32L();
	period = (TSyncServerPeriod) aStream.ReadInt32L();
	timer = NULL;
	RDebug::Print(_L("Read %d, %d"), profileId, period);
}

