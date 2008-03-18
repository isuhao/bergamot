#include "FeedInfo.h"
#include <e32hashtab.h>


CFeedInfo* CFeedInfo::NewL(TInt aVersion)
	{
	CFeedInfo* self = new (ELeave) CFeedInfo(aVersion);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CFeedInfo* CFeedInfo::NewLC(TInt aVersion)
	{
	CFeedInfo* self = new (ELeave) CFeedInfo(aVersion);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CFeedInfo::CFeedInfo(TInt aVersion) : iVersion(aVersion)
	{
	iCustomTitle = EFalse;
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
	}

void CFeedInfo::ExternalizeL(RWriteStream& aStream) const 
	{
	if (iTitle == NULL) 
		{
		aStream.WriteInt32L(0);		
		} 
	else 
		{
		aStream.WriteInt32L(iTitle->Length());
		aStream.WriteL(*iTitle);
		}

	if (iUrl == NULL) 
		{
		aStream.WriteInt32L(0);		
		}
	else 
		{
		aStream.WriteInt32L(iUrl->Length());
		aStream.WriteL(*iUrl);
		}

	if (iDescription == NULL) 
		{
		aStream.WriteInt32L(0);		
		} 
	else 
		{
		aStream.WriteInt32L(iDescription->Length());
		aStream.WriteL(*iDescription);
		}

	if (iImageFileName == NULL) 
		{
		aStream.WriteInt32L(0);		
		}
	else 
		{
		aStream.WriteInt32L(iImageFileName->Length());
		aStream.WriteL(*iImageFileName);
		}

	aStream.WriteInt32L(I64LOW(iBuildDate.Int64()));
	aStream.WriteInt32L(I64HIGH(iBuildDate.Int64()));
	
	aStream.WriteInt32L(I64LOW(iLastUpdated.Int64()));
	aStream.WriteInt32L(I64HIGH(iLastUpdated.Int64()));
	
	aStream.WriteInt32L(iUid);
	
	aStream.WriteInt32L(iCustomTitle);
	}

void CFeedInfo::InternalizeL(RReadStream& aStream) 
	{
	TInt len = aStream.ReadInt32L(); 
	TBuf<2048> buffer;
	
	if (len > 0) 
		{
		aStream.ReadL(buffer, len);
		SetTitleL(buffer);
		}
	
	len = aStream.ReadInt32L();
	if (len > 0) 
		{
		aStream.ReadL(buffer, len);
		SetUrlL(buffer);
		}

	len = aStream.ReadInt32L();
	if (len > 0) 
		{
		aStream.ReadL(buffer, len);
		SetDescriptionL(buffer);
		}
		
	len = aStream.ReadInt32L();
	if (len > 0) 
		{
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
	
	if (iVersion < 5) {
		return;
	}
		iCustomTitle = aStream.ReadInt32L();
	}

const TDesC& CFeedInfo::Url() const
	{
	return iUrl ? *iUrl : KNullDesC();
	}

void CFeedInfo::SetUrlL(const TDesC &aUrl) 
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
		
	iUrl = aUrl.AllocL();		
	iUid = DefaultHash::Des16(Url());
	}

const TDesC& CFeedInfo::Title() const
	{
	return iTitle ? *iTitle : KNullDesC();
	}

void CFeedInfo::SetTitleL(const TDesC &aTitle)
	{
	if(iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}		
	iTitle = aTitle.AllocL();
	}

const TDesC& CFeedInfo::Description() const
	{
	return iDescription ? *iDescription : KNullDesC();
	}

void CFeedInfo::SetDescriptionL(const TDesC &aDescription)
	{
	if (iDescription)
		{
		delete iDescription;
		iDescription = NULL;
		}
	
	iDescription = aDescription.AllocL(); 
	}

const TDesC& CFeedInfo::ImageUrl() const
	{
	return iImageUrl ? *iImageUrl : KNullDesC();
	}

void CFeedInfo::SetImageUrlL(const TDesC &aImageUrl)
	{
	if (iImageUrl)
		{
		delete iImageUrl;
		iImageUrl = NULL;	
		}	
	iImageUrl = aImageUrl.AllocL();
	}

const TDesC& CFeedInfo::Link() const
	{
	return iLink ? *iLink : KNullDesC();
	}

void CFeedInfo::SetLinkL(const TDesC& aLink)
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

const TDesC& CFeedInfo::ImageFileName() const
	{
	return iImageFileName ? *iImageFileName : KNullDesC();
	}

void CFeedInfo::SetImageFileNameL(const TDesC& aFileName)
	{
	if (iImageFileName)
		{
		delete iImageFileName;
		iImageFileName = NULL;
		}
	iImageFileName = aFileName.AllocL();
	}


TBool CFeedInfo::IsBookFeed()
	{
	return iIsBookFeed;
	}

void CFeedInfo::SetIsBookFeed()
	{
	iIsBookFeed = ETrue;
	}

TBool CFeedInfo::CustomTitle()
	{
	return iCustomTitle;
	}

void CFeedInfo::SetCustomTitle()
	{
	iCustomTitle = ETrue;
	}

