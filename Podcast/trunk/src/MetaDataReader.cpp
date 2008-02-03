#include "MetaDataReader.h"

CMetaDataReader::CMetaDataReader(MMetaDataReaderObserver& aObserver) : iObserver(aObserver)
{
	iShow = NULL;
}

CMetaDataReader::~CMetaDataReader()
{
	iPlayer->Close();
	delete iPlayer;
	iShowsToParse.Close();
}

void CMetaDataReader::ConstructL()
{
	RDebug::Print(_L("ConstructL"));
	iPlayer = CMdaAudioPlayerUtility::NewL(*this);
}

void CMetaDataReader::SubmitShow(CShowInfo *aShowInfo)
{
	RDebug::Print(_L("SubmitShow"));
	iShowsToParse.Append(aShowInfo);
	
	if (iShow == NULL) {
		ParseNextShow();
	}
}

void CMetaDataReader::ParseNextShow()
	{
	RDebug::Print(_L("ParseNextShow"));
	iPlayer->Close();
	if (iShowsToParse.Count() == 0) {
		RDebug::Print(_L("No more shows, stopping"));
		iObserver.ReadMetaDataComplete();
		return;
	}
	
	iShow = iShowsToParse[0];
	iShowsToParse.Remove(0);
	
	iPlayer->OpenFileL(iShow->FileName());
	}

void CMetaDataReader::MapcPlayComplete(TInt /*aError*/)
{
	
}

void CMetaDataReader::MapcInitComplete(TInt /*aError*/, const TTimeIntervalMicroSeconds &aDuration)
{
	RDebug::Print(_L("MapcInitComplete, file=%S"), &(iShow->FileName()));

	int numEntries = 0;
	if (iPlayer->GetNumberOfMetaDataEntries(numEntries) == KErrNone) {
		RDebug::Print(_L("%d meta data entries"), numEntries);
		iShow->SetPlayTime((aDuration.Int64()/1000000));

		for (int i=0;i<numEntries;i++) {
			CMMFMetaDataEntry * entry;
			TRAPD(error, entry = iPlayer->GetMetaDataEntryL(i));
			
			if (error != KErrNone) {
				continue;
			}
			TBuf<1024> buf;
			if (entry->Name() == _L("title")) {
				buf.Copy(entry->Value());
				iShow->SetTitleL(buf);
				RDebug::Print(_L("title: %S"), &(iShow->Title()));
			} else if (entry->Name() == _L("artist")) {
				if (iShow->Description().Length() > 0) {
					buf.Copy(iShow->Description());
				}
				buf.Append(_L("\n"));
				buf.Append(entry->Value());
		
				iShow->SetDescriptionL(buf);
			} else if (entry->Name() == _L("album")) {
				if (iShow->Description().Length() > 0) {
					buf.Copy(iShow->Description());
				}
				buf.Append(_L("\n"));
				buf.Append(entry->Value());

				iShow->SetDescriptionL(buf);
			}			
		}
	}
	iObserver.ReadMetaData(iShow);
	iPlayer->Stop();
	iShow = NULL;
	ParseNextShow();
}
