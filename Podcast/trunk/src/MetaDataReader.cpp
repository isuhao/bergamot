#include "MetaDataReader.h"

CMetaDataReader::CMetaDataReader()
{
	iShow = NULL;
}

CMetaDataReader::~CMetaDataReader()
{
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
	
		return;
	}
	
	iShow = iShowsToParse[0];
	iShowsToParse.Remove(0);
	
	iPlayer->OpenFileL(iShow->FileName());
	}

void CMetaDataReader::MapcPlayComplete(TInt aError)
{
	
}

void CMetaDataReader::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration)
{
	RDebug::Print(_L("MapcInitComplete"));

	int numEntries = 0;
	if (iPlayer->GetNumberOfMetaDataEntries(numEntries) == KErrNone) {
		RDebug::Print(_L("%d meta data entries"), numEntries);
		for (int i=0;i<numEntries;i++) {
			CMMFMetaDataEntry * entry;
			TRAPD(error, entry = iPlayer->GetMetaDataEntryL(i));
			
			if (error != KErrNone) {
				continue;
			}
			TBuf<1024> buf;
			if (entry->Name() == _L("title")) {
				buf.Copy(entry->Value());
				iShow->SetTitle(buf);
				RDebug::Print(_L("title: %S"), &(iShow->Title()));
			} else if (entry->Name() == _L("artist")) {
				if (iShow->Description().Length() > 0) {
					buf.Copy(iShow->Description());
				}
				buf.Append(_L("\n"));
				buf.Append(entry->Value());
		
				iShow->SetDescription(buf);
			} else if (entry->Name() == _L("album")) {
				if (iShow->Description().Length() > 0) {
					buf.Copy(iShow->Description());
				}
				buf.Append(_L("\n"));
				buf.Append(entry->Value());

				iShow->SetDescription(buf);
			}			
		}
	}
	iPlayer->Stop();
	iShow = NULL;
	ParseNextShow();
}
