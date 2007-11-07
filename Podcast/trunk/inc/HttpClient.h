#include <e32test.h>
#include <f32file.h>
//#include <http\MHTTPAuthenticationCallback.h>
#include <http\mhttpdatasupplier.h>
#include <chttpformencoder.h>
const TInt KMaxContentTypeSize = 64;

class CHttpEventHandler;

class CHttpClient : public CBase, public MHTTPDataSupplier//, public MHTTPAuthenticationCallback
	{
public:
	virtual ~CHttpClient();
	static CHttpClient* NewLC();
	static CHttpClient* NewL();
	void StartClientL();

public:
	virtual TBool GetNextDataPart(TPtrC8& aDataPart);
	virtual void ReleaseData();
	virtual TInt OverallDataSize();
	virtual TInt Reset();

public:
	virtual TBool GetCredentialsL(const TUriC8& aURI, RString aRealm, 
								 RStringF aAuthenticationType,
								 RString& aUsername, 
								 RString& aPassword);
	
protected:
	CHttpClient();
	void ConstructL();

private:
	void ResetTimeElapsed();
	void DisplayTimeElapsed();
	void InvokeHttpMethodL(const TDesC8& aUri, RStringF aMethod);
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);
	void GetRequestBodyL(RStringF& aMethod);
	void GetPostBodyManuallyL();

private:
	TTime iLastTimeStamp;
	TInt iDataChunkCount;
	RHTTPSession iSess;
	RHTTPTransaction iTrans;
	TBool iHasARequestBody;
	RFs iFileServ;
	RFile iReqBodyFile;
	TFileName iReqBodyFileName;
	TParse iParsedFileName;
	TBuf<KMaxContentTypeSize> iReqBodyContentType;
	CHttpEventHandler* iTransObs;
	HBufC8* iReqBodySubmitBuffer;
	TPtr8 iReqBodySubmitBufferPtr;
	CHTTPFormEncoder* iFormEncoder;
	TBool iManualPost;
	};

class CHttpEventHandler : public CBase, public MHTTPTransactionCallback
	{
public:
	virtual ~CHttpEventHandler();
	static CHttpEventHandler* NewLC();
	static CHttpEventHandler* NewL();
	void SetVerbose(TBool aVerbose);
	TBool Verbose() const;

	virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);

protected:
	CHttpEventHandler();
	void ConstructL();

private:
	void DumpRespHeadersL(RHTTPTransaction& aTrans);
	void DumpRespBody(RHTTPTransaction& aTrans);
	void DumpIt(const TDesC8& aData);

private:
	TBool iVerbose;
	TBool iSavingResponseBody;
	RFs iFileServ;
	RFile iRespBodyFile;
	TFileName iRespBodyFileName;
	TParse iParsedFileName;
	MHTTPDataSupplier* iRespBody;
	};

