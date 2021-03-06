#ifndef FEEDPARSER_H_
#define FEEDPARSER_H_

#include "FeedParserCallbacks.h"
#include <xml/parser.h> // for CParser
#include <Xml/ContentHandler.h>	// for Xml::MContentHandler

using namespace Xml;

/*
_STLIT8(K1, "item");
_STLIT8(K2, "title");
_STLIT8(K3, "link");
_STLIT8(K4, "description");

const void * const KStringPointers[] =
    {
    (const void*)&K1,
    (const void*)&K2,
    (const void*)&K3,
    (const void*)&K4
    };

const TStringTable FeedXmlTable = {4, KStringPointers, ETrue};
*/

_LIT(KFeedItem, "item");
_LIT(KFeedTitle, "title");
_LIT(KFeedLink, "link");
_LIT(KFeedDescription, "description");

enum TFeedState {
	EStateNone,
	EStateItem,
	EStateTitle,
	EStateLink,
	EStateDescription
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
	RStringPool feedStringPool;
	TPodcastInfo* activeItem;
};

#endif
