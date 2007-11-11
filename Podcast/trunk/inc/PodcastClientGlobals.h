#ifndef __PODCASTCLIENTGLOBALS_H__
#define __PODCASTCLIENTGLOBALS_H__

const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastClientBaseViewID = {0x00000001};
const TUid KUidListBoxListView = {0x00001000};

#ifdef __WINS__
	_LIT(KPodcastDir, "c:\\logs\\");
#else
	_LIT(KPodcastDir, "d:\\podcasts\\");	
#endif

_LIT(KConfigFile, "config.cfg");
_LIT(KFeedsFileName, "feeds.cfg");

#endif
