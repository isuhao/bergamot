#ifndef SHOWINFO_H_
#define SHOWINFO_H_
#include <e32base.h>
#include <e32cmn.h>
#include <e32std.h>
#include <f32file.h>
#include <s32strm.h>

const int KShowInfoVersion = 4;
const int KTitleLength=256;
const int KUrlLength=1024;
const int KDescriptionLength=2048;

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
	CShowInfo();
	~CShowInfo();
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);
	TDesC& Title();
	void SetTitle(TDesC &aTitle);
	TDesC& Url();
	void SetUrl(TDesC &aUrl);
	TDesC& Description();
	void SetDescription(TDesC &aDescription);
	
public:
	TInt iFeedUid;
	TFileName iFileName;
	TTimeIntervalMicroSeconds iPosition;
	TPlayState iPlayState;
	TDownloadState iDownloadState;
	TInt iUid;
	TUint32 iShowSize;
	TTime iPubDate;
	
private:
	HBufC *iTitle;
	HBufC *iUrl;
	HBufC *iDescription;

};

typedef RPointerArray<CShowInfo> CShowInfoArray;
#endif
