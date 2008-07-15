#ifndef PODCASTCLIENTAUDIOBOOKDLGH
#define PODCASTCLIENTAUDIOBOOKDLGH

#include <eikdialg.h>

class CPodcastModel;

class CPodcastClientAudioBookDlg:public CEikDialog
{
public:
	CPodcastClientAudioBookDlg(CPodcastModel& aPodcastModel, CDesCArrayFlat& aFileNames, TBool aPlayList=EFalse);
	~CPodcastClientAudioBookDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	void PreLayoutDynInitL();
private:
	CPodcastModel& iPodcastModel;
	CDesCArrayFlat& iSelectedFilenames;
	TBool iPlayList;
};
#endif//PODCASTCLIENTAUDIOBOOKDLGH


