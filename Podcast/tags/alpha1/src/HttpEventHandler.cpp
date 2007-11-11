/***************************************************************************** 
COPYRIGHT All rights reserved Sony Ericsson Mobile Communications AB 2006. 
The software is the copyrighted work of Sony Ericsson Mobile Communications AB. 
The use of the software is subject to the terms of use or of the end-user license 
agreement which accompanies or is included with the software. The software is 
provided "as is" and Sony Ericsson specifically disclaim any warranty or 
condition whatsoever regarding merchantability or fitness for a specific 
purpose, title or non-infringement. No warranty of any kind is made in 
relation to the condition, suitability, availability, accuracy, reliability, 
merchantability and/or non-infringement of the software provided herein. 
*****************************************************************************/

// HttpEventHandler.cpp

#include "HttpEventHandler.h"
#include "e32debug.h"
#include "BAUTILS.H"

void CHttpEventHandler::ConstructL()
	{
	//iVerbose = ETrue;
	User::LeaveIfError(iFileServ.Connect());
	}


CHttpEventHandler::CHttpEventHandler(MHttpEventHandlerCallbacks &aCallbacks): iCallbacks(aCallbacks) 
	{
	}


CHttpEventHandler::~CHttpEventHandler()
	{
	iFileServ.Close();
	}


CHttpEventHandler* CHttpEventHandler::NewLC(MHttpEventHandlerCallbacks &aCallbacks)
	{
	CHttpEventHandler* me = new(ELeave)CHttpEventHandler(aCallbacks);
	CleanupStack::PushL(me);
	me->ConstructL();
	return me;
	}


CHttpEventHandler* CHttpEventHandler::NewL(MHttpEventHandlerCallbacks &aCallbacks)
	{
	CHttpEventHandler* me = NewLC(aCallbacks);
	CleanupStack::Pop(me);
	return me;
	}


void CHttpEventHandler::SetVerbose(TBool aVerbose)
	{
	iVerbose = aVerbose;
	}


TBool CHttpEventHandler::Verbose() const
	{
	return iVerbose;
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
			if (iVerbose)
				DumpRespHeadersL(aTransaction);

			// Determine if the body will be saved to disk
			iSavingResponseBody = ETrue;
			TBool cancelling = EFalse;
			if (resp.HasBody() && (status >= 200) && (status < 300) && (status != 204))
				{
				TInt dataSize = resp.Body()->OverallDataSize();
				if (dataSize >= 0)
					RDebug::Print(_L("Response body size is %d"), dataSize);
				else
					RDebug::Print(_L("Response body size is unknown"));

				iSavingResponseBody =ETrue;
				cancelling = EFalse;
				}

			// If we're cancelling, must do it now..
			if (cancelling)
				{
				RDebug::Print(_L("Transaction Cancelled"));
				aTransaction.Close();
				CActiveScheduler::Stop();
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
					RDebug::Print(_L("Setting file to save to"));
					TInt err = iRespBodyFile.Replace(iFileServ,
													 iParsedFileName.FullName(),
													 EFileWrite);
					if (err)
						{
						RDebug::Print(_L("There was an error"));
						iSavingResponseBody = EFalse;
						User::Leave(err);
						}
					}
				}

			} break;
		case THTTPEvent::EGotResponseBodyData:
			{
			// Get the body data supplier
			iRespBody = aTransaction.Response().Body();

			// Some (more) body data has been received (in the HTTP response)
			if (iVerbose)
				DumpRespBody(aTransaction);
			
			// Append to the output file if we're saving responses
			if (iSavingResponseBody)
				{
				TPtrC8 bodyData;
				TBool lastChunk = iRespBody->GetNextDataPart(bodyData);
				User::InfoPrint(_L("Downloading..."));
				iRespBodyFile.Write(bodyData);
				if (lastChunk)
					//iRespBodyFile.Flush();
					iRespBodyFile.Close();
				}

			// Done with that bit of body data
			iRespBody->ReleaseData();
			} break;
		case THTTPEvent::EResponseComplete:
			{
			// The transaction's response is complete

			RDebug::Print(_L("Transaction Complete"));
			} break;
		case THTTPEvent::ESucceeded:
			{
			aTransaction.Close();
			CActiveScheduler::Stop();
			iCallbacks.FileCompleteCallback(iFileName);
			RDebug::Print(_L("Transaction Successful"));

			} break;
		case THTTPEvent::EFailed:
			{
			RDebug::Print(_L("Transaction Failed"));
			aTransaction.Close();
			CActiveScheduler::Stop();
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
				CActiveScheduler::Stop();
				}
			} break;
		}
	}

TInt CHttpEventHandler::MHFRunError(TInt aError, RHTTPTransaction /*aTransaction*/, const THTTPEvent& /*aEvent*/)
	{
	RDebug::Print(_L("MHFRunError fired with error code %d"), aError);

	return KErrNone;
	}

void CHttpEventHandler::SetSaveFileName(TDesC &fName)
	{
	iFileName.Copy(fName);
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
			/*case THTTPHdrVal::KDateVal:
				/*{
				TDateTime date = fieldVal.DateTime();
				TBuf<40> dateTimeString;
				TTime t(date);
				t.FormatL(dateTimeString,KDateFormat);

				RDebug::Print(_L("%S: %S"), &fieldName16, &dateTimeString);
				} 
				break;*/
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
