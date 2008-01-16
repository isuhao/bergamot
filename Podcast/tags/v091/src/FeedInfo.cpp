#import "FeedInfo.h"
#include <e32hashtab.h>



CFeedInfo* CFeedInfo::NewL()
	{
	CFeedInfo* self = new (ELeave) CFeedInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CFeedInfo* CFeedInfo::NewLC()
	{
	CFeedInfo* self = new (ELeave) CFeedInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CFeedInfo::CFeedInfo()
	{
	}

CFeedInfo::~CFeedInfo()
	{
	delete iUrl;
	delete iTitle;
	delete iDescription;
	delete iImageUrl;
	delete iImageFileName;
	delete iLink;
	}

void CFeedInfo::ConstructL()
	{
	iUrl = HBufC::NewL(0);
	iTitle = HBufC::NewL(0);
	iDescription = HBufC::NewL(0);
	iImageUrl = HBufC::NewL(0);
	iImageFileName = HBufC::NewL(0);
	iLink = HBufC::NewL(0);
	}


void CFeedInfo::ExternalizeL(RWriteStream& aStream) const 
	{

	if (iTitle == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iTitle->Length());
		aStream.WriteL(*iTitle);
	}

	if (iUrl == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iUrl->Length());
		aStream.WriteL(*iUrl);
	}

	if (iDescription == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iDescription->Length());
		aStream.WriteL(*iDescription);
	}

	if (iImageFileName == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iImageFileName->Length());
		aStream.WriteL(*iImageFileName);
	}

	aStream.WriteInt32L(I64LOW(iBuildDate.Int64()));
	aStream.WriteInt32L(I64HIGH(iBuildDate.Int64()));
	
	aStream.WriteInt32L(I64LOW(iLastUpdated.Int64()));
	aStream.WriteInt32L(I64HIGH(iLastUpdated.Int64()));
	
	aStream.WriteInt32L(iUid);
	}

void CFeedInfo::InternalizeL(RReadStream& aStream) 
	{
	int len = aStream.ReadInt32L(); 
	TBuf<2048> buffer;
	
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetTitleL(buffer);
	}
	
	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetUrlL(buffer);
	}

	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetDescriptionL(buffer);
	}
		
	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetImageFileNameL(buffer);
	}
	
	TInt low = aStream.ReadInt32L();
	TInt high = aStream.ReadInt32L();
	iBuildDate = MAKE_TINT64(high, low);
	
	low = aStream.ReadInt32L();
	high = aStream.ReadInt32L();
	iLastUpdated = MAKE_TINT64(high, low);
	
	iUid = aStream.ReadInt32L();
	}

TDesC& CFeedInfo::Url() const
	{
	return *iUrl;
	}

void CFeedInfo::SetUrlL(TDesC &aUrl) 
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
		
	iUrl = aUrl.AllocL();		
	iUid = DefaultHash::Des16(Url());
	}

TDesC& CFeedInfo::Title() const
	{
	return *iTitle;
	}

void CFeedInfo::SetTitleL(TDesC &aTitle)
	{
	if(iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}		
	iTitle = aTitle.AllocL();
	}

TDesC& CFeedInfo::Description() const
	{
	return *iDescription;
	}

void CFeedInfo::SetDescriptionL(TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}
	
	iDescription = aDescription.AllocL(); 
	}

TDesC& CFeedInfo::ImageUrl() const
	{
	return *iImageUrl;
	}

void CFeedInfo::SetImageUrlL(TDesC &aImageUrl)
	{
	if (iImageUrl)
		{
		delete iImageUrl;
		iImageUrl = NULL;	
		}	
	iImageUrl = aImageUrl.AllocL();
	}

TDesC& CFeedInfo::Link() const
	{
	return *iLink;
	}

void CFeedInfo::SetLinkL(TDesC &aLink)
	{
	if (iLink)
	{
	delete iLink;
		iLink = NULL;
		}
	iLink = aLink.AllocL();
	}

TTime CFeedInfo::BuildDate()
	{
	return iBuildDate;
	}

void CFeedInfo::SetBuildDate(TTime aBuildDate)
	{
	iBuildDate = aBuildDate;
	}

TTime CFeedInfo::LastUpdated()
	{
	return iLastUpdated;
	}

void CFeedInfo::SetLastUpdated(TTime aUpdated)
	{
	iLastUpdated = aUpdated;
	}


TUint CFeedInfo::Uid()
	{
	return iUid;
	}

TDesC& CFeedInfo::ImageFileName()
	{
	return *iImageFileName;
	}

void CFeedInfo::SetImageFileNameL(TDesC &aFileName)
	{
	if (iImageFileName)
		{
		delete iImageFileName;
		iImageFileName = NULL;
		}
	iImageFileName = aFileName.AllocL();
	}
