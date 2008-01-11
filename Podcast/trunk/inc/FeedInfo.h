#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KFeedTitleLength=256;
const int KFeedUrlLength=1024;
const int KFeedDescriptionLength=2048;

const int KFeedInfoVersion = 4;

class CFeedInfo : public CBase
{
public:
	static CFeedInfo* NewL();
	static CFeedInfo* NewLC();
	
	~CFeedInfo();
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);
	
	TDesC& Url() const;
	void SetUrlL(TDesC &aUrl);
	TDesC& Title() const;
	void SetTitleL(TDesC &aTitle);
	TDesC& Description() const;
	void SetDescriptionL(TDesC &aDescription);
	TDesC& ImageUrl() const;
	void SetImageUrlL(TDesC &aImageUrl);
	TDesC& Link() const;
	void SetLinkL(TDesC &aLink);
	TTime BuildDate();
	void SetBuildDate(TTime aBuildDate);
	TTime LastUpdated();
	void SetLastUpdated(TTime aUpdated);
	TUint Uid();
	TDesC& ImageFileName();
	void SetImageFileNameL(TDesC &aFileName);

private:
	CFeedInfo();
	void ConstructL();


private:
	HBufC* iUrl;
	HBufC* iTitle;
	HBufC* iDescription;
	HBufC* iImageUrl;
	HBufC* iImageFileName;
	HBufC* iLink;
	TTime iBuildDate;
	TTime iLastUpdated;
	TUint iUid;
};

typedef RPointerArray<CFeedInfo> RFeedInfoArray;

#endif
