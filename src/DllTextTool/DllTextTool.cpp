#include "DllTextTool.h"

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

int ExportDllText(const UString& a_sDllFileName, const UString& a_sTxtFileName, bool a_bStandalone)
{
	FILE* fpDll = UFopen(a_sDllFileName.c_str(), USTR("rb"), false);
	if (fpDll == nullptr)
	{
		return 1;
	}
	fseek(fpDll, 0, SEEK_END);
	u32 uDllSize = ftell(fpDll);
	fseek(fpDll, 0, SEEK_SET);
	u8* pDll = new u8[uDllSize];
	fread(pDll, 1, uDllSize, fpDll);
	fclose(fpDll);
	PIMAGE_DOS_HEADER pImageDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pDll);
	if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete[] pDll;
		return 1;
	}
	PIMAGE_NT_HEADERS32 pImageNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS32>(pDll + pImageDosHeader->e_lfanew);
	if (pImageNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		delete[] pDll;
		return 1;
	}
	if ((pImageNTHeader->FileHeader.Machine & IMAGE_FILE_32BIT_MACHINE) == 0)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->FileHeader.SizeOfOptionalHeader == 0)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size == 0)
	{
		delete[] pDll;
		return 1;
	}
	PIMAGE_SECTION_HEADER pImageSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(pImageNTHeader + 1);
	for (n32 i = 0; i < pImageNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (strcmp(reinterpret_cast<char*>(pImageSectionHeader[i].Name), ".text") == 0)
		{
			u8* pTextSection = pDll + pImageSectionHeader[i].PointerToRawData;
			PIMAGE_COR20_HEADER pImageCor20Header = reinterpret_cast<PIMAGE_COR20_HEADER>(pTextSection + pImageNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress - pImageSectionHeader[i].VirtualAddress);
			if (pImageCor20Header->cb != sizeof(*pImageCor20Header))
			{
				delete[] pDll;
				return 1;
			}
			MetadataHeader* pMetadataHeader = reinterpret_cast<MetadataHeader*>(pTextSection + pImageCor20Header->MetaData.VirtualAddress - pImageSectionHeader[i].VirtualAddress);
			if (pMetadataHeader->Signature != 0x424A5342)
			{
				delete[] pDll;
				return 1;
			}
			StorageHeader* pStorageHeader = reinterpret_cast<StorageHeader*>(reinterpret_cast<u8*>(pMetadataHeader + 1) + pMetadataHeader->VersionStringLength);
			PIMAGE_DATA_DIRECTORY pImageDataDirectory = reinterpret_cast<PIMAGE_DATA_DIRECTORY>(pStorageHeader + 1);
			bool bFound = false;
			for (n32 j = 0; j < pStorageHeader->NumberofStreams; j++)
			{
				if (strcmp(reinterpret_cast<char*>(pImageDataDirectory) + 8, "#US") == 0)
				{
					bFound = true;
					break;
				}
				pImageDataDirectory = reinterpret_cast<PIMAGE_DATA_DIRECTORY>(reinterpret_cast<char*>(pImageDataDirectory) + 8 + (strlen(reinterpret_cast<char*>(pImageDataDirectory) + 8) + 4) / 4 * 4);
			}
			if (!bFound)
			{
				delete[] pDll;
				return 1;
			}
			vector<pair<u32, u32>> vRecord;
			FILE* fpTxt = nullptr;
			FILE* fpTxtASCII = nullptr;
			n32 nIndex = 0;
			u8* pStream = reinterpret_cast<u8*>(pMetadataHeader) + pImageDataDirectory->VirtualAddress;
			for (u8* pBytecode = reinterpret_cast<u8*>(pImageCor20Header + 1); pBytecode < reinterpret_cast<u8*>(pMetadataHeader) - 4; pBytecode++)
			{
				if (pBytecode[0] != 0x72)
				{
					continue;
				}
				if (pBytecode[4] != 0x70)
				{
					continue;
				}
				u32 uOffset = *reinterpret_cast<u32*>(pBytecode + 1) - 0x70000000;
				if (uOffset == 0 || uOffset >= pImageDataDirectory->Size)
				{
					continue;
				}
				if (pStream[uOffset - 1] != 0 && pStream[uOffset - 1] != 1)
				{
					continue;
				}
				u32 uRecordOffset = uOffset;
				u32 uByteSize = pStream[uOffset];
				if (uByteSize >= 0xC0)
				{
					uByteSize = (uByteSize << 24 | pStream[uOffset + 1] << 16 | pStream[uOffset + 2] << 8 | pStream[uOffset + 3]) - 0xC0000000;
					uOffset += 4;
				}
				else if (uByteSize >= 0x80)
				{
					uByteSize = (uByteSize << 8 | pStream[uOffset + 1]) - 0x8000;
					uOffset += 2;
				}
				else
				{
					uOffset++;
				}
				u32 uRecordSize = uOffset - uRecordOffset + uByteSize;
				vRecord.push_back(make_pair(uRecordOffset, uRecordSize));
				Char16_t* pText = reinterpret_cast<Char16_t*>(pStream + uOffset);
				u32 uCharCount = (uByteSize - 1) / 2;
				U16String sTxt16(pText, uCharCount);
				wstring sTxt = U16ToW(sTxt16);
				wstring::size_type uPos = 0;
				uPos = sTxt.find(L"[No].");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"[--------------------------------------]");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"[======================================]");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"<[b]>");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"<[v]>");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"<[f]>");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos = sTxt.find(L"<r>");
				if (uPos != wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				sTxt = Replace(sTxt, L"No.", L"[No].");
				sTxt = Replace(sTxt, L"--------------------------------------", L"[--------------------------------------]");
				sTxt = Replace(sTxt, L"======================================", L"[======================================]");
				sTxt = Replace(sTxt, L'\b', L"<[b]>");
				sTxt = Replace(sTxt, L'\v', L"<[v]>");
				sTxt = Replace(sTxt, L'\f', L"<[f]>");
				sTxt = Replace(sTxt, L'\r', L"<r>");
				sTxt = Replace(sTxt, L'\n', L"\r\n");
				bool bASCII = count_if(sTxt.begin(), sTxt.end(), isASCII) == sTxt.size();
				if (a_bStandalone || !bASCII)
				{
					if (fpTxt == nullptr)
					{
						fpTxt = UFopen(a_sTxtFileName.c_str(), USTR("wb"), false);
						if (fpTxt == nullptr)
						{
							if (fpTxtASCII != nullptr)
							{
								fclose(fpTxtASCII);
							}
							delete[] pDll;
							return 1;
						}
						fwrite("\xFF\xFE", 2, 1, fpTxt);
					}
					else
					{
						fu16printf(fpTxt, L"\r\n\r\n");
					}
					fu16printf(fpTxt, L"No.%d,%u\r\n", nIndex++, uCharCount);
					fu16printf(fpTxt, L"--------------------------------------\r\n");
					fu16printf(fpTxt, L"%ls\r\n", sTxt.c_str());
					fu16printf(fpTxt, L"======================================\r\n");
					fu16printf(fpTxt, L"%ls\r\n", sTxt.c_str());
					fu16printf(fpTxt, L"--------------------------------------\r\n");
				}
				else
				{
					if (fpTxtASCII == nullptr)
					{
						static UString c_sPath = a_sTxtFileName + USTR(".a.txt");
						fpTxtASCII = UFopen(c_sPath.c_str(), USTR("wb"), false);
						if (fpTxtASCII == nullptr)
						{
							if (fpTxt != nullptr)
							{
								fclose(fpTxt);
							}
							delete[] pDll;
							return 1;
						}
						fwrite("\xFF\xFE", 2, 1, fpTxtASCII);
					}
					else
					{
						fu16printf(fpTxtASCII, L"\r\n\r\n");
					}
					fu16printf(fpTxtASCII, L"No.%d,%u\r\n", nIndex++, uCharCount);
					fu16printf(fpTxtASCII, L"--------------------------------------\r\n");
					fu16printf(fpTxtASCII, L"%ls\r\n", sTxt.c_str());
					fu16printf(fpTxtASCII, L"======================================\r\n");
					fu16printf(fpTxtASCII, L"%ls\r\n", sTxt.c_str());
					fu16printf(fpTxtASCII, L"--------------------------------------\r\n");
				}
			}
			if (fpTxtASCII != nullptr)
			{
				fclose(fpTxtASCII);
			}
			if (fpTxt != nullptr)
			{
				fclose(fpTxt);
			}
			for (vector<pair<u32, u32>>::iterator it = vRecord.begin(); it != vRecord.end(); ++it)
			{
				pair<u32, u32>& offsetSize = *it;
				memset(pStream + offsetSize.first, 0, offsetSize.second);
			}
			for (u32 j = 0; j < pImageDataDirectory->Size; j++)
			{
				if (pStream[j] != 0)
				{
					UPrintf(USTR("0x%08X not load\n"), static_cast<u32>(pStream + j - pDll));
					delete[] pDll;
					return 1;
				}
			}
			break;
		}
	}
	delete[] pDll;
	return 0;
}

int ImportDllText(const UString& a_sDllFileName, const UString& a_sTxtFileName, bool a_bStandalone)
{
	FILE* fp = UFopen(a_sDllFileName.c_str(), USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	u32 uDllSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	u8* pDll = new u8[uDllSize];
	fread(pDll, 1, uDllSize, fp);
	fclose(fp);
	PIMAGE_DOS_HEADER pImageDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pDll);
	if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete[] pDll;
		return 1;
	}
	PIMAGE_NT_HEADERS32 pImageNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS32>(pDll + pImageDosHeader->e_lfanew);
	if (pImageNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		delete[] pDll;
		return 1;
	}
	if ((pImageNTHeader->FileHeader.Machine & IMAGE_FILE_32BIT_MACHINE) == 0)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->FileHeader.SizeOfOptionalHeader == 0)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
	{
		delete[] pDll;
		return 1;
	}
	if (pImageNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size == 0)
	{
		delete[] pDll;
		return 1;
	}
	PIMAGE_SECTION_HEADER pImageSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(pImageNTHeader + 1);
	wstring sTxt;
	{
		fp = UFopen(a_sTxtFileName.c_str(), USTR("rb"), false);
		if (fp != nullptr)
		{
			fseek(fp, 0, SEEK_END);
			u32 uTxtSize = ftell(fp);
			if (uTxtSize % 2 != 0)
			{
				fclose(fp);
				delete[] pDll;
				return 1;
			}
			uTxtSize /= 2;
			fseek(fp, 0, SEEK_SET);
			Char16_t* pTemp = new Char16_t[uTxtSize + 1];
			fread(pTemp, 2, uTxtSize, fp);
			fclose(fp);
			if (pTemp[0] != 0xFEFF)
			{
				delete[] pTemp;
				delete[] pDll;
				return 1;
			}
			pTemp[uTxtSize] = L'\0';
			sTxt = U16ToW(pTemp + 1);
			delete[] pTemp;
		}
		if (!a_bStandalone)
		{
			static UString c_sPath = a_sTxtFileName + USTR(".a.txt");
			fp = UFopen(c_sPath.c_str(), USTR("rb"), false);
			if (fp != nullptr)
			{
				fseek(fp, 0, SEEK_END);
				u32 uTxtSize = ftell(fp);
				if (uTxtSize % 2 != 0)
				{
					fclose(fp);
					delete[] pDll;
					return 1;
				}
				uTxtSize /= 2;
				fseek(fp, 0, SEEK_SET);
				Char16_t* pTemp = new Char16_t[uTxtSize + 1];
				fread(pTemp, 2, uTxtSize, fp);
				fclose(fp);
				if (pTemp[0] != 0xFEFF)
				{
					delete[] pTemp;
					delete[] pDll;
					return 1;
				}
				pTemp[uTxtSize] = L'\0';
				sTxt += L"\r\n";
				sTxt += U16ToW(pTemp + 1);
				delete[] pTemp;
			}
		}
	}
	for (n32 i = 0; i < pImageNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (strcmp(reinterpret_cast<char*>(pImageSectionHeader[i].Name), ".text") == 0)
		{
			u8* pTextSection = pDll + pImageSectionHeader[i].PointerToRawData;
			PIMAGE_COR20_HEADER pImageCor20Header = reinterpret_cast<PIMAGE_COR20_HEADER>(pTextSection + pImageNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress - pImageSectionHeader[i].VirtualAddress);
			if (pImageCor20Header->cb != sizeof(*pImageCor20Header))
			{
				delete[] pDll;
				return 1;
			}
			MetadataHeader* pMetadataHeader = reinterpret_cast<MetadataHeader*>(pTextSection + pImageCor20Header->MetaData.VirtualAddress - pImageSectionHeader[i].VirtualAddress);
			if (pMetadataHeader->Signature != 0x424A5342)
			{
				delete[] pDll;
				return 1;
			}
			StorageHeader* pStorageHeader = reinterpret_cast<StorageHeader*>(reinterpret_cast<u8*>(pMetadataHeader + 1) + pMetadataHeader->VersionStringLength);
			PIMAGE_DATA_DIRECTORY pImageDataDirectory = reinterpret_cast<PIMAGE_DATA_DIRECTORY>(pStorageHeader + 1);
			bool bFound = false;
			for (n32 j = 0; j < pStorageHeader->NumberofStreams; j++)
			{
				if (strcmp(reinterpret_cast<char*>(pImageDataDirectory) + 8, "#US") == 0)
				{
					bFound = true;
					break;
				}
				pImageDataDirectory = reinterpret_cast<PIMAGE_DATA_DIRECTORY>(reinterpret_cast<char*>(pImageDataDirectory) + 8 + (strlen(reinterpret_cast<char*>(pImageDataDirectory) + 8) + 4) / 4 * 4);
			}
			if (!bFound)
			{
				delete[] pDll;
				return 1;
			}
			n32 nIndex = 0;
			u8* pStream = reinterpret_cast<u8*>(pMetadataHeader) + pImageDataDirectory->VirtualAddress;
			for (u8* pBytecode = reinterpret_cast<u8*>(pImageCor20Header + 1); pBytecode < reinterpret_cast<u8*>(pMetadataHeader) - 4; pBytecode++)
			{
				if (pBytecode[0] != 0x72)
				{
					continue;
				}
				if (pBytecode[4] != 0x70)
				{
					continue;
				}
				u32 uOffset = *reinterpret_cast<u32*>(pBytecode + 1) - 0x70000000;
				if (uOffset == 0 || uOffset >= pImageDataDirectory->Size)
				{
					continue;
				}
				if (pStream[uOffset - 1] != 0 && pStream[uOffset - 1] != 1)
				{
					continue;
				}
				wstring sNum = Format(L"No.%d,", nIndex++);
				wstring::size_type uPos0 = sTxt.find(sNum);
				if (uPos0 == wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos0 += sNum.size();
				u32 uCharCountMax = SToU32(sTxt.c_str() + uPos0);
				uPos0 = sTxt.find(L"\r\n======================================\r\n", uPos0);
				if (uPos0 == wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				uPos0 += wcslen(L"\r\n======================================\r\n");
				wstring::size_type uPos1 = sTxt.find(L"\r\n--------------------------------------", uPos0);
				if (uPos1 == wstring::npos)
				{
					delete[] pDll;
					return 1;
				}
				wstring sStmt = sTxt.substr(uPos0, uPos1 - uPos0);
				sStmt = Replace(sStmt, L"\r\n", L"\n");
				sStmt = Replace(sStmt, L"<r>", L"\r");
				sStmt = Replace(sStmt, L"<[f]>", L"\f");
				sStmt = Replace(sStmt, L"<[v]>", L"\v");
				sStmt = Replace(sStmt, L"<[b]>", L"\b");
				sStmt = Replace(sStmt, L"[======================================]", L"======================================");
				sStmt = Replace(sStmt, L"[--------------------------------------]", L"--------------------------------------");
				sStmt = Replace(sStmt, L"[No].", L"No.");
				U16String sStmt16 = WToU16(sStmt);
				u32 uCharCount = static_cast<u32>(sStmt16.size());
				if (uCharCount > uCharCountMax)
				{
					UPrintf(USTR("%") PRIUS USTR("%u %u > %u\n"), WToU(sNum).c_str(), uCharCountMax, uCharCount, uCharCountMax);
					delete[] pDll;
					return 1;
				}
				static u8 c_uStringSize[4] = {};
				u32 uByteSize = static_cast<u32>(sStmt16.size()) * 2 + 1;
				u32 uRecordSize = uByteSize;
				if (uByteSize < 0x80)
				{
					uRecordSize++;
					c_uStringSize[0] = uByteSize;
				}
				else if (uByteSize < 0x4000)
				{
					uRecordSize += 2;
					c_uStringSize[0] = (uByteSize + 0x8000) >> 8 & 0xFF;
					c_uStringSize[1] = uByteSize & 0xFF;
				}
				else
				{
					uRecordSize += 4;
					c_uStringSize[0] = (uByteSize + 0xC0000000) >> 24 & 0xFF;
					c_uStringSize[1] = uByteSize >> 16 & 0xFF;
					c_uStringSize[2] = uByteSize >> 8 & 0xFF;
					c_uStringSize[3] = uByteSize & 0xFF;
				}
				memcpy(pStream + uOffset, c_uStringSize, uRecordSize - uByteSize);
				memcpy(pStream + uOffset + uRecordSize - uByteSize, sStmt.c_str(), sStmt.size() * 2);
			}
			break;
		}
	}
	fp = UFopen(a_sDllFileName.c_str(), USTR("wb"), false);
	if (fp == nullptr)
	{
		delete[] pDll;
		return 1;
	}
	fwrite(pDll, 1, uDllSize, fp);
	fclose(fp);
	delete[] pDll;
	return 0;
}

int UMain(int argc, UChar* argv[])
{
	if (argc < 4)
	{
		return 1;
	}
	bool bStandalone = argc > 4 && UCscmp(argv[4], USTR("0")) != 0;
	if (UCslen(argv[1]) == 1)
	{
		switch (*argv[1])
		{
		case USTR('E'):
		case USTR('e'):
			return ExportDllText(argv[2], argv[3], bStandalone);
		case USTR('I'):
		case USTR('i'):
			return ImportDllText(argv[2], argv[3], bStandalone);
		default:
			break;
		}
	}
	return 1;
}
