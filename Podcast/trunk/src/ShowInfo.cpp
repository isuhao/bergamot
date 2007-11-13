#include "ShowInfo.h"

void TShowInfo::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteL(title);
	aStream.WriteL(url);
	aStream.WriteL(description);
	aStream.WriteL(fileName);
	aStream.WriteInt32L(position.Int64());
	aStream.WriteInt32L(state);
	}

void TShowInfo::InternalizeL(RReadStream& aStream) {
	aStream.ReadL(title);
	aStream.ReadL(url);
	aStream.ReadL(description);
	aStream.ReadL(fileName);
	position = aStream.ReadInt32L(); 
	state = (TShowState) aStream.ReadInt32L();
	playing = EFalse;
}