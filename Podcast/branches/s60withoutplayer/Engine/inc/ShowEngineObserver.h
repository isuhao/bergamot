/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef SHOWENGINEOBSERVER_H_
#define SHOWENGINEOBSERVER_H_
class MShowEngineObserver {
public:
	virtual void ShowListUpdated() = 0;
	virtual void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows) = 0;
	virtual void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal) = 0;
};
#endif /*SHOWENGINEOBSERVER_H_*/