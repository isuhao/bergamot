#include "MetaDataReader.h"
#include "id3tag.h"
#include "field.h"
_LIT(KMP3Extension, ".MP3");
CMetaDataReader::CMetaDataReader(MMetaDataReaderObserver& aObserver) : iObserver(aObserver)
{
	iShow = NULL;
}

CMetaDataReader::~CMetaDataReader()
{
	iPlayer->Close();
	delete iPlayer;
	delete iParseNextShowCallBack;
	iShowsToParse.Close();
}

void CMetaDataReader::ConstructL()
{
	RDebug::Print(_L("ConstructL"));
	iPlayer = CMdaAudioPlayerUtility::NewL(*this);

	TCallBack callback(ParseNextShowL, this);
	iParseNextShowCallBack = new (ELeave)CAsyncCallBack(callback, CActive::EPriorityStandard);
}

void CMetaDataReader::SubmitShow(CShowInfo *aShowInfo)
{
	RDebug::Print(_L("SubmitShow"));
	iShowsToParse.Append(aShowInfo);
	
	if (iShow == NULL) {
		ParseNextShow();
	}
}

TInt CMetaDataReader::ParseNextShowL(TAny* aMetaDataReader)
{
	static_cast<CMetaDataReader*>(aMetaDataReader)->ParseNextShow();
	return KErrNone;
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
	TParsePtrC parser(iShow->FileName());

	if(parser.Ext().CompareF(KMP3Extension()) == 0)
	{
		iTempFileName.Copy(iShow->FileName());
		iTempFileName.PtrZ();
		struct id3_file * id3_file = id3_file_open((const char*) iTempFileName.Ptr(), ID3_FILE_MODE_READONLY);
		
		if(id3_file != NULL)
		{
			id3_tag * tag = id3_file_tag(id3_file);
			id3_length_t len;
			id3_frame * frame = id3_tag_findframe(tag, ID3_FRAME_TITLE, 0);
			TInt cnt;
			char* ptr = NULL;
			enum id3_field_textencoding encoding = ID3_FIELD_TEXTENCODING_UTF_8;
			if(frame != NULL)
			{
				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				iTempDataBuffer.SetLength(len);
				iStringBuffer.Copy(iTempDataBuffer);
				iShow->SetTitleL(iStringBuffer);
			}
			
			frame = id3_tag_findframe(tag, ID3_FRAME_ARTIST, 0);

			iStringBuffer.Zero();

			if(frame != NULL)
			{
				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				iTempDataBuffer.SetLength(len);
				iStringBuffer.Copy(iTempDataBuffer);
				iShow->SetDescriptionL(iStringBuffer);
			}
			
			if(frame == NULL || iStringBuffer.Length() == 0)
			{
				frame = id3_tag_findframe(tag, ID3_FRAME_ALBUM, 0);
				if(frame != NULL)
				{
					for( cnt = 0;cnt<frame->nfields;cnt++)
					{
						ptr = (char*) iTempDataBuffer.Ptr();
						len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
					}
					iTempDataBuffer.SetLength(len);
					iStringBuffer.Copy(iTempDataBuffer);
					iShow->SetDescriptionL(iStringBuffer);
				}
			}
					 			
			id3_file_close(id3_file);
		}
		iObserver.ReadMetaData(iShow);
		iShow = NULL;
		iParseNextShowCallBack->CallBack();
	}
	else
	{
		iPlayer->OpenFileL(iShow->FileName());
	}
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
