#ifndef FEEDENGINEOBSERVER_H_
#define FEEDENGINEOBSERVER_H_
class MFeedEngineObserver {
public:
	virtual void ShowListUpdated() = 0;
	virtual void FeedInfoUpdated(const TFeedInfo& aFeedInfo) = 0;
};
#endif /*FEEDENGINEOBSERVER_H_*/
