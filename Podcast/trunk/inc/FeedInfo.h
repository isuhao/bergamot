#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KFeedTitleLength=256;
const int KFeedUrlLength=1024;
const int KFeedDescriptionLength=2048;

const int KFeedInfoVersion = 3;

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
	TDesC& ImageUrl() const;
	void SetImageUrl(TDesC &aImageUrl);
	TDesC& Link() const;
	void SetLink(TDesC &aLink);
	TTime PubDate();
	void SetPubDate(TTime aPubDate);
	TUint Uid();
	
	TDesC &ImageFileName();
	void SetImageFileName(TDesC &aFileName);

private:
	HBufC* iUrl;
	HBufC* iTitle;
	HBufC* iDescription;
	HBufC* iImageUrl;
	HBufC* iImageFileName;
	HBufC* iLink;
	TTime iPubDate;
	//TDateTime iLastChecked;
	TUint iUid;
};

typedef RPointerArray<CFeedInfo> CFeedInfoArray;

#endif
