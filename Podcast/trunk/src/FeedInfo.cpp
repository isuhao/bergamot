#import "FeedInfo.h"
#include <e32hashtab.h>

CFeedInfo::CFeedInfo()
	{
	iUrl = HBufC::NewL(0);
	iTitle = HBufC::NewL(0);
	iDescription = HBufC::NewL(0);
	iFeedFileName = HBufC::NewL(0);
	iImageUrl = HBufC::NewL(0);
	iLink = HBufC::NewL(0);
	iImageFileName = HBufC::NewL(0);
	}

CFeedInfo::~CFeedInfo()
	{
	delete iUrl;
	delete iTitle;
	delete iDescription;
	delete iFeedFileName;
	delete iImageUrl;
	delete iLink;
	delete iImageFileName;
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

	if (iImageFileName == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iImageFileName->Length());
		aStream.WriteL(*iImageFileName);
	}

	if (iDescription == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iDescription->Length());
		aStream.WriteL(*iDescription);
	}

	if (iFeedFileName == NULL) {
		aStream.WriteInt32L(0);		
	} else {
		aStream.WriteInt32L(iFeedFileName->Length());
		aStream.WriteL(*iFeedFileName);
	}

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
		SetImageFileName(buffer);
	}
	

	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetDescription(buffer);
	}
	
	len = aStream.ReadInt32L();
	if (len > 0) {
		aStream.ReadL(buffer, len);
		SetFeedFileName(buffer);
	}
	
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

TDesC& CFeedInfo::FeedFileName() const
	{
	return *iFeedFileName;
	}

void CFeedInfo::SetFeedFileName(TDesC &aFeedFileName)
	{
	iFeedFileName = aFeedFileName.Alloc();
	}

TDesC& CFeedInfo::ImageUrl() const
	{
	return *iImageUrl;
	}

void CFeedInfo::SetImageUrl(TDesC &aImageUrl)
	{
	iImageUrl = aImageUrl.Alloc();
	}

TDesC& CFeedInfo::Link() const
	{
	return *iLink;
	}

void CFeedInfo::SetLink(TDesC &aLink)
	{
	iLink = aLink.Alloc();
	}

TDesC& CFeedInfo::ImageFileName() const
	{
	return *iImageFileName;
	}

void CFeedInfo::SetImageFileName(TDesC &aImageFileName)
	{
	iImageFileName = aImageFileName.Alloc();
	}

TTime CFeedInfo::PubDate()
	{
	return iPubDate;
	}

void CFeedInfo::SetPubDate(TTime aPubDate)
	{
	iPubDate = aPubDate;
	}

TUint CFeedInfo::Uid()
	{
	return iUid;
	}
