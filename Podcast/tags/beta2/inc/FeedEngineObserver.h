#ifndef FEEDENGINEOBSERVER_H_
#define FEEDENGINEOBSERVER_H_
class MFeedEngineObserver {
public:
	virtual void FeedInfoUpdated(CFeedInfo* aFeedInfo) = 0;
	virtual void FeedDownloadUpdatedL(TInt aPercentOfCurrentDownload) = 0;
	virtual void FeedUpdateComplete() = 0;
};
#endif /*FEEDENGINEOBSERVER_H_*/
