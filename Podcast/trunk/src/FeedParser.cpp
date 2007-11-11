#include "FeedParser.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <charconv.h>
#include <xml/StringDictionaryCollection.h>

CFeedParser::CFeedParser(MFeedParserCallbacks& aCallbacks) : 	iCallbacks(aCallbacks)
{
	//feedStringPool.OpenL(FeedXmlTable);
	iFeedState=EStateNone;
}

CFeedParser::~CFeedParser()
{
}


void CFeedParser::ParseFeedL(TFileName &feedFileName)
	{
	RFs rfs;
	rfs.Connect();
	_LIT8(KXmlMimeType, "text/xml");
	// Contruct the parser object
	CParser* parser = CParser::NewLC(KXmlMimeType, *this);
	
	_LIT(KXmlFileName, "c:\\devices.xml");
	
	ParseL(*parser, rfs, feedFileName);
	
	// Destroy the parser when done.
	CleanupStack::PopAndDestroy();

/*

	
	RDebug::Print(_L("ParseL: %S"), &feedFileName);
	TParse feedFile;
	rfs.Parse(feedFileName, feedFile);
	
	if (!BaflUtils::FileExists(rfs, feedFileName)) {
		RDebug::Print(_L("Feed file not found"));	
		return;
	}
	RFile feed;
	
	int error = feed.Open(rfs, feedFileName, EFileRead);
	
	if (error != KErrNone) {
		RDebug::Print(_L("error=%d"), error);
		goto exit_point;
	}
	
	RDebug::Print(_L("Opened file"));

	TBuf8<1024> buf;
	error = feed.Read(buf);	
	while(error == KErrNone) {
		int pos = buf.Find(_L8("<"));
		int pos2 = buf.Find(_L("<"));
		RDebug::Print(_L("Hittade < på pos: %d, pos2: %d"), pos, pos2);
		error = feed.Read(buf);
	}
	
	
	feed.Close();
	exit_point:
	rfs.Close();*/
}

// from MContentHandler
void CFeedParser::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode)
	{
	//RDebug::Print(_L("OnStartDocumentL()"));
	activeItem=new TShowInfo;
	}

void CFeedParser::OnEndDocumentL(TInt aErrorCode)
	{
	//RDebug::Print(_L("OnEndDocumentL()"));
	}

void CFeedParser::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode)
	{

	//RStringDictionaryCollection collection;
	//collection.OpenL();
	//RStringPool thePool = collection.StringPool();
	//thePool.
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());
	//RDebug::Print(_L("OnStartElementL START state=%d, element=%S"), iFeedState, &str);

	if(str.CompareF(KFeedItem) == 0) {
	
		iFeedState=EStateItem;
		activeItem = new TShowInfo();
	} else if (iFeedState == EStateItem) {
		if (str.CompareF(KFeedTitle) == 0) {
			iFeedState=EStateTitle;
		} else if (str.CompareF(KFeedLink) == 0) {
			iFeedState=EStateLink;
		} else if (str.CompareF(KFeedDescription) == 0) {
			iFeedState=EStateDescription;
		}
	}
	//	collection.Close();
	//RDebug::Print(_L("OnStartElementL END state=%d"), iFeedState);
	}

void CFeedParser::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	{
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());
	//RDebug::Print(_L("OnEndElementL START state=%d, element=%S"), iFeedState, &str);
	//RDebug::Print(_L("iFeedState: %d"),  iFeedState);

	if (str.CompareF(KFeedItem) == 0) {
		iFeedState=EStateNone;		
		iCallbacks.Item(activeItem);
	} else if (iFeedState > EStateItem) {
		iFeedState = EStateItem;
	} /*else {
		iFeedState = EStateNone;
	}*/
	//RDebug::Print(_L("OnEndElementL END state=%d"), iFeedState);	
	}

void CFeedParser::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
	{
	TBuf<2048> str;
	str.Copy(aBytes);
	//RDebug::Print(_L("OnContentL: %S, state: %d"), &str, iFeedState);

	
	switch (iFeedState) {
	case EStateTitle:
		if(activeItem->title.Length()+str.Length() < KTitleLength) {
			activeItem->title.Append(str);
		}
		break;
	case EStateDescription:
		if(activeItem->description.Length()+str.Length() < KDescriptionLength) {
			activeItem->description.Append(str);
		}
		break;
	case EStateLink:
	if(activeItem->url.Length()+str.Length() < KUrlLength) {
			activeItem->url.Append(str);
		}
		break;	
	}
	}

void CFeedParser::OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode)
	{
	RDebug::Print(_L("OnStartPrefixMappingL()"));
	}

void CFeedParser::OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode)
	{
	RDebug::Print(_L("OnEndPrefixMappingL()"));
	}

void CFeedParser::OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode)
	{
	RDebug::Print(_L("OnIgnorableWhiteSpaceL()"));
	}

void CFeedParser::OnSkippedEntityL(const RString& aName, TInt aErrorCode)
	{
	RDebug::Print(_L("OnSkippedEntityL()"));
	}

void CFeedParser::OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode)
	{
	RDebug::Print(_L("OnProcessingInstructionL()"));
	}

void CFeedParser::OnError(TInt aErrorCode)
	{
	RDebug::Print(_L("OnError()"));
	}

TAny* CFeedParser::GetExtendedInterface(const TInt32 aUid)
	{
	RDebug::Print(_L("GetExtendedInterface()"));
	return NULL;
	}