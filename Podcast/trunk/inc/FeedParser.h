#ifndef FEEDPARSER_H_
#define FEEDPARSER_H_

#include "FeedParserCallbacks.h"
#include <xml/parser.h> // for CParser
#include <xml/contenthandler.h>	// for Xml::MContentHandler

using namespace Xml;

_LIT(KTagItem, "item");
_LIT(KTagTitle, "title");
_LIT(KTagImage, "image");
_LIT(KTagLink, "link");
_LIT(KTagDescription, "description");
_LIT(KTagChannel, "channel");

enum TFeedState {
	EStateNone,
	EStateRoot,
	EStateChannel,
	EStateChannelTitle,
	EStateChannelDescription,
	EStateChannelImage,
	EStateItem,
	EStateItemTitle,
	EStateItemLink,
	EStateItemDescription
};

class CFeedParser : public MContentHandler
{
public:
	CFeedParser(MFeedParserCallbacks& aCallbacks);
	virtual ~CFeedParser();
	
public:
	void ParseFeedL(TFileName &feedFileName);
	
public: // from MContentHandler
	void OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode);
	void OnEndDocumentL(TInt aErrorCode);
	void OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode);
	void OnEndElementL(const RTagInfo& aElement, TInt aErrorCode);
	void OnContentL(const TDesC8& aBytes, TInt aErrorCode);
	void OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode);
	void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);
	void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);
	void OnSkippedEntityL(const RString& aName, TInt aErrorCode);
	void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode);
	void OnError(TInt aErrorCode);
	TAny* GetExtendedInterface(const TInt32 aUid);
private:
	MFeedParserCallbacks& iCallbacks;
	TFeedState iFeedState;
	TShowInfo* activeItem;
	TDes* activeString;
	TBuf<1024> iChannelTitle;
	TBuf<2048> iChannelDescription;
};

#endif
