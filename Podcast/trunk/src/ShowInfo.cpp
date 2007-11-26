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

	aStream.WriteInt32L(iTitle->Length());
//	RDebug::Print(_L("wrote len: %d"), title.Length());

	aStream.WriteL(*iTitle);
//	RDebug::Print(_L("wrote title: %S"), &title);
	
	aStream.WriteInt32L(iUrl->Length());
//	RDebug::Print(_L("wrote len: %d"), url.Length());
	
	aStream.WriteL(*iUrl);
//	RDebug::Print(_L("wrote url: %S"), &url);
	
	aStream.WriteInt32L(iDescription->Length());
//	RDebug::Print(_L("wrote len: %d"), description.Length());

	aStream.WriteL(*iDescription);
//	RDebug::Print(_L("wrote description: %S"), &description);
	
	aStream.WriteInt32L(iFileName.Length());
//	RDebug::Print(_L("wrote len: %d"), fileName.Length());

	aStream.WriteL(iFileName);
//	RDebug::Print(_L("wrote fileName: %S"), &fileName);

	aStream.WriteInt32L(iFeedUid);
//	RDebug::Print(_L("wrote feedUrl: %S"), &feedUrl);

	aStream.WriteInt32L(iPosition.Int64());
	aStream.WriteInt32L(iDownloadState);
	aStream.WriteInt32L(iPlayState);
	aStream.WriteInt32L(iUid);
	aStream.WriteInt32L(iShowSize);

	aStream.WriteInt32L(I64LOW(iPubDate.Int64()));
	aStream.WriteInt32L(I64HIGH(iPubDate.Int64()));
	}

void CShowInfo::InternalizeL(RReadStream& aStream) {
	int len;
	TBuf<2045> buffer;
	TRAPD(error, len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);

	TRAP(error, aStream.ReadL(buffer, len));
	if (error != KErrNone) {
		return;
	}
	SetTitle(buffer);
	//RDebug::Print(_L("read title: %S"), &title);
	
	TRAP(error,len = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(buffer, len));
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("read url: %S"), &url);
	SetUrl(buffer);
	
	TRAP(error,len = aStream.ReadInt32L()) 
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);
	
	TRAP(error,aStream.ReadL(buffer, len));
	if (error != KErrNone) {
		return;
	}
	SetDescription(buffer);
	//RDebug::Print(_L("read description: %S"), &description);

	TRAP(error,len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(iFileName, len));
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("read fileName: %S"), &fileName);

	TRAP(error,iFeedUid = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("read feedUid: %d"), iFeedUid);
	
	TRAP(error,iPosition = aStream.ReadInt32L();) 
	if (error != KErrNone) {
		return;
	}

	TRAP(error,iDownloadState = (TDownloadState) aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

	TRAP(error,iPlayState = (TPlayState) aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

	// no point in setting playing directly
	if (iPlayState != ENeverPlayed) {
		iPlayState = EPlayed;
	}
	
	TRAP(error,iUid = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	
	TRAP(error,iShowSize = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

	TInt low = 0;
	TRAP(error,low = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	
	TInt high = 0;

	TRAP(error,high = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

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