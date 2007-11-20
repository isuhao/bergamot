#include "ShowInfo.h"

TShowInfo::TShowInfo()
	{
	iDownloadState = ENotDownloaded;
	iPlayState = ENeverPlayed;
	iPosition = 0;
	}

void TShowInfo::ExternalizeL(RWriteStream& aStream) const {

	aStream.WriteInt32L(iTitle.Length());
//	RDebug::Print(_L("wrote len: %d"), title.Length());

	aStream.WriteL(iTitle);
//	RDebug::Print(_L("wrote title: %S"), &title);
	
	aStream.WriteInt32L(iUrl.Length());
//	RDebug::Print(_L("wrote len: %d"), url.Length());
	
	aStream.WriteL(iUrl);
//	RDebug::Print(_L("wrote url: %S"), &url);
	
	aStream.WriteInt32L(iDescription.Length());
//	RDebug::Print(_L("wrote len: %d"), description.Length());

	aStream.WriteL(iDescription);
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

	}

void TShowInfo::InternalizeL(RReadStream& aStream) {
	int len;

	TRAPD(error, len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);

	TRAP(error, aStream.ReadL(iTitle, len));
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("read title: %S"), &title);
	
	TRAP(error,len = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(iUrl, len));
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("read url: %S"), &url);
	
	TRAP(error,len = aStream.ReadInt32L()) 
	if (error != KErrNone) {
		return;
	}
	//RDebug::Print(_L("len: %d"), len);
	
	TRAP(error,aStream.ReadL(iDescription, len));
	if (error != KErrNone) {
		return;
	}
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
}
