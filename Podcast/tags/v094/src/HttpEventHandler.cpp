
// HttpEventHandler.cpp
#include <e32debug.h>

#include "HttpEventHandler.h"
#include "bautils.h"
#include "Httpclient.h"

void CHttpEventHandler::ConstructL()
	{
	//iVerbose = ETrue;
	User::LeaveIfError(iFileServ.Connect());
	}


CHttpEventHandler::CHttpEventHandler(CHttpClient* aClient, MHttpClientObserver &aCallbacks): iHttpClient(aClient), iCallbacks(aCallbacks) 
	{
	}


CHttpEventHandler::~CHttpEventHandler()
	{
	iFileServ.Close();
	}


CHttpEventHandler* CHttpEventHandler::NewLC(CHttpClient* aClient, MHttpClientObserver &aCallbacks)
	{
	CHttpEventHandler* me = new(ELeave)CHttpEventHandler(aClient, aCallbacks);
	CleanupStack::PushL(me);
	me->ConstructL();
	return me;
	}


CHttpEventHandler* CHttpEventHandler::NewL(CHttpClient* aClient, MHttpClientObserver &aCallbacks)
	{
	CHttpEventHandler* me = NewLC(aClient, aCallbacks);
	CleanupStack::Pop(me);
	return me;
	}

void CHttpEventHandler::MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent)
	{
	switch (aEvent.iStatus)
		{
		case THTTPEvent::EGotResponseHeaders:
			{
			// HTTP response headers have been received. We can determine now if there is
			// going to be a response body to save.
			RHTTPResponse resp = aTransaction.Response();
			TInt status = resp.StatusCode();
			RStringF statusStr = resp.StatusText();
			TBuf<32> statusStr16;
			statusStr16.Copy(statusStr.DesC());
			RDebug::Print(_L("Status: %d (%S)"), status, &statusStr16);

			// Dump the headers if we're being verbose
			//DumpRespHeadersL(aTransaction);

			// Determine if the body will be saved to disk
			iSavingResponseBody = ETrue;
			TBool cancelling = EFalse;
			if (resp.HasBody() && (status >= 200) && (status < 300) && (status != 204))
				{
				//iBytesDownloaded = 0;
				TInt dataSize = resp.Body()->OverallDataSize();
				if (dataSize >= 0) {
					RDebug::Print(_L("Response body size is %d"), dataSize);
					iBytesTotal = dataSize;	
				} else {
					RDebug::Print(_L("Response body size is unknown"));
					iBytesTotal = -1;
				}
				iCallbacks.DownloadInfo(iHttpClient, dataSize);

				cancelling = EFalse;
				}

			// If we're cancelling, must do it now..
			if (cancelling)
				{
				RDebug::Print(_L("Transaction Cancelled"));
				aTransaction.Close();
				iHttpClient->ClientRequestCompleteL(EFalse);
				}
			else if (iSavingResponseBody) // If we're saving, then open a file handle for the new file
				{
				iFileServ.Parse(iFileName, iParsedFileName);
				TInt valid = iFileServ.IsValidName(iFileName);
				if (!valid)
					{
					RDebug::Print(_L("The specified filename is not valid!."));
					iSavingResponseBody = EFalse;
					}
				else
					{
					if (iContinue) {
						TInt err = iRespBodyFile.Open(iFileServ, iParsedFileName.FullName(),EFileWrite);
						if (err)
							{
							RDebug::Print(_L("There was an error opening file"));
							iSavingResponseBody = EFalse;
							User::Leave(err);
							} else {
							int pos = -KByteOverlap;
							if((err=iRespBodyFile.Seek(ESeekEnd, pos)) != KErrNone) {
								RDebug::Print(_L("Failed to set position!"));
								User::Leave(err);
							}
							iBytesDownloaded = pos;
							iBytesTotal += iBytesDownloaded;
							RDebug::Print(_L("Total bytes is now %u"), iBytesTotal);
							RDebug::Print(_L("Seeking end: %d"), pos);
							}
					} else {
						TInt err = iRespBodyFile.Replace(iFileServ,
														 iParsedFileName.FullName(),
														 EFileWrite);
						if (err)
							{
							RDebug::Print(_L("There was an error replacing file"));
							iSavingResponseBody = EFalse;
							User::Leave(err);
							}
						}
					}
				}

			} break;
		case THTTPEvent::EGotResponseBodyData:
			{
			// Get the body data supplier
			iRespBody = aTransaction.Response().Body();

			// Some (more) body data has been received (in the HTTP response)
			//DumpRespBody(aTransaction);
			//RDebug::Print(_L("Saving: %d"), iSavingResponseBody);
			// Append to the output file if we're saving responses
			if (iSavingResponseBody)
				{
				TPtrC8 bodyData;
				iRespBody->GetNextDataPart(bodyData);
				iBytesDownloaded += bodyData.Length();
				iRespBodyFile.Write(bodyData);
				if (!iSilent) {
					iCallbacks.Progress(iHttpClient, iBytesDownloaded, iBytesTotal);
				}
				}

			// Done with that bit of body data
			iRespBody->ReleaseData();
			} break;
		case THTTPEvent::EResponseComplete:
			{
			// The transaction's response is complete

			RDebug::Print(_L("Transaction Complete"));
			RDebug::Print(_L("Closing file"));
			iRespBodyFile.Close();
			} break;
		case THTTPEvent::ESucceeded:
			{
			RDebug::Print(_L("Transaction Successful"));
			aTransaction.Close();
			iHttpClient->ClientRequestCompleteL(ETrue);
			} break;
		case THTTPEvent::EFailed:
			{
			RDebug::Print(_L("Transaction Failed"));
			aTransaction.Close();
			iHttpClient->ClientRequestCompleteL(EFalse);
			} break;
		case THTTPEvent::ERedirectedPermanently:
			{
			RDebug::Print(_L("Permanent Redirection"));
			} break;
		case THTTPEvent::ERedirectedTemporarily:
			{
			RDebug::Print(_L("Temporary Redirection"));
			} break;
		default:
			{
			RDebug::Print(_L("<unrecognised event: %d>"), aEvent.iStatus);
			// close off the transaction if it's an error
			if (aEvent.iStatus < 0)
				{
				aTransaction.Close();
				iHttpClient->ClientRequestCompleteL(EFalse);
				}
			} break;
		}
	}

TInt CHttpEventHandler::MHFRunError(TInt aError, RHTTPTransaction /*aTransaction*/, const THTTPEvent& /*aEvent*/)
	{
	RDebug::Print(_L("MHFRunError fired with error code %d"), aError);

	return KErrNone;
	}

void CHttpEventHandler::SetSaveFileName(const TDesC &fName, TBool aContinue)
	{
	iFileName.Copy(fName);
	iContinue = aContinue;
	}

void CHttpEventHandler::DumpRespHeadersL(RHTTPTransaction& aTrans)
	{
	RHTTPResponse resp = aTrans.Response();
	RStringPool strP = aTrans.Session().StringPool();
	RHTTPHeaders hdr = resp.GetHeaderCollection();
	THTTPHdrFieldIter it = hdr.Fields();

	TBuf<KMaxHeaderNameLen>  fieldName16;
	TBuf<KMaxHeaderValueLen> fieldVal16;

	while (it.AtEnd() == EFalse)
		{
		RStringTokenF fieldName = it();
		RStringF fieldNameStr = strP.StringF(fieldName);
		THTTPHdrVal fieldVal;
		if (hdr.GetField(fieldNameStr,0,fieldVal) == KErrNone)
			{
			const TDesC8& fieldNameDesC = fieldNameStr.DesC();
			fieldName16.Copy(fieldNameDesC.Left(KMaxHeaderNameLen));
			switch (fieldVal.Type())
				{
			case THTTPHdrVal::KTIntVal:
				RDebug::Print(_L("%S: %d"), &fieldName16, fieldVal.Int());
				break;
			case THTTPHdrVal::KStrFVal:
				{
				RStringF fieldValStr = strP.StringF(fieldVal.StrF());
				const TDesC8& fieldValDesC = fieldValStr.DesC();
				fieldVal16.Copy(fieldValDesC.Left(KMaxHeaderValueLen));
				RDebug::Print(_L("%S: %S"), &fieldName16, &fieldVal16);
				}
				break;
			case THTTPHdrVal::KStrVal:
				{
				RString fieldValStr = strP.String(fieldVal.Str());
				const TDesC8& fieldValDesC = fieldValStr.DesC();
				fieldVal16.Copy(fieldValDesC.Left(KMaxHeaderValueLen));
				RDebug::Print(_L("%S: %S"), &fieldName16, &fieldVal16);
				}
				break;
			case THTTPHdrVal::KDateVal:
				{
				TDateTime date = fieldVal.DateTime();
				} 
				break;
			default:
				RDebug::Print(_L("%S: <unrecognised value type>"), &fieldName16);
				break;
				}

			// Display realm for WWW-Authenticate header
			RStringF wwwAuth = strP.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable());
			if (fieldNameStr == wwwAuth)
				{
				// check the auth scheme is 'basic'
				RStringF basic = strP.StringF(HTTP::EBasic,RHTTPSession::GetTable());
				RStringF realm = strP.StringF(HTTP::ERealm,RHTTPSession::GetTable());
				THTTPHdrVal realmVal;
				if ((fieldVal.StrF() == basic) && 
					(!hdr.GetParam(wwwAuth, realm, realmVal)))
					{
					RStringF realmValStr = strP.StringF(realmVal.StrF());
					fieldVal16.Copy(realmValStr.DesC());
					RDebug::Print(_L("Realm is: %S"), &fieldVal16);
					}
				}
			}
		++it;
		}
	}

void CHttpEventHandler::DumpRespBody(RHTTPTransaction& aTrans)
	{
	MHTTPDataSupplier* body = aTrans.Response().Body();
	TPtrC8 dataChunk;
	TBool isLast = body->GetNextDataPart(dataChunk);
	DumpIt(dataChunk);
	if (isLast)
		RDebug::Print(_L("Got last data chunk."));
	}


void CHttpEventHandler::DumpIt(const TDesC8& aData)
//Do a formatted dump of binary data
	{
	// Iterate the supplied block of data in blocks of cols=80 bytes
	const TInt cols=16;
	TInt pos = 0;
	TBuf<KMaxFileName - 2> logLine;
	TBuf<KMaxFileName - 2> anEntry;
	const TInt dataLength = aData.Length();

	while (pos < dataLength)
		{
		//start-line hexadecimal( a 4 digit number)
		anEntry.Format(TRefByValue<const TDesC>_L("%04x : "), pos);
		logLine.Append(anEntry);

		// Hex output
		TInt offset;
		for (offset = 0; offset < cols; ++offset)
			{
			if (pos + offset < aData.Length())
				{
				TInt nextByte = aData[pos + offset];
				anEntry.Format(TRefByValue<const TDesC>_L("%02x "), nextByte);
				logLine.Append(anEntry);
				}
			else
				{
				//fill the remaining spaces with blanks untill the cols-th Hex number 
				anEntry.Format(TRefByValue<const TDesC>_L("   "));
				logLine.Append(anEntry);
				}
			}
			anEntry.Format(TRefByValue<const TDesC>_L(": "));
			logLine.Append(anEntry);

		// Char output
		for (offset = 0; offset < cols; ++offset)
			{
			if (pos + offset < aData.Length())
				{
				TInt nextByte = aData[pos + offset];
				if ((nextByte >= ' ') && (nextByte <= '~'))
					{
					anEntry.Format(TRefByValue<const TDesC>_L("%c"), nextByte);
					logLine.Append(anEntry);
					}
				else
					{
					anEntry.Format(TRefByValue<const TDesC>_L("."));
					logLine.Append(anEntry);
					}
				}
			else
				{
				anEntry.Format(TRefByValue<const TDesC>_L(" "));
				logLine.Append(anEntry);
				}
			}
			RDebug::Print(TRefByValue<const TDesC>_L("%S"), &logLine);	
			logLine.Zero();

		// Advance to next  byte segment (1 seg= cols)
		pos += cols;
		}
	}

void CHttpEventHandler::SetSilent(TBool aSilent)
	{
	iSilent = aSilent;
	}

void CHttpEventHandler::CloseSaveFile()
{
	if(iRespBody != NULL)
	{
		TInt size;
		
		iRespBodyFile.Size(size);
		RDebug::Print(_L("Closing file at size %d, bytes downloaded %d"), size, iBytesDownloaded);
		iRespBodyFile.Close();
	}
}
