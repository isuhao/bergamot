#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32base.h>
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KShowInfoVersion = 6;

enum TPlayState {
	ENeverPlayed,
	EPlayed,
	EPlaying
};

enum TDownloadState {
	ENotDownloaded,
	EQueued,
	EDownloading,
	EDownloaded
};

class CShowInfo: public CBase {
public:
	static CShowInfo* NewL();
	~CShowInfo();
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TDesC& Title() const;
	void SetTitle(TDesC &aTitle);

	TDesC& Url() const;
	void SetUrl(TDesC &aUrl);
	
	TDesC& Description() const;
	void SetDescription(TDesC &aDescription);
		
	TTimeIntervalMicroSeconds& Position();
	void SetPosition(TTimeIntervalMicroSeconds aPosition);

	TPlayState PlayState();
	void SetPlayState(TPlayState aPlayState);

	TDownloadState DownloadState();
	void SetDownloadState(TDownloadState aDownloadState);

	TUint FeedUid();
	void SetFeedUid(TUint aFeedUid);

	void SetUid(TUint aUid);
	TUint Uid() const;
	
	TUint ShowSize();
	void SetShowSize(TUint aShowSize);
	
	const TTime PubDate() const;
	void SetPubDate(TTime aPubDate);
	
	TDesC &FileName();
	void SetFileName(TDesC &aFileName);

private:
	CShowInfo();
	void ConstructL();

private:
	HBufC* iTitle;
	HBufC* iUrl;
	HBufC* iDescription;
	HBufC* iFileName;
	TTimeIntervalMicroSeconds iPosition;
	TPlayState iPlayState;
	TDownloadState iDownloadState;
	TUint iFeedUid;
	TUint iUid;
	TUint iShowSize;
	TTime iPubDate;
};

typedef RPointerArray<CShowInfo> RShowInfoArray;
#endif
