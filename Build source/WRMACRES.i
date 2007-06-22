/*
	WRMACRES.i
	Copyright (C) 2007 Paul Pratt

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
	WRite MACintosh RESources configuration
*/


LOCALPROC WriteBeginResResource(char *types, int id)
{
	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("resource '");
	WriteCStrToDestFile(types);
	WriteSingleQuoteToDestFile();
	WriteCStrToDestFile(" (");
	WriteUnsignedToOutput(id);
	WriteCStrToDestFile(") {");
	WriteEndDestFileLn();
	++DestFileIndent;
}

LOCALPROC WriteEndResResource(void)
{
	--DestFileIndent;
	WriteDestFileLn("};");
}

LOCALPROC WriteQuotedInclude(char *name)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("#include ");
	WriteQuoteToDestFile();
	WriteCStrToDestFile(name);
	WriteQuoteToDestFile();
	WriteEndDestFileLn();
}

LOCALPROC WriteResTypeComma(char *types)
{
	WriteBgnDestFileLn();
	WriteSingleQuoteToDestFile();
	WriteCStrToDestFile(types);
	WriteSingleQuoteToDestFile();
	WriteCStrToDestFile(",");
	WriteEndDestFileLn();
}

LOCALPROC WriteAFREFres(char *types, int i)
{
	WriteBeginResResource("FREF", 128 + i);
		WriteResTypeComma(types);

		WriteBgnDestFileLn();
		WriteUnsignedToOutput(i);
		WriteCStrToDestFile(",");
		WriteEndDestFileLn();

		WriteBgnDestFileLn();
		WriteQuoteToDestFile();
		WriteQuoteToDestFile();
		WriteEndDestFileLn();
	WriteEndResResource();
}

LOCALPROC WriteDocTypeDefIconId(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("#define ");
	WriteDocTypeIconShortName();
	WriteCStrToDestFile("IconId ");
	WriteUnsignedToOutput(128 + DocTypeCounter);
	WriteEndDestFileLn();
}

LOCALPROC WriteDocTypeFREFres(void)
{
	WriteAFREFres(DoDocType_gd()->MacType, DocTypeCounter);
}

LOCALPROC WriteDocTypeBNDLFREF(void)
{
	WriteBgnDestFileLn();
	if (0 != DocTypeCounter) {
		WriteCStrToDestFile(",");
	}
	WriteUnsignedToOutput(DocTypeCounter);
	WriteCStrToDestFile(", ");
	WriteUnsignedToOutput(128 + DocTypeCounter);
	WriteEndDestFileLn();
}

LOCALPROC WriteDocTypeBNDLICN(void)
{
	WriteBgnDestFileLn();
	if (0 != DocTypeCounter) {
		WriteCStrToDestFile(",");
	}
	WriteUnsignedToOutput(DocTypeCounter);
	WriteCStrToDestFile(", ");
	WriteDocTypeIconShortName();
	WriteCStrToDestFile("IconId");
	WriteEndDestFileLn();
}

LOCALPROC WriteDocTypeIncludeIconFile(void)
{
	WriteBgnDestFileLn();
	WriteCStrToDestFile("#include ");
	WriteQuoteToDestFile();
	WriteDocTypeIconShortName();
	WriteCStrToDestFile("Icon.r");
	WriteQuoteToDestFile();
	WriteEndDestFileLn();
}

LOCALPROC WriteCommonCNFGRSRC(void)
{
	if (WriteOpenDestFile(&SrcDirR, "CNFGRSRC", ".h")) { /* Resource Configuration file */

	WriteBlankLineToDestFile();
#if support_ide_mw8
	if (cur_ide == gbk_ide_mw8) {
		if ((cur_targ == gbk_targ_mach) || (cur_targ == gbk_targ_imch)) {
			WriteDestFileLn("#include <Carbon/Carbon.r>");
		} else {
			WriteQuotedInclude("Types.r");
			WriteQuotedInclude("Icons.r");
		}
	} else
#endif
	if ((cur_ide == gbk_ide_bgc) || (cur_ide == gbk_ide_xcd)) {
		WriteQuotedInclude("Carbon.r");
	} else if (cur_ide == gbk_ide_mpw) {
		WriteQuotedInclude("Types.r");
		WriteQuotedInclude("Icons.r");
	}

	WriteBlankLineToDestFile();
	WriteBgnDestFileLn();
	WriteCStrToDestFile("#define kStrAppName ");
	WriteQuoteToDestFile();
	WriteCStrToDestFile(kStrAppName);
	WriteQuoteToDestFile();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("#define kAppVariationStr ");
	WriteQuoteToDestFile();
	WriteAppVariationStr();
	WriteQuoteToDestFile();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("#define kStrCopyrightYear ");
	WriteQuoteToDestFile();
	WriteCStrToDestFile(kStrCopyrightYear);
	WriteQuoteToDestFile();
	WriteEndDestFileLn();

	WriteBgnDestFileLn();
	WriteCStrToDestFile("#define kStrHomePage ");
	WriteQuoteToDestFile();
	WriteHomePage();
	WriteQuoteToDestFile();
	WriteEndDestFileLn();

	if (! ((cur_targ == gbk_targ_mach) || (cur_targ == gbk_targ_imch))) {
		if (cur_targ == gbk_targ_carb) {
			WriteBlankLineToDestFile();
			WriteDestFileLn("data 'plst' (0) {");
			++DestFileIndent;
				WriteDestFileLn("$\"00\"");
			--DestFileIndent;
			WriteDestFileLn("};");
		}

		WriteBlankLineToDestFile();
		WriteBgnDestFileLn();
		WriteCStrToDestFile("data '");
		WriteCStrToDestFile(kMacCreatorSig);
		WriteCStrToDestFile("' (0, \"Owner resource\") {");
		WriteEndDestFileLn();
		++DestFileIndent;
			WriteDestFileLn("$\"00\"");
		--DestFileIndent;
		WriteDestFileLn("};");

		WriteBeginResResource("vers", 1);
			WriteBgnDestFileLn();
			WriteCStrToDestFile(kMajorVersion);
			WriteCStrToDestFile(",");
			WriteEndDestFileLn();

			WriteBgnDestFileLn();
			WriteCStrToDestFile(kMinorVersion);
			WriteCStrToDestFile(" * 16 + ");
			WriteCStrToDestFile(kMinorSubVersion);
			WriteCStrToDestFile(",");
			WriteEndDestFileLn();

			WriteDestFileLn("release,");
			WriteDestFileLn("0x0,");
			WriteDestFileLn("0,");

			WriteBgnDestFileLn();
			WriteQuoteToDestFile();
			WriteVersionStr();
			WriteQuoteToDestFile();
			WriteCStrToDestFile(",");
			WriteEndDestFileLn();

			WriteBgnDestFileLn();
			WriteQuoteToDestFile();
			WriteGetInfoString();
			WriteQuoteToDestFile();
			WriteEndDestFileLn();
		WriteEndResResource();

#if 0
		WriteBeginResResource("vers", 2);
			WriteDestFileLn("0x0,");
			WriteDestFileLn("0x0,");
			WriteDestFileLn("release,");
			WriteDestFileLn("0x0,");
			WriteDestFileLn("0,");
			WriteBgnDestFileLn();
			WriteQuoteToDestFile();
			WriteQuoteToDestFile();
			WriteCStrToDestFile(",");
			WriteEndDestFileLn();
			WriteBgnDestFileLn();
			WriteQuoteToDestFile();
			WriteCStrToDestFile(kShortDescription);
			WriteQuoteToDestFile();
			WriteEndDestFileLn();
		WriteEndResResource();
#endif

		if (cur_ide != gbk_ide_mw8) {
			WriteBlankLineToDestFile();
			WriteDestFileLn("resource 'SIZE' (-1) {");
			++DestFileIndent;
				WriteDestFileLn("reserved,");
				WriteDestFileLn("acceptSuspendResumeEvents,");
				WriteDestFileLn("reserved,");
				WriteDestFileLn("canBackground,");
				WriteDestFileLn("multiFinderAware,");
				WriteDestFileLn("backgroundAndForeground,");
				WriteDestFileLn("dontGetFrontClicks,");
				WriteDestFileLn("ignoreChildDiedEvents,");
				WriteDestFileLn("is32BitCompatible,");

				/* following 4 should be "reserved" if api not available */
				WriteDestFileLn("isHighLevelEventAware,");
				WriteDestFileLn("localAndRemoteHLEvents,");
				WriteDestFileLn("isStationeryAware,");
				WriteDestFileLn("useTextEditServices,");

				WriteDestFileLn("reserved,");
				WriteDestFileLn("reserved,");
				WriteDestFileLn("reserved,");
				WriteDestFileLn("5242880,");
				WriteDestFileLn("1572864");
			WriteEndResResource();
		}

		WriteBlankLineToDestFile();
		DoAllDocTypesWithSetup(WriteDocTypeDefIconId);


		DoAllDocTypesWithSetup(WriteDocTypeFREFres);
		WriteAFREFres("****", DocTypeCounter);

		WriteBeginResResource("BNDL", 128);
			WriteResTypeComma(kMacCreatorSig);

			WriteDestFileLn("0,");
			WriteDestFileLn("{");
			++DestFileIndent;
				WriteResTypeComma("FREF");

				WriteDestFileLn("{");
				++DestFileIndent;
					DoAllDocTypesWithSetup(WriteDocTypeBNDLFREF);
					WriteDocTypeBNDLFREF();
				--DestFileIndent;
				WriteDestFileLn("},");

				WriteResTypeComma("ICN#");

				WriteDestFileLn("{");
				++DestFileIndent;
					DoAllDocTypesWithSetup(WriteDocTypeBNDLICN);
					WriteBgnDestFileLn();
					WriteCStrToDestFile(",");
					WriteUnsignedToOutput(DocTypeCounter);
					WriteCStrToDestFile(", 0");
					WriteEndDestFileLn();
				--DestFileIndent;
				WriteDestFileLn("}");
			--DestFileIndent;
			WriteDestFileLn("}");
		WriteEndResResource();

		WriteBlankLineToDestFile();
		WriteDestFileLn("#define SmallIconAPIAvail 1");
		WriteDestFileLn("#define ColorIconAPIAvail 1");

		WriteBlankLineToDestFile();
		DoAllDocTypesWithSetup(WriteDocTypeIncludeIconFile);
	}

	WriteCloseDestFile();
	}
}
