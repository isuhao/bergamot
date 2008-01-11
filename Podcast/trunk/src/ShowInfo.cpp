#include "ShowInfo.h"
#include <e32hashtab.h>

CShowInfo* CShowInfo::NewL()
{
	CShowInfo* self = new (ELeave) CShowInfo;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

void CShowInfo::ConstructL()
{
	iTitle = HBufC::NewL(1);
	iUrl = HBufC::NewL(1);
	iDescription = HBufC::NewL(1);
	iFileName = HBufC::NewL(1);
}

CShowInfo::CShowInfo()
	{
	iDownloadState = ENotDownloaded;
	iDelete = EFalse;
	}

CShowInfo::~CShowInfo()
{
	delete iTitle;
	delete iUrl;
	delete iDescription;
	delete iFileName;
}

void CShowInfo::ExternalizeL(RWriteStream& aStream) const {

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
	
	if (iFileName == NULL) {
		aStream.WriteInt32L(0);
	} else {
		aStream.WriteInt32L(iFileName->Length());
		aStream.WriteL(*iFileName);
	}
	
	aStream.WriteInt32L(iFeedUid);
	aStream.WriteInt32L(iDownloadState);
	aStream.WriteInt32L(iPlayState == ENeverPlayed ? ENeverPlayed : EPlayed);
	aStream.WriteInt32L(iUid);
	aStream.WriteInt32L(iShowSize);
	aStream.WriteInt32L(I64LOW(iPosition.Int64()));
	aStream.WriteInt32L(I64HIGH(iPosition.Int64()));
	aStream.WriteInt32L(I64LOW(iPubDate.Int64()));
	aStream.WriteInt32L(I64HIGH(iPubDate.Int64()));
	}

void CShowInfo::InternalizeL(RReadStream& aStream) {
	TBuf<2048> buffer;
	int len = aStream.ReadInt32L();
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
		SetFileNameL(buffer);
	}
	
	iFeedUid = aStream.ReadInt32L();
	iDownloadState = (TDownloadState) aStream.ReadInt32L();
	iPlayState = (TPlayState) aStream.ReadInt32L();
	iUid = aStream.ReadInt32L();
	iShowSize = aStream.ReadInt32L();

	TInt low = aStream.ReadInt32L();
	TInt high = aStream.ReadInt32L();
	iPosition = MAKE_TINT64(high, low);

	low = aStream.ReadInt32L();
	high = aStream.ReadInt32L();
	iPubDate = MAKE_TINT64(high, low);
}

TDesC& CShowInfo::Title() const
	{
	return *iTitle;
	}

void CShowInfo::SetTitleL(TDesC &aTitle)
	{
	if (iTitle)
		{
		delete iTitle;
		iTitle = NULL;
		}
	iTitle = aTitle.AllocL();
	}

TDesC& CShowInfo::Url() const
	{
	return *iUrl;
	}

void CShowInfo::SetUrlL(TDesC &aUrl)
	{
	if (iUrl)
		{
		delete iUrl;
		iUrl = NULL;
		}
	iUrl = aUrl.AllocL();
	iUid = DefaultHash::Des16(Url());
	}

TDesC& CShowInfo::Description() const
	{
	return *iDescription;
	}

void CShowInfo::SetDescriptionL(TDesC &aDescription)
	{
	if (iDescription)
	{
	delete iDescription;
	iDescription = NULL;
		}

	iDescription = aDescription.AllocL();
	}

TTimeIntervalMicroSeconds& CShowInfo::Position()
	{
	return iPosition;
	}

void CShowInfo::SetPosition(TTimeIntervalMicroSeconds aPosition)
	{
	RDebug::Print(_L("SetPosition"));
	iPosition = aPosition;
	}

TPlayState CShowInfo::PlayState()
	{
	return iPlayState;
	}

void CShowInfo::SetPlayState(TPlayState aPlayState)
	{
	iPlayState = aPlayState;
	}

TDownloadState CShowInfo::DownloadState()
	{
	return iDownloadState;
	}

void CShowInfo::SetDownloadState(TDownloadState aDownloadState)
	{
	iDownloadState = aDownloadState;
	}

TUint CShowInfo::FeedUid()
	{
	return iFeedUid;
	}

void CShowInfo::SetFeedUid(TUint aFeedUid)
	{
	iFeedUid = aFeedUid;
	}

void CShowInfo::SetUid(TUint aUid)
	{
	iUid = aUid;
	}
		
TUint CShowInfo::Uid() const
	{
	return iUid;
	}

TUint CShowInfo::ShowSize()
	{
	return iShowSize;
	}

void CShowInfo::SetShowSize(TUint aShowSize)
	{
	iShowSize = aShowSize;
	}

const TTime CShowInfo::PubDate() const
	{
	return iPubDate;
	}

void CShowInfo::SetPubDate(TTime aPubDate)
	{
	iPubDate = aPubDate;
	}

TDesC& CShowInfo::FileName()
	{
	return *iFileName;
	}
void CShowInfo::SetDelete()
	{
	iDelete = ETrue;
	}

TBool CShowInfo::Delete()
	{
	return iDelete;
	}

void CShowInfo::SetFileNameL(TDesC &aFileName)
	{
	if (iFileName)
	{
	delete iFileName;
		iFileName = NULL;
		}

	iFileName = aFileName.AllocL();
	}
