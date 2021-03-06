#include "OpmlParser.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <charconv.h>
#include <xml/stringdictionarycollection.h>
#include <utf.h>

using namespace Xml;

COpmlParser::COpmlParser(CFeedEngine& aFeedEngine) : iFeedEngine(aFeedEngine)
{
}

COpmlParser::~COpmlParser()
{
}

void COpmlParser::ParseOpmlL(const TFileName &feedFileName)
	{
	RDebug::Print(_L("ParseOpmlL BEGIN: %S"), &feedFileName);
	RFs rfs;
	rfs.Connect();
	CleanupClosePushL(rfs);
	
	_LIT8(KXmlMimeType, "text/xml");
	// Contruct the parser object
	CParser* parser = CParser::NewLC(KXmlMimeType, *this);
	iOpmlState = EStateOpmlRoot;
	iEncoding = EUtf8;

	ParseL(*parser, rfs, feedFileName);

	CleanupStack::PopAndDestroy(parser);
	CleanupStack::PopAndDestroy(&rfs); // this makes sure the file was closed
	//RDebug::Print(_L("ParseFeedL END"));
	}

// from MContentHandler
void COpmlParser::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnStartDocumentL()"));
	TBuf<1024> charset;
	charset.Copy(aDocParam.CharacterSetName().DesC());
	iEncoding = EUtf8;
	if (charset.CompareF(_L("utf-8")) == 0) {
		RDebug::Print(_L("setting UTF8"));
		iEncoding = EUtf8;
	} else if (charset.CompareF(_L("ISO-8859-1")) == 0) {
		iEncoding = EUtf8; //Latin1;
	} else {
		RDebug::Print(_L("unknown charset: %S"), &charset);
	}
	}

void COpmlParser::OnEndDocumentL(TInt /*aErrorCode*/)
	{
	//RDebug::Print(_L("OnEndDocumentL()"));
	}

void COpmlParser::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt /*aErrorCode*/)
	{
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());
	RDebug::Print(_L("OnStartElementL START state=%d, element=%S"), iOpmlState, &str);
	iBuffer.Zero();
	switch (iOpmlState) {
	case EStateOpmlRoot:
		// <body>
		if (str.CompareF(KTagBody) == 0) {
			iOpmlState = EStateOpmlBody;
		}
		break;
	case EStateOpmlBody:
		// <body> <outline>
		if(str.CompareF(KTagOutline) == 0) {
			//RDebug::Print(_L("New item"));
			iOpmlState=EStateOpmlOutline;
			CFeedInfo* newFeed = CFeedInfo::NewL();
			
			TBool hasTitle = EFalse;
			TBool hasUrl = EFalse;
			
			for (int i=0;i<aAttributes.Count();i++) {
				RAttribute attr = aAttributes[i];
				TBuf<100> attr16;
				attr16.Copy(attr.Attribute().LocalName().DesC());
				TBuf<1024> val16;
				val16.Copy(attr.Value().DesC());
				// xmlUrl=...
				if (attr16.Compare(KTagXmlUrl) == 0) {
					newFeed->SetUrlL(val16);
					hasUrl = ETrue;
				// text=...
				} else if (attr16.Compare(KTagText) == 0) {
					newFeed->SetTitleL(val16);
					newFeed->SetCustomTitle();
					hasTitle = ETrue;
				}
			}

			if (!hasUrl) {
				break;
			}
			
			if (!hasTitle) {
				newFeed->SetTitleL(newFeed->Url());
			}
			
			iFeedEngine.AddFeed(newFeed);
		}
		break;
	default:
		//RDebug::Print(_L("Ignoring tag %S when in state %d"), &str, iFeedState);
		break;
	}
//	RDebug::Print(_L("OnStartElementL END state=%d"), iFeedState);
	}

void COpmlParser::OnEndElementL(const RTagInfo& aElement, TInt /*aErrorCode*/)
	{
	
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());

	//RDebug::Print(_L("OnEndElementL START state=%d, element=%S"), iFeedState, &str);

	switch (iOpmlState) {
		case EStateOpmlOutline:
			iOpmlState=EStateOpmlBody;
			break;
		case EStateOpmlBody:
			iOpmlState = EStateOpmlRoot;
			break;
		default:
			// fall back to body level when in doubt
			iOpmlState = EStateOpmlBody;
			//RDebug::Print(_L("Don't know how to handle end tag %S when in state %d"), &str, iFeedState);
			break;
	}

	//RDebug::Print(_L("OnEndElementL END state=%d"), iFeedState);	
	}

void COpmlParser::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnContentL()"));
	}

void COpmlParser::OnStartPrefixMappingL(const RString& /*aPrefix*/, const RString& /*aUri*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnStartPrefixMappingL()"));
	}

void COpmlParser::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnEndPrefixMappingL()"));
	}

void COpmlParser::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnIgnorableWhiteSpaceL()"));
	}

void COpmlParser::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnSkippedEntityL()"));
	}

void COpmlParser::OnProcessingInstructionL(const TDesC8& /*aTarget*/, const TDesC8& /*aData*/, TInt /*aErrorCode*/)
	{
	RDebug::Print(_L("OnProcessingInstructionL()"));
	}

void COpmlParser::OnError(TInt aErrorCode)
	{
	RDebug::Print(_L("COpmlParser::OnError %d"), aErrorCode);
	}

TAny* COpmlParser::GetExtendedInterface(const TInt32 /*aUid*/)
	{
	RDebug::Print(_L("GetExtendedInterface()"));
	return NULL;
	}
