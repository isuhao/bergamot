#include "FeedParser.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <charconv.h>
#include <xml/stringdictionarycollection.h>
#include <utf.h>
#include <tinternetdate.h>

CFeedParser::CFeedParser(MFeedParserObserver& aCallbacks) : 	iCallbacks(aCallbacks)
{
}

CFeedParser::~CFeedParser()
{
}

void CFeedParser::ParseFeedL(const TFileName &feedFileName, CFeedInfo *info, TUint aMaxItems)
	{
	//RDebug::Print(_L("ParseFeedL BEGIN: %S"), &feedFileName);
	RFs rfs;
	rfs.Connect();
	_LIT8(KXmlMimeType, "text/xml");
	// Contruct the parser object
	CParser* parser = CParser::NewLC(KXmlMimeType, *this);
	iActiveFeed = info;
	iFeedState = EStateRoot;
	iActiveShow = CShowInfo::NewL();
	iItemsParsed = 0;
	iMaxItems = aMaxItems;
	iStoppedParsing = EFalse;
	iEncoding = ELatin1;

	ParseL(*parser, rfs, feedFileName);
	rfs.Close(); // this makes sure the file was closed
	
	rfs.Connect();
	BaflUtils::DeleteFile(rfs,feedFileName);
	rfs.Close();
	// Destroy the parser when done.
	CleanupStack::PopAndDestroy();
	//RDebug::Print(_L("ParseFeedL END"));
}

// from MContentHandler
void CFeedParser::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode)
	{
	RDebug::Print(_L("OnStartDocumentL()"));
	TBuf<1024> charset;
	charset.Copy(aDocParam.CharacterSetName().DesC());
	if (charset.CompareF(_L("utf-8")) == 0) {
		RDebug::Print(_L("setting UTF8"));
		iEncoding = EUtf8;
	} else if (charset.CompareF(_L("ISO-8859-1")) == 0) {
		iEncoding = ELatin1;
	} else {
		RDebug::Print(_L("unknown charset: %S"), &charset);
	}
	}

void CFeedParser::OnEndDocumentL(TInt aErrorCode)
	{
	//RDebug::Print(_L("OnEndDocumentL()"));
	iCallbacks.ParsingComplete(iActiveFeed);
	}

void CFeedParser::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode)
	{
	if (iStoppedParsing) {
		iActiveShow = NULL;
		return;
	}
	
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());
	//RDebug::Print(_L("OnStartElementL START state=%d, element=%S"), iFeedState, &str);
	iBuffer.Zero();
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

			iActiveShow = CShowInfo::NewL();
			if (iActiveShow == NULL) {
				RDebug::Print(_L("Out of memory!"));
				iStoppedParsing = ETrue;
				return;
			}
			iActiveShow->SetFeedUid(iActiveFeed->Uid());

		// <channel> <lastBuildDate>
		} else if (str.CompareF(KTagLastBuildDate) == 0) {
			RDebug::Print(_L("LastBuildDate BEGIN"));
			iFeedState=EStateChannelLastBuildDate;
		// <channel> <link>
		}else if (str.CompareF(KTagTitle) == 0) {
			iFeedState=EStateChannelTitle;
		// <channel> <link>
		} else if (str.CompareF(KTagLink) == 0) {
			iFeedState = EStateChannelLink;
		// <channel> <description>
		} else if (str.CompareF(KTagDescription) == 0) {
			iFeedState=EStateChannelDescription;
		// <channel> <image>
		} else if (str.CompareF(KTagImage) == 0) {
			iFeedState=EStateChannelImage;
		}
		break;
	case EStateChannelImage:
		// <channel> <image> <url>
		if (str.CompareF(KTagUrl) == 0) {
			iFeedState=EStateChannelImageUrl;
		} else {
			iFeedState=EStateChannelImage;
		}
		break;
	case EStateItem:
		// <channel> <item> <title>
		if (str.CompareF(KTagTitle) == 0) {
			iFeedState=EStateItemTitle;
		// <channel> <item> <link>
		} else if (str.CompareF(KTagLink) == 0) {
			iFeedState=EStateItemLink;
		// <channel> <item> <enclosure ...>
		} else if (str.CompareF(KTagEnclosure) == 0) {
			//RDebug::Print(_L("Enclosure START"));
			for (int i=0;i<aAttributes.Count();i++) {
				RAttribute attr = aAttributes[i];
				TBuf<100> attr16;
				attr16.Copy(attr.Attribute().LocalName().DesC());
				// url=...
				if (attr16.Compare(KTagUrl) == 0) {
					TBuf<1024> val16;
					val16.Copy(attr.Value().DesC());
					iActiveShow->SetUrl(val16);
				// length=...
				} else if (attr16.Compare(KTagLength) == 0) {
					TLex8 lex(attr.Value().DesC());
					TUint size = -1;
					lex.Val(size, EDecimal);
					iActiveShow->SetShowSize(size);
				}
			}
		// <channel> <item> <description>
		} else if (str.CompareF(KTagDescription) == 0) {
			iFeedState=EStateItemDescription;
		// <channel> <item> <pubdate>
		} else if (str.CompareF(KTagPubDate) == 0) {
			//RDebug::Print(_L("LastBuildDate BEGIN"));
			iFeedState = EStateItemPubDate;
		}
		break;
	//default:
		//RDebug::Print(_L("Ignoring tag %S when in state %d"), &str, iFeedState);
	}
//	RDebug::Print(_L("OnStartElementL END state=%d"), iFeedState);
	}

void CFeedParser::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
	{
	
	if (iStoppedParsing) {
		return;
	}
	
	TDesC8 lName = aElement.LocalName().DesC();
	TBuf<100> str;
	str.Copy(aElement.LocalName().DesC());

	//RDebug::Print(_L("OnEndElementL START state=%d, element=%S"), iFeedState, &str);

	switch (iFeedState) {
		case EStateChannelTitle:
			if(str.CompareF(KTagTitle) == 0) {
				iActiveFeed->SetTitle(iBuffer);
				iFeedState = EStateChannel;
			}
			break;
		case EStateChannelLink:
			iActiveFeed->SetLink(iBuffer);
			iFeedState = EStateChannel;
			break;
		case EStateChannelDescription:
			iActiveFeed->SetDescription(iBuffer);
			iFeedState = EStateChannel;
			break;
		case EStateChannelLastBuildDate:
			{
			//RDebug::Print(_L("LastBuildDate END"));
			TInternetDate internetDate;
			TBuf8<128> temp;
			temp.Copy(iBuffer);
					
			TRAPD(parseError, internetDate.SetDateL(temp));
			if(parseError == KErrNone) {				
				if (TTime(internetDate.DateTime()) > iActiveFeed->BuildDate()) {
					RDebug::Print(_L("Successfully parsed build date"));
					iActiveFeed->SetBuildDate(TTime(internetDate.DateTime()));
				} else {
					RDebug::Print(_L("*** Nothing new, aborting parsing"));
					iStoppedParsing = ETrue;
				}
			} else {
				RDebug::Print(_L("Failed to parse last build date"));
			}
			iFeedState = EStateChannel;
			}
			break;
		case EStateChannelImageUrl:
			//RDebug::Print(_L("Image url: %S"), &iBuffer);
			iActiveFeed->SetImageUrl(iBuffer);
			iFeedState = EStateChannelImage;
			break;
		case EStateChannelImage:
			if(str.CompareF(KTagImage) == 0) {
				iFeedState = EStateChannel;
			}
			break;
		case EStateItem:
			iCallbacks.NewShow(iActiveShow);
			if (str.CompareF(KTagItem) == 0) {
				iItemsParsed++;
				//RDebug::Print(_L("iItemsParsed: %d, iMaxItems: %d"), iItemsParsed, iMaxItems);
				if (iItemsParsed > iMaxItems) {
					iStoppedParsing = ETrue;
					RDebug::Print(_L("*** Too many items, aborting parsing"));
				}
				
				iFeedState=EStateChannel;
			}
			break;
		case EStateItemPubDate:
			//RDebug::Print(_L("PubDate END"));
			if (str.CompareF(KTagPubDate) == 0) {
				// hack for feeds that don't always write day as two digits
				TChar five(iBuffer[5]);
				TChar six(iBuffer[6]);
				
				if (five.IsDigit() && !six.IsDigit()) {
					TBuf<100> fix;
					fix.Copy(iBuffer.Left(4));
					fix.Append(_L(" 0"));
					fix.Append(iBuffer.Mid(5));
					iBuffer.Copy(fix);
				}
				// end hack
				
				TBuf8<128> temp;
				temp.Copy(iBuffer);

				TInternetDate internetDate;
				TRAPD(parseError, internetDate.SetDateL(temp));
				if(parseError == KErrNone) {				
					//RDebug::Print(_L("PubDate parse success: '%S'"), &iBuffer);
					iActiveShow->SetPubDate(TTime(internetDate.DateTime()));
			
					
					/*RDebug::Print(_L("Successfully parsed pubdate %d/%d/%d %d:%d:%d"),
							iActiveShow->iPubDate.DateTime().Year(),
							iActiveShow->iPubDate.DateTime().Month(),
							iActiveShow->iPubDate.DateTime().Day(),
							iActiveShow->iPubDate.DateTime().Hour(),
							iActiveShow->iPubDate.DateTime().Minute(),
							iActiveShow->iPubDate.DateTime().Second());*/
							
				} else {
					RDebug::Print(_L("Pubdate parse error: '%S', error=%d"), &iBuffer, parseError);
				}
			}
			iFeedState=EStateItem;
			break;
		case EStateItemTitle:
			//RDebug::Print(_L("title: %S"), &iBuffer);
			iActiveShow->SetTitle(iBuffer);
			iFeedState = EStateItem;
			break;
		case EStateItemLink:
			iActiveShow->SetUrl(iBuffer);
			iFeedState = EStateItem;
			break;
		case EStateItemDescription:
			iActiveShow->SetDescription(iBuffer);
			iFeedState = EStateItem;
			break;
		default:
			// fall back to channel level when in doubt
			iFeedState = EStateChannel;
			//RDebug::Print(_L("Don't know how to handle end tag %S when in state %d"), &str, iFeedState);
			break;
	}

	//RDebug::Print(_L("OnEndElementL END state=%d"), iFeedState);	
	}

void CFeedParser::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
	{
	TBuf<KBufferLength> temp;
	if (iEncoding == EUtf8) {
		CnvUtfConverter::ConvertToUnicodeFromUtf8(temp, aBytes);
	} else {
		temp.Copy(aBytes);
	}
	
	if(temp.Length() + iBuffer.Length() < KBufferLength) {
		iBuffer.Append(temp);
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

CFeedInfo& CFeedParser::ActiveFeed()
	{
		return *iActiveFeed;
	}
