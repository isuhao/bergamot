#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KFeedTitleLength=256;
const int KFeedUrlLength=1024;
const int KFeedDescriptionLength=2048;

const int KFeedInfoVersion = 2;

class CFeedInfo : public CBase
{
public:
	CFeedInfo();
	~CFeedInfo();
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);
	
	TDesC& Url() const;
	void SetUrl(TDesC &aUrl);
	TDesC& Title() const;
	void SetTitle(TDesC &aTitle);
	TDesC& Description() const;
	void SetDescription(TDesC &aDescription);
	TDesC& FeedDirectory() const;
	TDesC& FeedFileName() const;
	TDesC& ImageUrl() const;
	void SetImageUrl(TDesC &aImageUrl);
	TDesC& Link() const;
	void SetLink(TDesC &aLink);
	TDesC& ImageFileName() const;
	void SetImageFileName(TDesC &aImageFileName);
	TTime PubDate();
	void SetPubDate(TTime aPubDate);
	TUint Uid();
	
private:
	void ReplaceString(TDes & aString, const TDesC& aStringToReplace,const TDesC& aReplacement );
	void SetFeedFileName(TDesC &aFeedFileName);
private:
	HBufC* iUrl;
	HBufC* iTitle;
	HBufC* iDescription;
	HBufC* iFeedFileName;
	HBufC* iFeedDirectory;
	HBufC* iImageUrl;
	HBufC* iLink;
	HBufC* iImageFileName;
	TTime iPubDate;
	//TDateTime iLastChecked;
	TUint iUid;
};

typedef RPointerArray<CFeedInfo> CFeedInfoArray;

#endif
