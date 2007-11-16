#include "ShowInfo.h"
TShowInfo::TShowInfo()
	{
	downloadState = ENotDownloaded;
	}

void TShowInfo::ExternalizeL(RWriteStream& aStream) const {

	aStream.WriteInt32L(title.Length());
	RDebug::Print(_L("wrote len: %d"), title.Length());

	aStream.WriteL(title);
	RDebug::Print(_L("wrote title: %S"), &title);
	
	aStream.WriteInt32L(url.Length());
	RDebug::Print(_L("wrote len: %d"), url.Length());
	
	aStream.WriteL(url);
	RDebug::Print(_L("wrote url: %S"), &url);
	
	aStream.WriteInt32L(description.Length());
	RDebug::Print(_L("wrote len: %d"), description.Length());

	aStream.WriteL(description);
	RDebug::Print(_L("wrote description: %S"), &description);
	
	aStream.WriteInt32L(fileName.Length());
	RDebug::Print(_L("wrote len: %d"), fileName.Length());

	aStream.WriteL(fileName);
	RDebug::Print(_L("wrote fileName: %S"), &fileName);

	aStream.WriteInt32L(feedUrl.Length());
	RDebug::Print(_L("wrote len: %d"), feedUrl.Length());

	aStream.WriteL(feedUrl);
	RDebug::Print(_L("wrote feedUrl: %S"), &feedUrl);

	aStream.WriteInt32L(position.Int64());
	aStream.WriteInt32L(downloadState);
	aStream.WriteInt32L(playState);
	}

void TShowInfo::InternalizeL(RReadStream& aStream) {
	int len;

	TRAPD(error, len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("len: %d"), len);

	TRAP(error, aStream.ReadL(title, len));
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("read title: %S"), &title);
	
	TRAP(error,len = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(url, len));
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("read url: %S"), &url);
	
	TRAP(error,len = aStream.ReadInt32L()) 
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("len: %d"), len);
	
	TRAP(error,aStream.ReadL(description, len));
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("read description: %S"), &description);

	TRAP(error,len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(fileName, len));
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("read fileName: %S"), &fileName);
	
	TRAP(error,len = aStream.ReadInt32L()); 
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("len: %d"), len);

	TRAP(error,aStream.ReadL(feedUrl, len));
	if (error != KErrNone) {
		return;
	}
	RDebug::Print(_L("read feedUrl: %S"), &feedUrl);
	
	TRAP(error,position = aStream.ReadInt32L();) 
	if (error != KErrNone) {
		return;
	}

	TRAP(error,downloadState = (TDownloadState) aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

	TRAP(error,playState = (TPlayState) aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}

	// no point in setting playing directly
	if (playState != ENeverPlayed) {
		playState = EPlayed;
	}
}
