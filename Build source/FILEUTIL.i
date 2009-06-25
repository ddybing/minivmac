/*
	FILEUTIL.i
	Copyright (C) 2007 Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	FILE UTILilities
*/


struct MyDir_R {
	long DirId;
	short VRefNum;
};
typedef struct MyDir_R MyDir_R;

GLOBALFUNC OSErr MyHGetDir_v2(MyDir_R *d)
{
	OSErr err;
	WDPBRec r;

	r.ioCompletion = NULL;
	r.ioNamePtr = NULL;

#if Support64kROM
	if (Have64kROM()) {
		err = PBGetVolSync((ParamBlockRec *)&r);
		d->VRefNum = r.ioVRefNum;
		d->DirId = 0;
	} else
#endif
	{
		err = PBHGetVolSync(&r);
		d->VRefNum = r.ioWDVRefNum;
		d->DirId = r.ioWDDirID;
	}
	return err;
}

GLOBALFUNC blnr MyHGetDir(MyDir_R *d)
{
	return CheckSysErr(MyHGetDir_v2(d));
}

GLOBALFUNC blnr MyMakeNamedDir(MyDir_R *d, StringPtr s,
	MyDir_R *new_d)
{
	HParamBlockRec r;
	blnr IsOk = falseblnr;

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioDirID = d->DirId;
	r.fileParam.ioNamePtr = s;
	if (CheckSysErr(PBDirCreateSync(&r))) {
		new_d->VRefNum = d->VRefNum;
		new_d->DirId = r.fileParam.ioDirID;
		IsOk = trueblnr;
	}

	return IsOk;
}

GLOBALFUNC blnr MyDeleteFile(MyDir_R *d, StringPtr s)
{
	HParamBlockRec r;
	OSErr err;

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioNamePtr = s;

#if Support64kROM
	if (Have64kROM()) {
		r.fileParam.ioFVersNum = 0;
		err = PBDeleteSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHDeleteSync(&r);
	}

	return CheckSysErr(err);
}

#define NotAfileRef (-1)

/*
	Probably should use PBHOpenDF instead
	of PBHOpen when it is available.
	(System 7 according to Technical Note FL515)
*/

LOCALFUNC OSErr MyFileOpen00(MyDir_R *d, StringPtr s,
	char Permssn, short *refnum)
{
	HParamBlockRec r;
	OSErr err;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = s;
	r.ioParam.ioVRefNum = d->VRefNum;
	r.ioParam.ioPermssn = Permssn;
	r.ioParam.ioMisc = 0; /* use volume buffer */

#if Support64kROM
	if (Have64kROM()) {
		r.ioParam.ioVersNum = 0;
		err = PBOpenSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHOpenSync(&r);
	}

	if (noErr == err) {
		*refnum = r.ioParam.ioRefNum;
		/*
			Don't change *refnum unless file opened,
			so can initialize to NotAfileRef, and
			compare later before closing in uninit.
		*/
	}
	return err;
}

LOCALFUNC blnr MyFileOpen0(MyDir_R *d, StringPtr s,
	char Permssn, short *refnum)
{
	return CheckSysErr(MyFileOpen00(d, s, Permssn, refnum));
}

GLOBALFUNC blnr MyOpenOldFileRead(MyDir_R *d, StringPtr s,
	short *refnum)
{
	return MyFileOpen0(d, s, (char)fsRdPerm, refnum);
}

GLOBALFUNC blnr MyFileOpenWrite(MyDir_R *d, StringPtr s,
	short *refnum)
{
	return MyFileOpen0(d, s, (char)fsWrPerm, refnum);
}

GLOBALFUNC blnr MyFileOpenRFWrite(MyDir_R *d, StringPtr s,
	short *refnum)
{
	HParamBlockRec r;
	OSErr err;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = s;
	r.ioParam.ioVRefNum = d->VRefNum;
	r.ioParam.ioPermssn = (char)fsWrPerm;
	r.ioParam.ioMisc = 0; /* use volume buffer */

#if Support64kROM
	if (Have64kROM()) {
		r.ioParam.ioVersNum = 0;
		err = PBOpenRFSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHOpenRFSync(&r);
	}

	if (noErr == err) {
		*refnum = r.ioParam.ioRefNum;
	}
	return CheckSysErr(err);
}

LOCALFUNC OSErr MyCreateFile0(MyDir_R *d, StringPtr s)
{
	HParamBlockRec r;
	OSErr err;

	r.fileParam.ioFlVersNum = 0;
		/*
			Think reference say to do this,
			but not Inside Mac IV
		*/

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioNamePtr = s;
	r.fileParam.ioVRefNum = d->VRefNum;

#if Support64kROM
	if (Have64kROM()) {
		r.fileParam.ioFVersNum = 0;
		err = PBCreateSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHCreateSync(&r);
	}

	return err;
}

LOCALFUNC OSErr MyCreateFile(MyDir_R *d, StringPtr s)
{
	return CheckSysErr(MyCreateFile0(d, s));
}

LOCALFUNC blnr MyCreateFileOverWrite(MyDir_R *d, StringPtr s)
{
	OSErr err;
	blnr IsOk = falseblnr;

	err = MyCreateFile0(d, s);
	if (dupFNErr == err) {
		if (MyDeleteFile(d, s)) {
			IsOk = MyCreateFile(d, s);
		}
	} else {
		IsOk = CheckSysErr(err);
	}

	return IsOk;
}

GLOBALFUNC blnr MyFileGetInfo(MyDir_R *d, StringPtr s,
	HParamBlockRec *r)
{
	OSErr err;

	r->fileParam.ioCompletion = NULL;
	r->fileParam.ioNamePtr = s;
	r->fileParam.ioVRefNum = d->VRefNum;
	r->fileParam.ioFVersNum = (char)0;
	r->fileParam.ioFDirIndex = (short)0;

#if Support64kROM
	if (Have64kROM()) {
		err = PBGetFInfoSync((ParamBlockRec *)r);
	} else
#endif
	{
		r->fileParam.ioDirID = d->DirId;
		err = PBHGetFInfoSync(r);
	}

	return CheckSysErr(err);
}

GLOBALFUNC blnr MyFileSetInfo(MyDir_R *d, StringPtr s,
	HParamBlockRec *r)
{
	OSErr err;

	r->fileParam.ioCompletion = NULL;
	r->fileParam.ioNamePtr = s;
	r->fileParam.ioVRefNum = d->VRefNum;

#if Support64kROM
	if (Have64kROM()) {
		r->fileParam.ioFVersNum = (char)0;
		err = PBSetFInfoSync((ParamBlockRec *)r);
	} else
#endif
	{
		r->fileParam.ioDirID = d->DirId;
		err = PBHSetFInfoSync(r);
	}

	return CheckSysErr(err);
}

GLOBALFUNC blnr MyFileSetTypeCreator(MyDir_R *d, StringPtr s,
	OSType creator, OSType fileType)
{
	HParamBlockRec r;
	blnr IsOk = falseblnr;

	if (MyFileGetInfo(d, s, &r)) {
		r.fileParam.ioFlFndrInfo.fdType = fileType;
		r.fileParam.ioFlFndrInfo.fdCreator = creator;
		if (MyFileSetInfo(d, s, &r)) {
			IsOk = trueblnr;
		}
	}

	return IsOk;
}

GLOBALFUNC blnr MyOpenNewFile(MyDir_R *d, StringPtr s,
	OSType creator, OSType fileType,
	short *refnum)
{
	blnr IsOk = falseblnr;

	if (MyCreateFile(d, s)) {
		if (MyFileSetTypeCreator(d, s,
			creator, fileType))
		{
			IsOk = MyFileOpenWrite(d, s, refnum);
		}
		if (! IsOk) {
			(void) MyDeleteFile(d, s);
		}
	}

	return IsOk;
}

GLOBALFUNC blnr MyCloseFile(short refNum)
{
	return CheckSysErr(FSClose(refNum));
}

GLOBALFUNC blnr MyWriteBytes(short refNum, MyPtr p, uimr L)
{
	ParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioRefNum = refNum;
	r.ioParam.ioBuffer = (Ptr)p;
	r.ioParam.ioReqCount = L;
	r.ioParam.ioPosMode = (short) fsFromMark;
	r.ioParam.ioPosOffset = 0;

	return CheckSysErr(PBWriteSync(&r));
}

GLOBALFUNC blnr MyReadBytes(short refNum, MyPtr p, uimr L)
{
	ParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioRefNum = refNum;
	r.ioParam.ioBuffer = (Ptr)p;
	r.ioParam.ioReqCount = L;
	r.ioParam.ioPosMode = (short) fsFromMark;
	r.ioParam.ioPosOffset = 0;

	return CheckSysErr(PBReadSync(&r));
}

GLOBALFUNC blnr MyBackWriteBytes(short refNum, uimr offset,
	MyPtr p, uimr L)
{
	long savepos;
	blnr IsOk = falseblnr;

	if (CheckSysErr(GetFPos(refNum, &savepos)))
	if (CheckSysErr(SetFPos(refNum, fsFromStart, offset)))
	if (MyWriteBytes(refNum, p, L))
	if (CheckSysErr(SetFPos(refNum, fsFromStart, savepos)))
	{
		IsOk = trueblnr;
	}

	return IsOk;
}

GLOBALFUNC blnr MyBackReadBytes(short refNum, uimr offset,
	MyPtr p, uimr L)
{
	long savepos;
	blnr IsOk = falseblnr;

	if (CheckSysErr(GetFPos(refNum, &savepos)))
	if (CheckSysErr(SetFPos(refNum, fsFromStart, offset)))
	if (MyReadBytes(refNum, p, L))
	if (CheckSysErr(SetFPos(refNum, fsFromStart, savepos)))
	{
		IsOk = trueblnr;
	}

	return IsOk;
}

LOCALFUNC blnr MyOpenFileGetEOF(short refnum, uimr *L)
{
	return CheckSysErr(GetEOF(refnum, (long *)L));
}

LOCALFUNC blnr MyOpenFileSetEOF(short refnum, uimr L)
{
	return CheckSysErr(SetEOF(refnum, (long)L));
}

LOCALPROC MyFileFlush(short refnum)
{
	short vRefNum;

#if Support64kROM
	if (Have64kROM()) {
		vCheckSysErr(-1);
			/*
				fix me, GetVRefNum glue broken in MPW 3.
				Thinks FCB for pre-HFS is 94 instead of 30.
			*/
		return;
	}
#endif

	if (CheckSysErr(GetVRefNum(refnum, &vRefNum))) {
		vCheckSysErr(FlushVol(NULL, vRefNum));
	}
}

LOCALFUNC blnr MyOpenFileUpdtLocation(short refNum,
	MyDir_R *d, ps3p s)
{
	/*
		because the user could move or rename the
		file while the program is working on it.
	*/
	blnr IsOk = falseblnr;

#if Support64kROM
	if (Have64kROM()) {
		IsOk = trueblnr;
		/*
			Do nothing. Volume of open file can't
			change. And haven't figured out a
			way to get name.
		*/
	} else
#endif
	{
		FCBPBRec b;

		b.ioCompletion = NULL;
		b.ioNamePtr = (StringPtr)s;
		b.ioVRefNum = 0;
		b.ioRefNum = refNum;
		b.ioFCBIndx = 0;
		if (CheckSysErr(PBGetFCBInfoSync(&b))) {
			d->VRefNum = b.ioFCBVRefNum;
			d->DirId = b.ioFCBParID;
			IsOk = trueblnr;
		}
	}

	return IsOk;
}

LOCALPROC MyCloseNewFile(short refNum, MyDir_R *d, ps3p s, blnr KeepIt)
/* s may be modified, must be Str255 (and d modified too) */
{
	blnr DoDelete;

	DoDelete = (! KeepIt)
		&& MyOpenFileUpdtLocation(refNum, d, s);
	(void) MyCloseFile(refNum);
	if (DoDelete) {
		(void) MyDeleteFile(d, s);
	}
}

#define CatInfoIsFolder(cPB) (((cPB)->hFileInfo.ioFlAttrib & ioDirMask) != 0)

GLOBALFUNC blnr CatInfoOpenReadDF(CInfoPBRec *cPB,
	short *refnum)
{
	HParamBlockRec r;
	blnr IsOk = falseblnr;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = cPB->hFileInfo.ioNamePtr;
	r.ioParam.ioVRefNum = cPB->hFileInfo.ioVRefNum;
	r.ioParam.ioPermssn = (char)fsRdPerm;
	r.ioParam.ioMisc = 0; /* use volume buffer */
	r.fileParam.ioDirID = cPB->hFileInfo.ioFlParID;
	if (CheckSysErr(PBHOpenSync(&r))) {
		*refnum = r.ioParam.ioRefNum;
		IsOk = trueblnr;
	}
	return IsOk;
}

GLOBALFUNC blnr CatInfoOpenReadRF(CInfoPBRec *cPB,
	short *refnum)
{
	HParamBlockRec r;
	blnr IsOk = falseblnr;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = cPB->hFileInfo.ioNamePtr;
	r.ioParam.ioVRefNum = cPB->hFileInfo.ioVRefNum;
	r.ioParam.ioPermssn = (char)fsRdPerm;
	r.ioParam.ioMisc = 0; /* use volume buffer */
	r.fileParam.ioDirID = cPB->hFileInfo.ioFlParID;
	if (CheckSysErr(PBHOpenRFSync(&r))) {
		*refnum = r.ioParam.ioRefNum;
		IsOk = trueblnr;
	}
	return IsOk;
}

GLOBALFUNC OSErr MyFileGetCatInfo0(MyDir_R *d, StringPtr s,
	StringPtr NameBuffer, CInfoPBRec *cPB)
{
	cPB->hFileInfo.ioCompletion = NULL;
	cPB->hFileInfo.ioVRefNum = d->VRefNum;
	cPB->dirInfo.ioDrDirID = d->DirId;
	if (NULL == s) {
		if (NULL == NameBuffer) {
			/*
				then this field must already be
				set up by caller
			*/
		} else {
			cPB->hFileInfo.ioNamePtr = NameBuffer;
		}
		cPB->dirInfo.ioFDirIndex = (short)(- 1);
	} else {
		if (NULL == NameBuffer) {
			cPB->hFileInfo.ioNamePtr = s;
		} else {
			cPB->hFileInfo.ioNamePtr = NameBuffer;
			PStrCopy(NameBuffer, s);
		}
		if (0 == PStrLength(s)) {
			cPB->dirInfo.ioFDirIndex = (short)(- 1);
		} else {
			cPB->dirInfo.ioFDirIndex = 0;
		}
	}

	return PBGetCatInfoSync(cPB);
}

GLOBALFUNC blnr MyFileGetCatInfo(MyDir_R *d, StringPtr s,
	StringPtr NameBuffer, CInfoPBRec *cPB)
{
	return CheckSysErr(MyFileGetCatInfo0(d, s,
		NameBuffer, cPB));
}

GLOBALFUNC blnr MyFileExists(MyDir_R *d, StringPtr s,
	blnr *Exists)
{
	MyPStr NameBuffer;
	CInfoPBRec cPB;
	OSErr err;
	blnr IsOk = falseblnr;

	err = MyFileGetCatInfo0(d, s, NameBuffer, &cPB);
	if (noErr == err) {
		*Exists = trueblnr;
		IsOk = trueblnr;
	} else if (fnfErr == err) {
		*Exists = falseblnr;
		IsOk = trueblnr;
	}

	return IsOk;
}

GLOBALFUNC blnr MyCatInfoCopyInfo(CInfoPBRec *cPB, MyDir_R *dst_d, StringPtr dst_s)
{
	MyPStr NameBuffer;
	CInfoPBRec r;
	blnr IsOk = falseblnr;

	if (MyFileGetCatInfo(dst_d, dst_s, NameBuffer, &r)) {

		r.hFileInfo.ioFlFndrInfo.fdType = cPB->hFileInfo.ioFlFndrInfo.fdType;
		r.hFileInfo.ioFlFndrInfo.fdCreator = cPB->hFileInfo.ioFlFndrInfo.fdCreator;
			/* or frRect for folder */
		r.hFileInfo.ioFlFndrInfo.fdLocation = cPB->hFileInfo.ioFlFndrInfo.fdLocation;
			/* or frLocation for folder */
		r.hFileInfo.ioFlFndrInfo.fdFlags =
			cPB->hFileInfo.ioFlFndrInfo.fdFlags;
			/* (0x0100 & r.hFileInfo.ioFlFndrInfo.fdFlags)
			|
			(0xFEFF & cPB->hFileInfo.ioFlFndrInfo.fdFlags) */;
			/* or frFlags */

		r.hFileInfo.ioFlCrDat = cPB->hFileInfo.ioFlCrDat;
		r.hFileInfo.ioFlMdDat = cPB->hFileInfo.ioFlMdDat;
		r.hFileInfo.ioFlBkDat = cPB->hFileInfo.ioFlBkDat;
			/* or ioDrCrDat, ioDrMdDat, and ioDrBkDat */

		if (CatInfoIsFolder(cPB)) {
			r.dirInfo.ioDrFndrInfo.frScroll = cPB->dirInfo.ioDrFndrInfo.frScroll;
			r.dirInfo.ioDrUsrWds.frView = cPB->dirInfo.ioDrUsrWds.frView;
		}

		r.hFileInfo.ioDirID = r.hFileInfo.ioFlParID;
		if (CheckSysErr(PBSetCatInfoSync(&r))) {
			IsOk = trueblnr;
		}
	}

	return IsOk;
}

GLOBALFUNC blnr MyFileClearInitted(MyDir_R *dst_d, StringPtr dst_s)
{
	MyPStr NameBuffer;
	CInfoPBRec r;
	blnr IsOk = falseblnr;

	if (MyFileGetCatInfo(dst_d, dst_s, NameBuffer, &r)) {

		r.hFileInfo.ioFlFndrInfo.fdFlags &= 0xFEFF;
			/* or frFlags */

		r.hFileInfo.ioDirID = r.hFileInfo.ioFlParID;
		if (CheckSysErr(PBSetCatInfoSync(&r))) {
			IsOk = trueblnr;
		}
	}

	return IsOk;
}

GLOBALFUNC blnr MyFileCopyFolderInfo(MyDir_R *src_d, MyDir_R *dst_d)
{
	MyPStr NameBuffer;
	CInfoPBRec cPB;
	blnr IsOk = falseblnr;

	if (MyFileGetCatInfo(src_d, NULL, NameBuffer, &cPB))
	if (MyCatInfoCopyInfo(&cPB, dst_d, NULL))
	{
		IsOk = trueblnr;
	}

	return IsOk;
}

GLOBALPROC MyCatInfoGetMyDir(CInfoPBRec *cPB, MyDir_R *d)
/* assumes CatInfoIsFolder(cPB) */
{
	d->VRefNum = cPB->hFileInfo.ioVRefNum;
	d->DirId = cPB->dirInfo.ioDrDirID;
}

GLOBALFUNC blnr MyCatGetNextChild(CInfoPBRec *cPB,
	MyDir_R *old_d, int *index, blnr *FinishOk)
{
	OSErr err;
	blnr GotOne = falseblnr;
	blnr FinishOk0 = falseblnr;

	cPB->dirInfo.ioFDirIndex = (*index)++;
	cPB->dirInfo.ioDrDirID = old_d->DirId;
	err = PBGetCatInfoSync(cPB);
	if (noErr == err) {
		GotOne = trueblnr;
	} else if (fnfErr != err) {
		vCheckSysErr(err);
	} else {
		FinishOk0 = trueblnr;
	}

	*FinishOk = FinishOk0;
	return GotOne;
}

GLOBALFUNC OSErr MyGetNamedVolDir(StringPtr s, MyDir_R *d)
{
	OSErr err;
	ParamBlockRec r;
	MyPStr NameBuffer;

	PStrCopy(NameBuffer, s);
	PStrApndChar(NameBuffer, ':');

	r.volumeParam.ioCompletion = NULL;
	r.volumeParam.ioNamePtr = NameBuffer;
	r.volumeParam.ioVRefNum = 0;
	r.volumeParam.ioVolIndex = -1;
	err = PBGetVInfoSync(&r);
	if (noErr == err) {
		d->VRefNum = r.volumeParam.ioVRefNum;
		d->DirId = 2;
	}

	return err;
}

GLOBALFUNC OSErr MyFindParentDir(MyDir_R *src_d, MyDir_R *dst_d)
{
	OSErr err;
	MyPStr NameBuffer;
	CInfoPBRec cPB;

	err = MyFileGetCatInfo0(src_d, NULL, NameBuffer, &cPB);
	if (noErr == err) {
		if (! CatInfoIsFolder(&cPB)) {
			err = dirNFErr;
		} else {
			dst_d->VRefNum = cPB.hFileInfo.ioVRefNum;
			dst_d->DirId = cPB.hFileInfo.ioFlParID;
		}
	}

	return err;
}

GLOBALFUNC OSErr MyFindNamedChildDir0(MyDir_R *src_d, StringPtr s, MyDir_R *dst_d)
{
	OSErr err;
	MyPStr NameBuffer;
	CInfoPBRec cPB;

	err = MyFileGetCatInfo0(src_d, s, NameBuffer, &cPB);
	if (noErr == err) {
		if (! CatInfoIsFolder(&cPB)) {
			err = dirNFErr;
		} else {
			MyCatInfoGetMyDir(&cPB, dst_d);
		}
	}

	return err;
}

GLOBALFUNC blnr MyFindNamedChildDir(MyDir_R *src_d, StringPtr s, MyDir_R *dst_d)
{
	return CheckSysErr(MyFindNamedChildDir0(src_d, s, dst_d));
}

GLOBALFUNC OSErr MyResolveAliasDir0(MyDir_R *src_d, StringPtr s, MyDir_R *dst_d)
{
	OSErr err;
	FSSpec spec;
	Boolean isFolder;
	Boolean isAlias;
	MyDir_R src2_d;

	spec.vRefNum = src_d->VRefNum;
	spec.parID = src_d->DirId;
	PStrCopy(spec.name, s);
	err = ResolveAliasFile(&spec, true, &isFolder, &isAlias);
	if (noErr == err) {
		if (! isAlias) {
			err = dirNFErr;
		} else {
			src2_d.VRefNum = spec.vRefNum;
			src2_d.DirId = spec.parID;
			err = MyFindNamedChildDir0(&src2_d, spec.name, dst_d);
		}
	}

	return err;
}

GLOBALFUNC OSErr MyResolveNamedChildDir0(MyDir_R *src_d, StringPtr s, MyDir_R *dst_d)
{
	OSErr err;

	err = MyFindNamedChildDir0(src_d, s, dst_d);
	if (dirNFErr == err) {
		if (HaveAliasMgrAvail()) {
			err = MyResolveAliasDir0(src_d, s, dst_d);
		}
	}

	return err;
}

GLOBALFUNC OSErr MyResolveIfAlias(MyDir_R *d, StringPtr s)
{
	OSErr err;
	FSSpec spec;
	Boolean isFolder;
	Boolean isAlias;

	if ((! HaveAliasMgrAvail())
		|| (0 == PStrLength(s))) /* means the directory, which can't be alias anyway */
	{
		err = noErr;
	} else {
		spec.vRefNum = d->VRefNum;
		spec.parID = d->DirId;
		PStrCopy(spec.name, s);
		err = ResolveAliasFile(&spec, true, &isFolder, &isAlias);
		if (noErr == err) {
			if (isAlias) {
				d->VRefNum = spec.vRefNum;
				d->DirId = spec.parID;
				PStrCopy(s, spec.name);
			}
		}
	}

	return err;
}

LOCALFUNC OSErr MyDirFromWD0(short VRefNum, MyDir_R *d)
{
	OSErr err;
	Str63 s;
	WDPBRec pb;

#if Support64kROM
	if (Have64kROM()) {
		d->VRefNum = VRefNum;
		d->DirId = 0;
		err = noErr;
	} else
#endif
	{
		pb.ioCompletion = NULL;
		pb.ioNamePtr = s;
		pb.ioVRefNum = VRefNum;
		pb.ioWDIndex = 0;
		pb.ioWDProcID = 0;
		err = PBGetWDInfoSync(&pb);
		if (noErr == err) {
			d->VRefNum = pb.ioWDVRefNum;
			d->DirId = pb.ioWDDirID;
		}
	}

	return err;
}

LOCALFUNC blnr MyDirFromWD(short VRefNum, MyDir_R *d)
{
	return CheckSysErr(MyDirFromWD0(VRefNum, d));
}

#ifdef Have_MACINITS
LOCALFUNC blnr MyFilePutNew(StringPtr prompt, StringPtr origName,
		MyDir_R *d, StringPtr s)
{
	blnr IsOk = falseblnr;

	if (! HaveCustomPutFileAvail()) {
		SFReply reply;
		Point tempPt;

		tempPt.h = 100;
		tempPt.v = 100;
		SFPutFile(tempPt, prompt, origName, NULL, &reply);
		if (reply.good) {
			PStrCopy(s, reply.fName);
			IsOk = MyDirFromWD(reply.vRefNum, d);
		}
	} else {
		StandardFileReply reply;

		StandardPutFile(prompt, origName, &reply);
		if (reply.sfGood) {
			d->VRefNum = reply.sfFile.vRefNum;
			d->DirId = reply.sfFile.parID;
			PStrCopy(s, reply.sfFile.name);
			IsOk = trueblnr;
		}
	}

	return IsOk;
}
#endif

#ifdef Have_MACINITS
LOCALFUNC blnr MyFileGetOld(simr nInputTypes,
		ConstSFTypeListPtr pfInputType,
		MyDir_R *d, StringPtr s)
{
	blnr IsOk = falseblnr;

	if (! HaveCustomPutFileAvail()) {
		SFReply reply;
		Point tempPt;

		tempPt.h = 50;
		tempPt.v = 50;
		SFGetFile(tempPt, "\p", NULL, nInputTypes,
			(SFTypeList)pfInputType, NULL, &reply);
		if (reply.good) {
			PStrCopy(s, reply.fName);
			IsOk = MyDirFromWD(reply.vRefNum, d);
		}
	} else {
		StandardFileReply reply;

		StandardGetFile(NULL, nInputTypes, (SFTypeList)pfInputType, &reply);
		if (reply.sfGood) {
			d->VRefNum = reply.sfFile.vRefNum;
			d->DirId = reply.sfFile.parID;
			PStrCopy(s, reply.sfFile.name);
			IsOk = trueblnr;
		}
	}

	return IsOk;
}
#endif

#ifdef Have_MACINITS
LOCALFUNC blnr CreateOpenNewFile(StringPtr prompt, StringPtr origName, OSType creator, OSType fileType,
		MyDir_R *d, StringPtr s, short *refNum)
{
	blnr IsOk = falseblnr;

	if (MyFilePutNew(prompt, origName, d, s)) {
		IsOk = MyOpenNewFile(d, s, creator, fileType, refNum);
	}

	return IsOk;
}
#endif
