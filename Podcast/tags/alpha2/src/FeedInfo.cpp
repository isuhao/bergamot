#import "FeedInfo.h"

void TFeedInfo::ExternalizeL(RWriteStream& aStream) const {

	aStream.WriteInt32L(iTitle.Length());
//	RDebug::Print(_L("wrote len: %d"), title.Length());

	aStream.WriteL(iTitle);
	RDebug::Print(_L("wrote title: %S"), &iTitle);
	
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

	aStream.WriteInt32L(iUid);
	}

void TFeedInfo::InternalizeL(RReadStream& aStream) {
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
	RDebug::Print(_L("read title: %S"), &iTitle);
	
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
		
	TRAP(error,iUid = aStream.ReadInt32L());
	if (error != KErrNone) {
		return;
	}
	
}
