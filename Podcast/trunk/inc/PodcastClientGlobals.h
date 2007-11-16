#ifndef __PODCASTCLIENTGLOBALS_H__
#define __PODCASTCLIENTGLOBALS_H__

const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastBaseViewID = {0x00000001};
const TUid KUidPodcastFeedViewID = {0x00000002};
const TUid KUidPodcastShowsViewID = {0x00000003};
const TUid KUidPodcastPlayViewID = {0x00000004};

#ifdef __WINS__
	_LIT(KPodcastDir, "c:\\logs\\");
#else
	_LIT(KPodcastDir, "e:\\podcasts\\");	
#endif

_LIT(KConfigFile, "config.cfg");
_LIT(KFeedsFileName, "feeds.cfg");
_LIT(KMetaDataFile, "metadata.db");

#endif
