#include "KitchenSyncData.h"
#include <e32debug.h>

void CKitchenSyncData::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteInt32L(profileId);
	aStream.WriteInt32L(period);
	RDebug::Print(_L("Wrote %d, %d"), profileId, period);
}

void CKitchenSyncData::InternalizeL(RReadStream& aStream) {
	profileId = aStream.ReadInt32L();
	period = (TKitchenSyncPeriod) aStream.ReadInt32L();
	timer = NULL;
	RDebug::Print(_L("Read %d, %d"), profileId, period);
}

