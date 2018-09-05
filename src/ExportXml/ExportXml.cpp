#include <sdw.h>
#include <tinyxml2.h>

int UMain(int argc, UChar* argv[])
{
	if (argc != 3)
	{
		return 1;
	}
	FILE* fp = UFopen(argv[1], USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	tinyxml2::XMLDocument doc;
	doc.LoadFile(fp);
	fclose(fp);
	unordered_map<string, vector<string>> mConfig;
	mConfig["mst_bgm_data"].push_back("mst_bgm");
	mConfig["mst_bgm_data"].push_back("bgm_record");
	mConfig["mst_bgm_jukebox_data"].push_back("mst_bgm_jukebox");
	mConfig["mst_bgm_jukebox_data"].push_back("Jukebox_record");
	mConfig["mst_furniture_data"].push_back("mst_furniture");
	mConfig["mst_furniture_data"].push_back("Title");
	mConfig["mst_furnituretext_data"].push_back("mst_furnituretext");
	mConfig["mst_furnituretext_data"].push_back("Description");
	mConfig["mst_maparea_data"].push_back("mst_maparea");
	mConfig["mst_maparea_data"].push_back("Name");
	mConfig["mst_mapenemy2_data"].push_back("mst_mapenemy2");
	mConfig["mst_mapenemy2_data"].push_back("Deck_name");
	mConfig["mst_mapinfo_data"].push_back("mst_mapinfo");
	mConfig["mst_mapinfo_data"].push_back("Name");
	mConfig["mst_mapinfo_data"].push_back("Opetext");
	mConfig["mst_mapinfo_data"].push_back("Infotext");
	mConfig["mst_mission2_data"].push_back("mst_mission2");
	mConfig["mst_mission2_data"].push_back("Name");
	mConfig["mst_mission2_data"].push_back("Details");
	mConfig["mst_payitem_data"].push_back("mst_payitem");
	mConfig["mst_payitem_data"].push_back("Name");
	mConfig["mst_payitemtext_data"].push_back("mst_payitemtext");
	mConfig["mst_payitemtext_data"].push_back("Description");
	mConfig["mst_quest_data"].push_back("mst_quest");
	mConfig["mst_quest_data"].push_back("Name");
	mConfig["mst_quest_data"].push_back("Details");
	mConfig["mst_ship_data"].push_back("mst_ship");
	mConfig["mst_ship_data"].push_back("Name");
	mConfig["mst_ship_data"].push_back("Yomi");
	mConfig["mst_ship_class_data"].push_back("mst_ship_class");
	mConfig["mst_ship_class_data"].push_back("Name");
	mConfig["mst_shiptext_data"].push_back("mst_shiptext");
	mConfig["mst_shiptext_data"].push_back("Getmes");
	mConfig["mst_shiptext_data"].push_back("Sinfo");
	mConfig["mst_slotitem_data"].push_back("mst_slotitem");
	mConfig["mst_slotitem_data"].push_back("Name");
	mConfig["mst_slotitem_equiptype_data"].push_back("mst_slotitem_equiptype");
	mConfig["mst_slotitem_equiptype_data"].push_back("Name");
	mConfig["mst_slotitem_remodel_data"].push_back("mst_slotitem_remodel");
	mConfig["mst_slotitem_remodel_data"].push_back("RemodelData");
	mConfig["mst_slotitemtext_data"].push_back("mst_slotitemtext");
	mConfig["mst_slotitemtext_data"].push_back("Info");
	mConfig["mst_stype_data"].push_back("mst_stype");
	mConfig["mst_stype_data"].push_back("Name");
	mConfig["mst_useitem_data"].push_back("mst_useitem");
	mConfig["mst_useitem_data"].push_back("Name");
	mConfig["mst_useitemtext_data"].push_back("mst_useitemtext");
	mConfig["mst_useitemtext_data"].push_back("Description");
	const tinyxml2::XMLElement* pRoot = doc.RootElement();
	unordered_map<string, vector<string>>::iterator it = mConfig.find(pRoot->Value());
	if (it != mConfig.end())
	{
		fp = UFopen(argv[2], USTR("wb"), false);
		if (fp == nullptr)
		{
			return 1;
		}
		fwrite("\xFF\xFE", 2, 1, fp);
		n32 nIndex = 0;
		vector<string>& vConfig = it->second;
		if (vConfig.size() < 2)
		{
			fclose(fp);
			return 1;
		}
		for (const tinyxml2::XMLElement* pElement = pRoot->FirstChildElement(vConfig[0].c_str()); pElement != nullptr; pElement = pElement->NextSiblingElement(vConfig[0].c_str()), nIndex++)
		{
			for (n32 i = 1; i < static_cast<n32>(vConfig.size()); i++)
			{
				const tinyxml2::XMLElement* pElementText = pElement->FirstChildElement(vConfig[i].c_str());
				if (pElementText == nullptr)
				{
					continue;
				}
				if (pElementText->FirstChild() == nullptr)
				{
					continue;
				}
				wstring sTxt = U8ToW(pElementText->FirstChild()->ToText()->Value());
				wstring::size_type uPos = 0;
				uPos = sTxt.find(L"[No].");
				if (uPos != wstring::npos)
				{
					fclose(fp);
					return 1;
				}
				uPos = sTxt.find(L"[--------------------------------------]");
				if (uPos != wstring::npos)
				{
					fclose(fp);
					return 1;
				}
				uPos = sTxt.find(L"[======================================]");
				if (uPos != wstring::npos)
				{
					fclose(fp);
					return 1;
				}
				uPos = sTxt.find_first_of(L"\r\n");
				if (uPos != wstring::npos)
				{
					fclose(fp);
					return 1;
				}
				sTxt = Replace(sTxt, L"No.", L"[No].");
				sTxt = Replace(sTxt, L"--------------------------------------", L"[--------------------------------------]");
				sTxt = Replace(sTxt, L"======================================", L"[======================================]");
				sTxt = Replace(sTxt, L"\\n", L"\r\n");
				if (ftell(fp) != 2)
				{
					fu16printf(fp, L"\r\n\r\n");
				}
				fu16printf(fp, L"No.%d\t%ls\r\n", nIndex * (vConfig.size() - 1) + i - 1, U8ToW(vConfig[i]).c_str());
				fu16printf(fp, L"--------------------------------------\r\n");
				fu16printf(fp, L"%ls\r\n", sTxt.c_str());
				fu16printf(fp, L"======================================\r\n");
				fu16printf(fp, L"%ls\r\n", sTxt.c_str());
				fu16printf(fp, L"--------------------------------------\r\n");
			}
		}
		fclose(fp);
	}
	return 0;
}
