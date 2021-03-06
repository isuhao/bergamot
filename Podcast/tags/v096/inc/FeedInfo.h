#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32base.h>
#include <s32strm.h>

const TInt KFeedTitleLength=256;
const TInt KFeedUrlLength=1024;
const TInt KFeedDescriptionLength=2048;
const TInt KFeedInfoVersion = 5;

class CFeedInfo : public CBase
	{
	public:
		static CFeedInfo* NewL(TInt aVersion=KFeedInfoVersion);
		static CFeedInfo* NewLC(TInt aVersion=KFeedInfoVersion);
	
		~CFeedInfo();
		
		void ExternalizeL(RWriteStream& aStream) const;
		void InternalizeL(RReadStream& aStream);
	
		const TDesC& Url() const;
		void SetUrlL(const TDesC &aUrl);
		const TDesC& Title() const;
		void SetTitleL(const TDesC &aTitle);
		const TDesC& Description() const;
		void SetDescriptionL(const TDesC &aDescription);
		const TDesC& ImageUrl() const;
		void SetImageUrlL(const TDesC &aImageUrl);
		const TDesC& Link() const;
		void SetLinkL(const TDesC &aLink);
		TTime BuildDate();
		void SetBuildDate(TTime aBuildDate);
		TTime LastUpdated();
		void SetLastUpdated(TTime aUpdated);
		TUint Uid();
		const TDesC& ImageFileName() const;
		void SetImageFileNameL(const TDesC &aFileName);
	
		TBool IsBookFeed();
		void SetIsBookFeed();
		
		void SetCustomTitle();
		TBool CustomTitle();
	private:
		CFeedInfo(TInt aVersion);
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
		TBool iIsBookFeed;
		TBool iCustomTitle;
		TInt iVersion;
	};

typedef RPointerArray<CFeedInfo> RFeedInfoArray;

#endif
