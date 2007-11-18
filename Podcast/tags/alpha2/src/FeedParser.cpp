#include "FeedParser.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <charconv.h>
#include <xml/stringdictionarycollection.h>

CFeedParser::CFeedParser(MFeedParserCallbacks& aCallbacks) : 	iCallbacks(aCallbacks)
{
}

CFeedParser::~CFeedParser()
{
}


void CFeedParser::ParseFeedL(TFileName &feedFileName, TFeedInfo *info)
	{
	RDebug::Print(_L("ParseFeedL BEGIN: %S"), &feedFileName);
	RFs rfs;
	rfs.Connect();
	_LIT8(KXmlMimeType, "text/xml");
	// Contruct the parser object
	CParser* parser = CParser::NewLC(KXmlMimeType, *this);
	iActiveFeed = info;
	ParseL(*parser, rfs, feedFileName);
	
	// Destroy the parser when done.
	CleanupStack::PopAndDestroy();
}

// from MContentHandler
void CFeedParser::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode)
	{
//	RDebug::Print(_L("OnStartDocumentL()"));
	iFeedState = EStateRoot;
	activeString = NULL;
	iChannelTitle.Copy(_L(""));
	activeItem=new TShowInfo;
	}

void CFeedParser::OnEndDocumentL(TInt aErrorCode)
	{
	//RDebug::Print(_L("OnEndDocumentL()"));
	iCallbacks.ParsingCompleteCallback(iActiveFeed);
	}

void CFeedParser::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode)
	{
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());
	//RDebug::Print(_L("OnStartElementL START state=%d, element=%S"), iFeedState, &str);
	
	switch (iFeedState) {
	case EStateRoot:
		// <channel>
		if (str.CompareF(KTagChannel) == 0) {
			iFeedState = EStateChannel;
		}
		break;
	case EStateChannel:
		// <channel> <item>
		if(str.CompareF(KTagItem) == 0) {
			//RDebug::Print(_L("New item"));
			iFeedState=EStateItem;
			activeItem = new TShowInfo();
			activeItem->iFeedUid = iActiveFeed->iUid;
		// <channel> <title>
		} else if (str.CompareF(KTagTitle) == 0) {
			iActiveFeed->iTitle.Copy(_L(""));
			RDebug::Print(_L("***Feed title!"));
			activeString = &iChannelTitle;
			iFeedState=EStateChannelTitle;
		// <channel> <description>
		} else if (str.CompareF(KTagDescription) == 0) {
			activeString = &iChannelDescription;
			iFeedState=EStateChannelTitle;
		// <channel> <image>
		} else if (str.CompareF(KTagImage) == 0) {
			activeString = NULL;
			iFeedState=EStateChannelImage;
		}
		break;
	case EStateItem:
		// <channel> <item> <title>
		if (str.CompareF(KTagTitle) == 0) {
			activeString = &activeItem->iTitle;
			iFeedState=EStateItemTitle;
		// <channel> <item> <link>
		} else if (str.CompareF(KTagLink) == 0) {
			//activeString = &activeItem->url;
			iFeedState=EStateItemLink;
		// <channel> <item> <enclosure url=...>
		} else if (str.CompareF(KTagEnclosure) == 0) {
			//RDebug::Print(_L("Enclosure"));
			for (int i=0;i<aAttributes.Count();i++) {
				RAttribute attr = aAttributes[i];
				TBuf<100> attr16;
				attr16.Copy(attr.Attribute().LocalName().DesC());
				if (attr16.Compare(_L("url")) == 0) {
					TBuf<100> val16;
					val16.Copy(attr.Value().DesC());
					activeItem->iUrl.Copy(val16);
//					RDebug::Print(_L("Attr %S=%S"), &attr16, &val16);
				}
			}
		} else if (str.CompareF(KTagDescription) == 0) {
			activeString = &activeItem->iDescription;
			iFeedState=EStateItemDescription;
		}
		break;
	default:
		activeString = NULL;
		//RDebug::Print(_L("Ignoring tag %S when in state %d"), &str, iFeedState);
	}
//	RDebug::Print(_L("OnStartElementL END state=%d"), iFeedState);
	}

void CFeedParser::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	{
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());

	//RDebug::Print(_L("OnEndElementL START state=%d, element=%S"), iFeedState, &str);

	switch (iFeedState) {
		case EStateChannelTitle:
			RDebug::Print(_L("Feed title: %S"), &iChannelTitle);
			iActiveFeed->iTitle.Copy(iChannelTitle);
		case EStateChannelDescription:
			iFeedState = EStateChannel;
			break;
		case EStateChannelImage:
			if (str.CompareF(KTagImage) == 0) {
				iFeedState = EStateChannel;
			}
			break;
		case EStateItem:
			if (str.CompareF(KTagItem) == 0) {
				iFeedState=EStateChannel;
//				activeItem->iFeedUrl.Copy(iActiveFeed->url);
				iCallbacks.NewShowCallback(activeItem);
			}
		break;
		case EStateItemTitle:
		case EStateItemLink:
		case EStateItemDescription:
			iFeedState = EStateItem;
			break;
		default:
			iFeedState = EStateChannel;
			//Debug::Print(_L("Don't know how to handle end tag %S when in state %d"), &str, iFeedState);
			break;
	}
	activeString = NULL;

	//RDebug::Print(_L("OnEndElementL END state=%d"), iFeedState);	
	}

void CFeedParser::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
	{
	if (activeString != NULL) {
		TBuf<2048> str;
		str.Copy(aBytes);
		//RDebug::Print(_L("OnContentL: %S, state: %d"), &str, iFeedState);
		if (activeString->Length() + str.Length() < KUrlLength) {
			activeString->Append(str);
		}
		//RDebug::Print(_L("activeString: %S"), activeString);
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
	RDebug::Print(_L("CFeedParser::OnError %d"), aErrorCode);
	}

TAny* CFeedParser::GetExtendedInterface(const TInt32 aUid)
	{
	RDebug::Print(_L("GetExtendedInterface()"));
	return NULL;
	}

TFeedInfo& CFeedParser::ActiveFeed()
	{
		return *iActiveFeed;
	}
