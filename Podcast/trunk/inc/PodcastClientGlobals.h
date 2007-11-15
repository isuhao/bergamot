#ifndef __PODCASTCLIENTGLOBALS_H__
#define __PODCASTCLIENTGLOBALS_H__

const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastClientBaseViewID = {0x00000001};
const TUid KUidPodcastClientPlayViewID = {0x00000002};
const TUid KUidPodcastClientFeedViewID = {0x00000003};
const TUid KUidPodcastClientPodcastsViewID = {0x00000004};

const TUid KUidPodcastBaseView = {KUidPodcastClientBaseViewID};
const TUid KUidPodcastPlayView = {KUidPodcastClientPlayViewID};
const TUid KUidPodcastFeedView = {KUidPodcastClientFeedViewID};
const TUid KUidPodcastPodcastsView = {KUidPodcastClientPodcastsViewID};

#ifdef __WINS__
	_LIT(KPodcastDir, "c:\\logs\\");
#else
	_LIT(KPodcastDir, "d:\\podcasts\\");	
#endif

_LIT(KConfigFile, "config.cfg");
_LIT(KFeedsFileName, "feeds.cfg");
_LIT(KMetaDataFile, "metadata.db");

#endif
