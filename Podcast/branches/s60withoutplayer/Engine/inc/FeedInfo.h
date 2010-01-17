/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef _FEEDINFO_H_
#define _FEEDINFO_H_

#include <e32base.h>
#include <s32strm.h>

const TInt KFeedTitleLength=256;
const TInt KFeedUrlLength=1024;
const TInt KFeedDescriptionLength=2048;

class CFeedInfo : public CBase
	{
	public:
		IMPORT_C static CFeedInfo* NewL();
		IMPORT_C static CFeedInfo* NewLC();
	
		~CFeedInfo();
		
		IMPORT_C const TDesC& Url() const;
		IMPORT_C void SetUrlL(const TDesC &aUrl);

		IMPORT_C const TDesC& Title() const;
		IMPORT_C void SetTitleL(const TDesC &aTitle);

		IMPORT_C const TDesC& Description() const;
		IMPORT_C void SetDescriptionL(const TDesC &aDescription);

		IMPORT_C const TDesC& ImageUrl() const;
		IMPORT_C void SetImageUrlL(const TDesC &aImageUrl);

		IMPORT_C const TDesC& Link() const;
		IMPORT_C void SetLinkL(const TDesC &aLink);

		IMPORT_C TTime BuildDate();
		IMPORT_C void SetBuildDate(TTime aBuildDate);

		IMPORT_C TTime LastUpdated();
		IMPORT_C void SetLastUpdated(TTime aUpdated);

		IMPORT_C TUint Uid();

		IMPORT_C const TDesC& ImageFileName() const;
		IMPORT_C void SetImageFileNameL(const TDesC &aFileName);
	
		IMPORT_C TBool IsBookFeed();
		IMPORT_C void SetIsBookFeed();
		
		IMPORT_C void SetCustomTitle();
		IMPORT_C TBool CustomTitle();
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
		TBool iIsBookFeed;
		TBool iCustomTitle;
	};

typedef RPointerArray<CFeedInfo> RFeedInfoArray;

#endif
