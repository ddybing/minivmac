/*
	WRLCCFLS.i
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
	WRite LCC-win32 specific FiLes
*/

#pragma segment LccW32Support

LOCALPROC DoSrcFileLccAddFile(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("File");
	WriteUnsignedToOutput(FileCounter + 1);
	WriteCStrToDestFile("=");
	WriteSrcFileFilePath();
	WriteEndDestFileLn();
}

LOCALPROC WriteLccErrFileName(void)
{
	WriteCStrToDestFile(kStrAppAbbrev);
	WriteCStrToDestFile(".err");
}

LOCALPROC WriteLccErrFilePath(void)
{
	WriteFileInDirToDestFile0(Write_obj_d_ToDestFile, WriteLccErrFileName);
}

static void WriteLccW32SpecificFiles(void)
{
	if (WriteOpenDestFile(&OutputDirR, kStrAppAbbrev, ".prj")) { /* workspace file */

	WriteDestFileLn("; Wedit project file. Syntax: Name = value");

	WriteBgnDestFileLn();
	WriteCStrToDestFile("[");
	WriteAppVariationStr();
	WriteCStrToDestFile("]");
	WriteEndDestFileLn();

	DoAllSrcFilesWithSetup(NullProc);
	++FileCounter; /* main.rc */

	WriteBgnDestFileLn();
	WriteCStrToDestFile("PrjFiles=");
	WriteUnsignedToOutput(FileCounter);
	WriteEndDestFileLn();

	DoAllSrcFilesWithSetup(DoSrcFileLccAddFile);

	WriteBgnDestFileLn();
	WriteCStrToDestFile("File");
	WriteUnsignedToOutput(++FileCounter);
	WriteCStrToDestFile("=");
	WriteMainRsrcSrcPath();
	WriteEndDestFileLn();

	WriteDestFileLn("ProjectFlags=0");

	WriteBgnDestFileLn();
	WriteCStrToDestFile("Name=");
	WriteAppVariationStr();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("ProjectPath=");
	WriteCStrToDestFile("c:\\output");
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("SourcesDir=");
	/* setting it to my_c_src_d does not work */
	WriteCStrToDestFile("c:\\output");
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("MakeDir=");
	Write_obj_d_ToDestFile();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("Exe=");
	WriteAppNamePath();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("DbgExeName=");
	WriteAppNamePath();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("DbgDir=");
	Write_obj_d_ToDestFile();
	WriteEndDestFileLn();

	switch (gbo_dbg) {
		case gbk_dbg_on:
			WriteDestFileLn("CompilerFlags=6728");
			break;
		case gbk_dbg_test:
			WriteDestFileLn("CompilerFlags=580");
			break;
		case gbk_dbg_off:
			WriteDestFileLn("CompilerFlags=581");
			break;
	}

	WriteBgnDestFileLn();
	WriteCStrToDestFile("Libraries=");
	WriteCStrToDestFile("shell32.lib ole32.lib uuid.lib winmm.lib");
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("ErrorFile=");
	WriteLccErrFilePath();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("CurrentFile=");
	WriteCNFGGLOBPath();
	WriteEndDestFileLn();

	WriteDestFileLn("OpenFiles=1");

	WriteBgnDestFileLn();
	WriteCStrToDestFile("OpenFile1=");
	WriteQuoteToDestFile();
	WriteCNFGGLOBPath();
	WriteQuoteToDestFile();
	WriteCStrToDestFile(" 1 29 14 532 435");
	WriteEndDestFileLn();

	WriteCloseDestFile();
	}
}


LOCALPROC DoSrcFileWriteLccW32clAddObjFile(void)
{
	WriteBgnDestFileLn();
	WriteSrcFileObjPath();
	WriteCStrToDestFile(" \\");
	WriteEndDestFileLn();
}

LOCALPROC DoSrcFileLccW32clEraseFile(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("del ");
	WriteSrcFileObjPath();
	WriteCStrToDestFile("");
	WriteEndDestFileLn();
}

LOCALPROC WriteMainRsrcObjLccbuild(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("lrc.exe -fo");
	WriteMainRsrcObjPath();
	WriteCStrToDestFile(" ");
	WriteMainRsrcSrcPath();
	WriteEndDestFileLn();
}

LOCALPROC WriteLccW32clSpecificFiles(void)
{
	if (WriteOpenDestFile(&OutputDirR, "Makefile", "")) { /* Make file */

	WriteBgnDestFileLn();
	WriteCStrToDestFile("# Wedit Makefile");
	WriteEndDestFileLn();

	WriteBlankLineToDestFile();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("mk_COptions= -c");
	if (gbo_dbg != gbk_dbg_on) {
		WriteCStrToDestFile(" -O");
	} else {
		WriteCStrToDestFile(" -g4");
	}
	WriteCStrToDestFile(" -A");
	WriteEndDestFileLn();

	WriteBlankLineToDestFile();
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("TheDefaultOutput:");
	if (CurPackageOut) {
		WriteMakeDependFile(WriteAppBinZipPath);
	} else {
		WriteMakeDependFile(WriteAppNamePath);
	}
	WriteEndDestFileLn();
	WriteBlankLineToDestFile();
	WriteBlankLineToDestFile();
	DoAllSrcFilesWithSetup(DoSrcFileMakeCompile);
	WriteBlankLineToDestFile();
	WriteDestFileLn("ObjFiles=\\");
	++DestFileIndent;
		DoAllSrcFilesWithSetup(DoSrcFileWriteLccW32clAddObjFile);
	--DestFileIndent;
	WriteBlankLineToDestFile();
	WriteBlankLineToDestFile();
	WriteBlankLineToDestFile();
	WriteMakeRule(WriteMainRsrcObjPath,
		WriteMainRsrcObjMSCdeps, WriteMainRsrcObjLccbuild);
	WriteBlankLineToDestFile();
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteAppNamePath();
	WriteCStrToDestFile(": $(ObjFiles) ");
	WriteMainRsrcObjPath();
	WriteEndDestFileLn();
	++DestFileIndent;
		WriteBgnDestFileLn();
		WriteCStrToDestFile("lcclnk.exe");
		if (gbo_dbg == gbk_dbg_off) {
			WriteCStrToDestFile(" -s");
		}
		WriteCStrToDestFile(" -subsystem windows -o ");
		WriteAppNamePath();
		WriteCStrToDestFile(" $(ObjFiles) ");
		WriteMainRsrcObjPath();
		WriteCStrToDestFile(" \\");
		WriteEndDestFileLn();
		++DestFileIndent;
			WriteDestFileLn("shell32.lib winmm.lib ole32.lib uuid.lib");
		--DestFileIndent;
	--DestFileIndent;
	WriteBlankLineToDestFile();

	WriteBlankLineToDestFile();
	WriteDestFileLn("clean:");
	++DestFileIndent;
		DoAllSrcFilesWithSetup(DoSrcFileLccW32clEraseFile);
		WriteRmFile(WriteMainRsrcObjPath);
		WriteRmFile(WriteAppNamePath);
		if (CurPackageOut) {
			WriteRmFile(WriteAppBinZipPath);
			WriteRmFile(WriteCheckSumFilePath);
		}
	--DestFileIndent;

	WriteCloseDestFile();
	}
}
