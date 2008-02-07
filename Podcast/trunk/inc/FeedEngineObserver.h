#ifndef FEEDENGINEOBSERVER_H_
#define FEEDENGINEOBSERVER_H_
class MFeedEngineObserver {
public:
	virtual void FeedInfoUpdated(CFeedInfo* aFeedInfo) = 0;
	virtual void FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload) = 0;
	virtual void FeedUpdateComplete(TUint aFeedUid) = 0;
};
#endif /*FEEDENGINEOBSERVER_H_*/
