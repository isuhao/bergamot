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

// HttpEventHandler.h

#ifndef HTTPEVENTHANDLER_H
#define HTTPEVENTHANDLER_H

#include <e32base.h>
#include <http\mhttptransactioncallback.h>
#include "MResultObs.h"

class CHttpEventHandler : public CBase,
                          public MHTTPTransactionCallback
  {
public:
  virtual ~CHttpEventHandler();
  static CHttpEventHandler* NewL(MResultObs& aResObs);

  // from MHTTPTransactionCallback
  virtual void MHFRunL(RHTTPTransaction aTransaction,
                       const THTTPEvent& aEvent);
  virtual TInt MHFRunError(TInt aError,
                           RHTTPTransaction aTransaction, 
                           const THTTPEvent& aEvent);

protected:
  CHttpEventHandler(MResultObs& aResObs);

private:
  static CHttpEventHandler* NewLC(MResultObs& aResObs);
  void ConstructL();
  void Dump(TPtrC8 aDataChunk);

private:
  MResultObs& iResObs;
};

#endif