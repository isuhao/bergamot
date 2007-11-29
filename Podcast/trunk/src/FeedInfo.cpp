#import "FeedInfo.h"
#include <e32hashtab.h>

CFeedInfo::CFeedInfo()
	{
	iUrl = HBufC::NewL(0);
	iTitle = HBufC::NewL(0);
	iDescription = HBufC::NewL(0);
	iImageUrl = HBufC::NewL(0);
	iImageFileName = HBufC::NewL(0);
	iLink = HBufC::NewL(0);
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
		SetTitle(buffer);
	}
	
	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetUrl(buffer);
	}

	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetDescription(buffer);
	}
		
	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetImageFileName(buffer);
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

void CFeedInfo::SetUrl(TDesC &aUrl) 
	{
	iUrl = aUrl.Alloc();
	iUid = DefaultHash::Des16(Url());
	}

TDesC& CFeedInfo::Title() const
	{
	return *iTitle;
	}

void CFeedInfo::SetTitle(TDesC &aTitle)
	{
	iTitle = aTitle.Alloc();
	}

TDesC& CFeedInfo::Description() const
	{
	return *iDescription;
	}

void CFeedInfo::SetDescription(TDesC &aDescription)
	{
	iDescription = aDescription.Alloc();
	}

TDesC& CFeedInfo::ImageUrl() const
	{
	return *iImageUrl;
	}

void CFeedInfo::SetImageUrl(TDesC &aImageUrl)
	{
	delete iImageUrl;
	iImageUrl = aImageUrl.Alloc();
	}

TDesC& CFeedInfo::Link() const
	{
	return *iLink;
	}

void CFeedInfo::SetLink(TDesC &aLink)
	{
	delete iLink;
	iLink = aLink.Alloc();
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

void CFeedInfo::SetImageFileName(TDesC &aFileName)
	{
	delete iImageFileName;
	iImageFileName = aFileName.Alloc();
	}
