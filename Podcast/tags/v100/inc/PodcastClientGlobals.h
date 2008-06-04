#ifndef __PODCASTCLIENTGLOBALS_H__
#define __PODCASTCLIENTGLOBALS_H__

const TUid KUidPodcastClientID          = {0xA00046AE};
const TUid KUidPodcastBaseViewID = {0x00000001};
const TUid KUidPodcastFeedViewID = {0x00000002};
const TUid KUidPodcastShowsViewID = {0x00000003};
const TUid KUidPodcastPlayViewID = {0x00000004};

const TUint KSizeKb = 1024;
const TUint KSizeMb = 1024000;

_LIT(KShowsSizeFormat,"%.1f");
_LIT(KShowsSizeUnit,"MB");
_LIT(KDateFormat,"%D%M%Y%/0%1%/1%2%/2%3%/3");
// Date/time format string
_LIT(KTimeFormat, "%-B%:0%J%:1%T%+B");
#endif
