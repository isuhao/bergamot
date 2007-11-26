#include "ShowInfo.h"
#include <e32hashtab.h>

CShowInfo::CShowInfo()
	{
	iDownloadState = ENotDownloaded;
	iPlayState = ENeverPlayed;
	iPosition = 0;
	iPubDate = 0;
	iShowSize = 0;
	iFeedUid = 0;
	}

CShowInfo::~CShowInfo()
{
	delete iTitle;
	delete iUrl;
	delete iDescription;
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
		SetFileName(buffer);
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

TDesC& CShowInfo::Title()
	{
	return *iTitle;
	}

void CShowInfo::SetTitle(TDesC &aTitle)
	{
	iTitle = aTitle.Alloc();
	}

TDesC& CShowInfo::Url()
	{
	return *iUrl;
	}

void CShowInfo::SetUrl(TDesC &aUrl)
	{
	iUrl = aUrl.Alloc();
	iUid = DefaultHash::Des16(Url());
	}

TDesC& CShowInfo::Description()
	{
	return *iDescription;
	}

void CShowInfo::SetDescription(TDesC &aDescription)
	{
	iDescription = aDescription.Alloc();
	}


TDesC& CShowInfo::FileName()
	{
	return *iFileName;
	}

void CShowInfo::SetFileName(TDesC &aFileName)
	{
	iFileName = aFileName.Alloc();
	}

TTimeIntervalMicroSeconds& CShowInfo::Position()
	{
	return iPosition;
	}

void CShowInfo::SetPosition(TTimeIntervalMicroSeconds aPosition)
	{
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

TUint CShowInfo::Uid()
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

const TTime CShowInfo::PubDate()
	{
	return iPubDate;
	}

void CShowInfo::SetPubDate(TTime aPubDate)
	{
	iPubDate = aPubDate;
	}
