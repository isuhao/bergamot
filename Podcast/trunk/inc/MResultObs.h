#ifndef _MRESULTOBS_H_
#define _MRESULTOBS_H_

class MResultObs
{
public:
  virtual void AddResult(const TDesC& aText) = 0;
  virtual void SetConnected() = 0;
  virtual void SetDisconnected() = 0;
};

#endif