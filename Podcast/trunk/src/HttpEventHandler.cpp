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
#include <http\mhttpdatasupplier.h>
#include "e32debug.h"
#include <utf.h>

CHttpEventHandler::~CHttpEventHandler()
  {
  }

CHttpEventHandler* CHttpEventHandler::NewL(MResultObs& aResObs)
  {
  CHttpEventHandler* me = NewLC(aResObs);
  CleanupStack::Pop(me);
  return me;
  }

void CHttpEventHandler::MHFRunL(RHTTPTransaction aTransaction,
                                const THTTPEvent& aEvent)
  {
  switch (aEvent.iStatus)
    {
    case THTTPEvent::EGotResponseHeaders:
      {
      iResObs.SetConnected();

      iResObs.AddResult(_L("EGotResponseHeaders "));
      } break;

    case THTTPEvent::EGotResponseBodyData:
      {
      // Some (more) body data has been received. Get the body data supplier
      MHTTPDataSupplier* body = aTransaction.Response().Body();
      TPtrC8 dataChunk;
      TBool isLast = body->GetNextDataPart(dataChunk);
      Dump(dataChunk);
      if (isLast)
        RDebug::Print(_L("Last data chunk received"));
      // Done with that bit of body data
      body->ReleaseData();

      //iResObs.AddResult(_L("EGotResponseBodyData "));
      } break;

    case THTTPEvent::EResponseComplete:
      {
      // The transaction's response is complete
      RDebug::Print(_L("Transaction Complete"));
      } break;

    case THTTPEvent::ESucceeded:
      {
      RDebug::Print(_L("Transaction Successful"));
      iResObs.SetDisconnected();
      aTransaction.Close();
      CActiveScheduler::Stop();
      } break;

    case THTTPEvent::EFailed:
      {
      RDebug::Print(_L("Transaction Failed"));
      iResObs.SetDisconnected();
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
      TBuf<50> info;
      info.Format(_L("<unknown event:\n%d>"), aEvent.iStatus);
      RDebug::Print(info);
      // close off the transaction if it's an error
      if (aEvent.iStatus < 0)
        {
        iResObs.SetDisconnected();
        aTransaction.Close();
        CActiveScheduler::Stop();
        }
      }
      break;
    } // end switch
  }

TInt CHttpEventHandler::MHFRunError(TInt aError,
                                    RHTTPTransaction /*aTransaction*/,
                                    const THTTPEvent& /*aEvent*/)
  {
  TBuf<50> info;
  info.Format(_L("MHFRunError %d"), aError);
  RDebug::Print(info);
  return KErrNone;
  }

CHttpEventHandler::CHttpEventHandler(MResultObs& aResObs) : iResObs(aResObs)
  {
  }

CHttpEventHandler* CHttpEventHandler::NewLC(MResultObs& aResObs)
  {
  CHttpEventHandler* me = new (ELeave) CHttpEventHandler(aResObs);
  CleanupStack::PushL(me);
  me->ConstructL();
  return me;
  }

void CHttpEventHandler::ConstructL()
  {
  }

void CHttpEventHandler::Dump(TPtrC8 aDataChunk)
  {
  RDebug::Print(_L("Dump START"));
  HBufC16* buf = HBufC16::NewL(aDataChunk.Length());
  CleanupDeletePushL(buf);
  //buf->Des().Copy(aDataChunk);
  //iResObs.AddResult(*buf);
  TBuf<1000> buffr;
  CnvUtfConverter::ConvertToUnicodeFromUtf8(buffr,aDataChunk);
  RDebug::Print(_L("chunk: %S"), &buffr);
  
  CleanupStack::PopAndDestroy();
  RDebug::Print(_L("Dump END"));
  }

