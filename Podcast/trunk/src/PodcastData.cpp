#include "PodcastData.h"
#include <e32debug.h>

void CPodcastData::ExternalizeL(RWriteStream& aStream) const {
}

void CPodcastData::InternalizeL(RReadStream& aStream) {
}

CPodcastData::CPodcastData()
{
}

CPodcastData::~CPodcastData()
{
	delete iTimer;
}
