#include <home/System.hpp>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <dirent.h>
#include <stratosphere.hpp>

extern AppletHolder global_hold_applet;

namespace home
{
    void DieExecution(std::string Msg)
    {
        consoleInit(NULL);
        std::cout << std::endl << "eQlipse error" << std::endl << "An error happened while launching menu: " << Msg << std::endl << "Press any key to power off.";
        consoleUpdate(NULL);
        while(true)
        {
            hidScanInput();
            if(hidKeysDown(CONTROLLER_P1_AUTO)) break;
        }
        consoleExit(NULL);
        exit(0);
    }

    void Initialize()
    {
        if(romfsInit() != 0) DieExecution("Failed to initialize RomFs.");
        if(nsInitialize() != 0) DieExecution("Failed to initialize service: 'ns:am'.");
        if(ncmInitialize() != 0) DieExecution("Failed to initialize service: 'ncm'.");
        mkdir(BaseDir.c_str(), 777);
        mkdir(ThemesDir.c_str(), 777);
        mkdir(ItemsMetaDir.c_str(), 777);
    }

    void Exit()
    {
        nsExit();
        ncmExit();
        romfsExit();
    }

    static void CreateFile(std::string p)
    {
        FILE *f = fopen(p.c_str(), "w");
        fclose(f);
    }

    static void CreateDir(std::string p)
    {
        mkdir(p.c_str(), 777);
    }

    static void MoveFile(std::string p1, std::string p2)
    {
        rename(p1.c_str(), p2.c_str());
    }

    static void DeleteFile(std::string p)
    {
        remove(p.c_str());
    }

    static void DeleteDirOnce(std::string p)
    {
        rmdir(p.c_str());
    }

    static void CopyFile(std::string p, std::string np)
    {
        FILE *inf = fopen(p.c_str(), "rb");
        FILE *outf = fopen(np.c_str(), "wb");
        fseek(inf, 0, SEEK_END);
        u64 fsize = ftell(inf);
        rewind(inf);
        u64 readsize = 0x4000;
        u64 tocopy = fsize;
        u8 *tmp = (u8*)malloc(readsize);
        memset(tmp, 0, readsize);
        while(tocopy)
        {
            auto read = fread(tmp, 1, std::min(readsize, tocopy), inf);
            fwrite(tmp, 1, read, outf);
            tocopy -= read;
        }
        free(tmp);
        fclose(outf);
        fclose(inf);
    }

    static void DeleteDir(std::string d)
    {
        DIR *dp = opendir(d.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                std::string fullp = d + "/" + std::string(dt->d_name);
                if(dt->d_type & DT_DIR) DeleteDirOnce(fullp);
                else if(dt->d_type & DT_REG) DeleteFile(fullp);
            }
            closedir(dp);
        }
        DeleteDirOnce(d);
    }

    static void DirPerform(std::string d, std::string nd, bool copy)
    {
        DIR *dp = opendir(d.c_str());
        CreateDir(nd);
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                std::string fullp = d + "/" + std::string(dt->d_name);
                std::string nfullp = nd + "/" + std::string(dt->d_name);
                if(dt->d_type & DT_DIR) DirPerform(fullp, nfullp, copy);
                else if(dt->d_type & DT_REG)
                {
                    if(copy) CopyFile(fullp, nfullp);
                    else MoveFile(fullp, nfullp);
                }
            }
            closedir(dp);
        }
        if(!copy) DeleteDirOnce(d);
    }

    static void MoveDir(std::string d, std::string nd)
    {
        DirPerform(d, nd, false);
    }

    static void CopyDir(std::string d, std::string nd)
    {
        DirPerform(d, nd, true);
    }

    static bool Exists(std::string p)
    {
        struct stat st;
        return (stat(p.c_str(), &st) == 0);
    }

    bool RunningAtmosphere()
    {
        u64 tmpc = 0;
        splInitialize();
        bool ok = R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &tmpc));
        splExit();
        return ok;
    }

    bool RunningReiNX()
    {
        Handle tmph = 0;
        Result rc = smRegisterService(&tmph, "rnx", false, 1);
        if(R_FAILED(rc)) return true;
        smUnregisterService("rnx");
        return false;
    }

    bool RunningSXOS()
    {
        Handle tmph = 0;
        Result rc = smRegisterService(&tmph, "tx", false, 1);
        if(R_FAILED(rc)) return true;
        smUnregisterService("tx");
        return false;
    }

    std::string GetBaseCfwDir()
    {
        if(RunningAtmosphere()) return "sdmc:/atmosphere";
        else if(RunningReiNX()) return "sdmc:/ReiNX";
        else if(RunningSXOS()) return "sdmc:/sxos";
        return "";
    }

    void TargetHbmenu(std::string Path)
    {
        HandleLayeredFs(BinDir + "/LibraryAppletHbTarget", TitleId_AppletShop);
        LibAppletArgs args;
        libappletArgsCreate(&args, HbTargetMagic);
        appletCreateLibraryApplet(&global_hold_applet, AppletId_shop, LibAppletMode_AllForeground);
        libappletArgsPush(&args, &global_hold_applet);
        HbTargetArgs hargs = {};
        hargs.Magic = HbTargetMagic;
        strcpy(hargs.Entry, Path.c_str());
        libappletPushInData(&global_hold_applet, &hargs, sizeof(HbTargetArgs));
        libappletStart(&global_hold_applet);
        appletHolderClose(&global_hold_applet);
        UnhandleLayeredFs(TitleId_AppletShop);
    }

    void HandleLayeredFs(std::string Path, u64 AppId)
    {
        std::stringstream strm;
        strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << AppId;
        auto lfspath = GetBaseCfwDir() + "/titles/" + strm.str();
        if(Exists(lfspath))
        {
            CreateDir(LFsTempDir);
            std::string tmp = LFsTempDir + "/" + strm.str();
            MoveDir(lfspath, tmp);
        }
        CopyDir(Path, lfspath);
    }

    void UnhandleLayeredFs(u64 AppId)
    {
        std::stringstream strm;
        strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << AppId;
        auto lfspath = GetBaseCfwDir() + "/titles/" + strm.str();
        if(Exists(lfspath)) DeleteDir(lfspath);
        CreateDir(LFsTempDir);
        std::string tmp = LFsTempDir + "/" + strm.str();
        if(Exists(tmp)) MoveDir(tmp, lfspath);
    }

    std::vector<Title> GetTitlesForStorage(FsStorageId Id)
    {
        std::vector<Title> titles;
        NcmContentMetaDatabase metadb;
        Result rc = ncmOpenContentMetaDatabase(Id, &metadb);
        if(rc == 0)
        {
            NcmContentStorage cst;
            rc = ncmOpenContentStorage(Id, &cst);
            if(rc == 0)
            {
                u32 srecs = sizeof(NcmMetaRecord) * 128;
                NcmMetaRecord recs[128] = {0};
                u32 wrt = 0;
                u32 total = 0;
                rc = ncmContentMetaDatabaseList(&metadb, 0x80, 0, 0, U64_MAX, recs, srecs, &wrt, &total);
                if((rc == 0) && (wrt > 0))
                {
                    titles.reserve(wrt);
                    for(u32 i = 0; i < wrt; i++)
                    {
                        u64 tid = recs[i].titleId;
                        NsApplicationControlData ctdata = {};
                        auto rc2 = nsGetApplicationControlData(1, tid, &ctdata, sizeof(ctdata), NULL);
                        if(rc2 == 0)
                        {
                            NacpLanguageEntry *ent;
                            nacpGetLanguageEntry(&ctdata.nacp, &ent);
                            if(ent != NULL)
                            {
                                Title t = {};
                                t.Name = std::string(ent->name);
                                t.Author = std::string(ent->author);
                                t.Storage = Id;
                                t.TotalSize = 0;
                                t.Id = tid;
                                t.Version = std::string(ctdata.nacp.version);
                                t.NeedsUser = (((u8*)&ctdata.nacp)[0x3025] > 0);

                                for(u32 j = 0; j < 6; j++)
                                {
                                    NcmNcaId contentid;
                                    rc2 = ncmContentMetaDatabaseGetContentIdByType(&metadb, (NcmContentType)j, &recs[i], &contentid);
                                    if(rc2 == 0)
                                    {
                                        u64 tmpsize = 0;
                                        ncmContentStorageGetSize(&cst, &contentid, &tmpsize);
                                        t.TotalSize += tmpsize;
                                    }
                                }

                                std::stringstream strm;
                                strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << tid;
                                std::string strid = strm.str();

                                mkdir(ItemsMetaDir.c_str(), 777);
                                std::string iconpth = ItemsMetaDir + "/" + strid + ".jpg";
                                
                                remove(iconpth.c_str());
                                FILE *iconf = fopen(iconpth.c_str(), "wb");
                                if(iconf)
                                {
                                    fwrite(ctdata.icon, 1, sizeof(ctdata.icon), iconf);
                                    fclose(iconf);
                                }
                                else DieExecution("Bad path? " + iconpth);

                                titles.push_back(t);
                            }
                        }
                    }
                }
                serviceClose(&cst.s);
            }
            serviceClose(&metadb.s);
        }
        return titles;
    }

    std::vector<Title> GetAllTitles()
    {
        std::vector<Title> titles;

        auto gc = GetTitlesForStorage(FsStorageId_GameCard);
        if(!gc.empty())
        {
            titles.reserve(gc.size());
            titles.insert(titles.end(), gc.begin(), gc.end());
            gc.clear();
        }
        
        auto sd = GetTitlesForStorage(FsStorageId_SdCard);
        if(!sd.empty())
        {
            titles.reserve(sd.size());
            titles.insert(titles.end(), sd.begin(), sd.end());
            sd.clear();
        }

        auto n = GetTitlesForStorage(FsStorageId_NandUser);
        if(!n.empty())
        {
            titles.reserve(n.size());
            titles.insert(titles.end(), n.begin(), n.end());
            n.clear();
        }

        return titles;
    }
}