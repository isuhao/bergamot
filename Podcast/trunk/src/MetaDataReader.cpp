#include "MetaDataReader.h"
#include "id3tag.h"
#include "field.h"
#include <utf.h>
#include <charconv.h>
#include <eikenv.h>
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
	delete iCharConverter;
}

void CMetaDataReader::ConstructL()
{
	RDebug::Print(_L("ConstructL"));
	iPlayer = CMdaAudioPlayerUtility::NewL(*this);

	TCallBack callback(ParseNextShowL, this);
	iParseNextShowCallBack = new (ELeave)CAsyncCallBack(callback, CActive::EPriorityStandard);
	iCharConverter = CCnvCharacterSetConverter::NewL();
	iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierIso88591, CEikonEnv::Static()->FsSession()); 
	iLastConverterCharset = KCharacterSetIdentifierIso88591;
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

void CMetaDataReader::ConvertToUniCodeL(TDes& aDestBuffer, TDes8& aInputBuffer, enum id3_field_textencoding aEncoding)
{
	switch(aEncoding)
	{
	case ID3_FIELD_TEXTENCODING_UTF_8:
		{		
			HBufC* tempBuffer = CnvUtfConverter::ConvertToUnicodeFromUtf8L(aInputBuffer);
			aDestBuffer.Copy(*tempBuffer);
			delete tempBuffer;
		}break;
	case ID3_FIELD_TEXTENCODING_UTF_16:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierUnicodeLittle)
			{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUnicodeLittle, CEikonEnv::Static()->FsSession()); 
				iLastConverterCharset = KCharacterSetIdentifierUnicodeLittle;
			}
			TInt unconvertable = 0;
			TInt state = 0;
			iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable);
		}break;
	case ID3_FIELD_TEXTENCODING_UTF_16BE:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierUnicodeBig)
			{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUnicodeBig, CEikonEnv::Static()->FsSession()); 
				iLastConverterCharset = KCharacterSetIdentifierUnicodeBig;
			}
			TInt unconvertable = 0;
			TInt state = 0;
			iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable);	
		}break;
	case ID3_FIELD_TEXTENCODING_ISO_8859_1:
		{
			if(iLastConverterCharset != KCharacterSetIdentifierIso88591)
				{
				iCharConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierIso88591, CEikonEnv::Static()->FsSession()); 
				iLastConverterCharset = KCharacterSetIdentifierIso88591;
				}
			TInt unconvertable = 0;
			TInt state = 0;
			iCharConverter->ConvertToUnicode(aDestBuffer, aInputBuffer, state, unconvertable);
		}break;
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
				len = 0;
				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}

				iTempDataBuffer.SetLength(len);
				ConvertToUniCodeL(iStringBuffer, iTempDataBuffer, encoding);
				iShow->SetTitleL(iStringBuffer);				
			}

			iStringBuffer.Zero();
	
			frame = id3_tag_findframe(tag, ID3_FRAME_ARTIST, 0);

			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}

				iTempDataBuffer.SetLength(len);
				iTempDataBuffer.Append(_L("\n"));
				ConvertToUniCodeL(iStringBuffer, iTempDataBuffer, encoding);			
			}			
						
			frame = id3_tag_findframe(tag, ID3_FRAME_ALBUM, 0);

			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				
				iTempDataBuffer.SetLength(len);
				ConvertToUniCodeL(iStringBuffer2, iTempDataBuffer, encoding);			
				iStringBuffer.Append(iStringBuffer2);
			}		
					
			iShow->SetDescriptionL(iStringBuffer);	 			

			frame = id3_tag_findframe(tag, ID3_FRAME_TRACK, 0);
			if(frame != NULL)
			{
				len = 0;

				for( cnt = 0;cnt<frame->nfields;cnt++)
				{
					ptr = (char*) iTempDataBuffer.Ptr();
					len = id3_field_render(&frame->fields[cnt], (id3_byte_t**)&ptr, &encoding,0);					
				}
				iTempDataBuffer.SetLength(len);
				ConvertToUniCodeL(iStringBuffer, iTempDataBuffer, encoding);			
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

void CMetaDataReader::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration)
{
	RDebug::Print(_L("MapcInitComplete, file=%S"), &(iShow->FileName()));


	if(aError == KErrNone)
	{
		TInt numEntries = 0;
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
	}

	iObserver.ReadMetaData(iShow);
	iPlayer->Stop();
	iShow = NULL;
	ParseNextShow();
}
