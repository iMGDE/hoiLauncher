#pragma region INCLUDES
//gen
#include <fstream>
#include <filesystem>
#include <list>
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//GUI
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Browser.H>
#pragma endregion

#pragma region GLOBALS
pthread_t threads[4];
#pragma endregion

#pragma region DEFINES
#define splitStr(str,split,out,rq)\
{size_t iIndex = str.find(split);\
if(iIndex != std::string::npos && str.length() >= strlen(split))\
    out = str.substr(iIndex + strlen(split), str.length() - iIndex - strlen(split));\
if(rq) {\
    out.erase(out.begin());\
    out.erase(out.end()-1);\
}}
#define splitStrR(str,split,out,rq)\
{size_t iIndex = str.rfind(split);\
if(iIndex != std::string::npos && str.length() >= strlen(split))\
    out = str.substr(iIndex + strlen(split), str.length() - iIndex - strlen(split));\
if(rq) {\
    out.erase(out.begin());\
    out.erase(out.end()-1);\
}}
#pragma endregion

namespace Mods
{
    class Mod
    {
    private:
        bool bActive;
    public:
        std::string name;
        std::string descriptor;
        std::string path;
        std::string version;
        unsigned long long remote_id;
        int num;
        Mod(std::string descriptor)
        {
            bActive = 0;
            this->descriptor = descriptor;
            std::string l;
            std::ifstream f;
            f.open(descriptor);
            if (!f.is_open()) std::cout << "sob" << std::endl;
            while (getline(f,l))
            {
                if(l.find("remote_file_id=") != std::string::npos){
                    std::string remID{};
                    splitStr(l,"id=",remID,1)
                    remote_id = std::stoll(remID);
                }
                if(l.find("name=") != std::string::npos){
                    splitStr(l,"ame=",name,1)
                }
                if(l.find("path=") != std::string::npos){
                    splitStr(l,"ath=",path,0)
                }
                if(l.find("archive=") != std::string::npos){
                    splitStr(l,"rchive=",path,0)
                }
                if(l.find("supported_version=") != std::string::npos){
                    splitStr(l,"upported_version=",version,1)
                }
                
            }
            f.close();
            std::cout << "[MODL] loaded Mod " << name << ";;" << version << ";;" << remote_id << " ;; " << descriptor << "\n";
        }
        bool isLocal()
        {
            if(path.find("Paradox Interactive/Hearts of Iron IV") != std::string::npos)
                return 1;
            return 0;
        }
        void openInSteam()
        {
            //TODO implementusus
        }
        std::string shortModDesc()
        {
            std::string out{};
            splitStrR(descriptor, "/",out,0)
            return out;
        }
        bool isActive()
        {
            return bActive;
        }
        void setActive(bool a)
        {
            if(a)
            {
                std::cout << "[MODS] added " << name << " to active Mods\n";
            } else {
                std::cout << "[MODS] removed " << name << " from active Mods\n";
            }
            bActive = a;
        }
    };
    
    bool sortAfterName(const Mod& first, const Mod& second) { return first.name < second.name; }

    std::string resetDLCload()
    {
        std::string dlcl{"{\"enabled_mods\":[],\"disabled_dlcs\":[]}"};
        std::ofstream fo;
        fo.open("/home/imgde/.local/share/Paradox Interactive/Hearts of Iron IV/dlc_load.json",std::ofstream::out | std::ofstream::trunc);
        if(!fo.is_open()) std::cout << "sob DLCLOAD" << std::endl;
        fo << dlcl.c_str();
        fo.flush();
        return dlcl;
    }

    static std::map<int,Mod> mods;
    
    static void getMods(Fl_Check_Browser* t)
    {
        std::cout << "[DEBUG] getMods start\n";
        //load all .mod files
        std::list<Mod> _listMods = std::list<Mod>();
        for (const auto& entry : std::filesystem::directory_iterator("/home/imgde/.local/share/Paradox Interactive/Hearts of Iron IV/mod"))
        {   
            //TODO implement check if file is .mod
            if(entry.exists() && entry.is_regular_file())
            {
                _listMods.push_back(Mod(entry.path().string()));
            }
        }
        if(_listMods.empty()) 
        {
            std::cout << "[DEBUG] no mods installed?\n";
            return;
        }
        std::cout << "[MODS] got " << _listMods.size() << " installed Mods\n";
        
        _listMods.sort(Mods::sortAfterName);
        //sort files with id into map
        for(auto item : _listMods)
        {
            std::string x("{" + std::to_string(item.remote_id)+ "} ");
            if(item.isLocal())
                x = x + "@ ";
            x = x + item.name + " [" + item.version + "]";
            item.num = t->add(x.c_str() ,item.isActive());
            Mods::mods.insert(std::pair<int,Mods::Mod>(item.num, item));
            std::cout << x << "\n" << item.num << ": " << item.name << " :: " << item.shortModDesc() << "\n";
        }

        //get active mods and toggle them
        std::string l;
        std::ifstream fi;
        fi.open("/home/imgde/.local/share/Paradox Interactive/Hearts of Iron IV/dlc_load.json");
        if (!fi.is_open()) std::cout << "sob I" << std::endl;
        getline(fi,l);
        fi.close();
        if(l.empty()) l = resetDLCload();
        std::cout << l << "\n";
        if(l.find("enabled_mods\":[") == std::string::npos || l.find("]") == std::string::npos) exit(187);
        std::cout << "[DEBUG] " << l.find(":[") << " :: " << l.find("]") << "\n";
        std::string modsStr{};
        splitStr(l,":[",modsStr,0)
        if(modsStr.find("],") != std::string::npos) modsStr.erase(modsStr.find("],"),std::string::npos);

        if(modsStr.empty())
        {
            std::cout << "[DEBUG] no active mods?\n";
            return;
        }
        //split str
        size_t pos = 0;
        std::list<std::string> activeMods = std::list<std::string>();
        while ((pos = modsStr.find(",")) != std::string::npos) {
            activeMods.push_back(modsStr.substr(5, pos-5-1)); // "mod/ = 5
            modsStr.erase(0, pos+1);
        }
        activeMods.push_back(modsStr.substr(5, modsStr.length()-5-1)); //5 = first 5, 1 = "
        //set each active mod active
        for(auto activeS : activeMods)
        {
            bool bFinished = 0;
            for(auto mod : mods)
            {
                if(mod.second.shortModDesc() == activeS)
                {
                    mod.second.setActive(1);
                    t->set_checked(mod.second.num);
                    std::cout << "[DEBUG MOD] " << mod.second.isActive() << "\n";
                    bFinished = 1;
                    break;
                }
            }
            if(!bFinished)
                std::cout << "[DEBUG] wtf bro mod not installed? " << activeS << "\n";
        }
    }

    static void buildModList(Fl_Check_Browser* t)
    {
        std::cout << "[DEBUG] buildModList start\n";
        std::ofstream fo;
        fo.open("/home/imgde/.local/share/Paradox Interactive/Hearts of Iron IV/dlc_load.json",std::ofstream::out | std::ofstream::trunc);
        if(!fo.is_open()) std::cout << "sob O" << std::endl;
        std::string modsStr{};
        for(auto item : mods)
        {
            if(item.second.isActive())
            {
                modsStr = modsStr + ",\"mod/" + item.second.shortModDesc() + "\"";
            }
        }
        modsStr.erase(0,1);
        fo << "{\"enabled_mods\":[" << modsStr << "],\"disabled_dlcs\":[]}";
        fo.close();
    }
}

namespace Launcher
{
    static bool bDebug = 0;
    static bool bCheats = 0;
    static std::string suffix{};
    static void* startHOI(void* dummy)
    {
        std::string start = "cd /home/imgde/.steam/steam/steamapps/common/Hearts\\ of\\ Iron\\ IV/";
        
        if(bCheats) {
            start.append(" && export LD_PRELOAD=/usr/lib/libfunchook.so.1.1.0:/home/imgde/Dev/PROJECTS/IronMP/bin/libAmogHook.so");
        }
        start.append(" && ./hoi4 -steam");
        
        if(bDebug) {
            start.append(" -debug");
        }
        int pid = fork();
        if(pid == 0) {
            system(start.c_str());
            std::cout << "[DEBUG] after game start ig?\n";
        } else {
            std::cout << "[BOOT] " << start.c_str() << "\n";
            std::cout << "[BOOT] " << pid << ": launched?\n";
        }
        return dummy;
    }
    static void killHOI()
    {
        //kllappt nicht lol
        //system("pkill -9 hoi4");

        //TODO add /proc iter die 
        //sigqueue(pid,9,0);
    }

}

class UserInterfasus {
private:
    //undef
    static void b_selectMods(Fl_Button*, void*);
    static void b_unselectAllMods(Fl_Button*, void*);

    //boot
    static void b_hackerButton(Fl_Button* t, void*) {
        
	    pthread_create(&threads[1], NULL, &Launcher::startHOI, NULL);
    }
    static void cb_installCE(Fl_Check_Button* t, void*) { Launcher::bCheats = t->value(); }
    static void cb_debugMode(Fl_Check_Button* t, void*) { Launcher::bDebug = t->value(); }
    static void cb_modList(Fl_Check_Browser* t, void*)
    {
        for(int i=1; i<=t->nitems(); i++)
        {
            if(t->checked(i))
                Mods::mods.at(i).setActive(1);
            else
                Mods::mods.at(i).setActive(0);
        }
    }
    
    //cock
	static void cb_hotel4uhr(Fl_Check_Button* t, void*) {
        std::cout << "hotel4uhr\n";
    }
	static void cb_louisBuitton(Fl_Light_Button* t, void*) {
        std::cout << "louis\n";
    }
    static void cb_floss808(Fl_Counter* t, void*) {
        std::cout << "floss\n";
    }
    
    //boot other
    
public:
    Fl_Window* window;
    Fl_Tabs *tabs;
    //boot tool
    Fl_Button *hackerButton;
    Fl_Button *buttonSelectMods;
    Fl_Button *buttonUnselectAllMods;
    Fl_Check_Button *installCE;
    Fl_Check_Button *debugMode;
    Fl_Check_Browser *modList;

    //cock sucking
    Fl_Light_Button *louisBuitton;
    Fl_Check_Button *hotel4uhr;
    Fl_Counter *floss808;
    Fl_Choice *choiceAI;
    Fl_Window* open()
	{
        Fl_Window* o = new Fl_Window(500, 500, "pdx sucks tool");
        o->user_data((void*)(this));
        tabs = new Fl_Tabs(0,0,500,500,"");
        {
            Fl_Group *grp1 = new Fl_Group(0,0,500,470,"hoi4 hacker tool");
            {
                {
                    hackerButton = new Fl_Button(0, 0, 110, 25, "hacker button");
                    hackerButton->color(fl_rgb_color(255, 192, 203));
                    hackerButton->when(FL_WHEN_RELEASE);
                    hackerButton->callback((Fl_Callback*)b_hackerButton);
                } // Fl_Button* hackerButton
                {
                    buttonSelectMods = new Fl_Button(110, 0, 110, 25, "Lock In Mods");
                    buttonSelectMods->color(fl_rgb_color(255, 192, 203));
                    buttonSelectMods->when(FL_WHEN_RELEASE);
                    buttonSelectMods->callback((Fl_Callback*)b_selectMods);
                } // Fl_Button* buttonUnselectAllMods
                {
                    buttonUnselectAllMods = new Fl_Button(110, 25, 110, 25, "Clear Mods");
                    buttonUnselectAllMods->color(fl_rgb_color(255, 192, 203));
                    buttonUnselectAllMods->when(FL_WHEN_RELEASE);
                    buttonUnselectAllMods->callback((Fl_Callback*)b_unselectAllMods);
                } // Fl_Button* buttonUnselectAllMods
                {
                    installCE = new Fl_Check_Button(220, 0, 110, 25, "arthur button?");
                    installCE->callback((Fl_Callback*)cb_installCE);
                } // Fl_Check_Button* installCE
                {
                    debugMode = new Fl_Check_Button(220, 25, 110, 25, "debug");
                    debugMode->callback((Fl_Callback*)cb_debugMode);
                } // Fl_Check_Button* debugMode
                {
                    modList = new Fl_Check_Browser(0,50,500,420);
                    modList->when(FL_WHEN_CHANGED);
                    modList->callback((Fl_Callback*)cb_modList);
                } // Fl_Check_Browser *modList;
                
            }
            grp1->end();
            Fl_Group *grp2 = new Fl_Group(0,0,500,460,"hmm development 100%");
            {
                {
                    louisBuitton = new Fl_Light_Button(0, 25, 110, 25, "louis buitton");
                    louisBuitton->when(FL_WHEN_RELEASE);
                    louisBuitton->callback((Fl_Callback*)cb_louisBuitton);
                } // Fl_Light_Button* louisBuitton
                { 
                    hotel4uhr = new Fl_Check_Button(0, 50, 110, 25, "4uhr hotel");
                    hotel4uhr->when(FL_WHEN_RELEASE);
                    hotel4uhr->callback((Fl_Callback*)cb_hotel4uhr);
                } // Fl_Check_Button* hotel4uhr
                { 
                    floss808 = new Fl_Counter(135, 0, 110, 25, "808 floss");
                    floss808->minimum(1);
                    floss808->step(1);
                    floss808->value(1);
                    floss808->when(FL_WHEN_CHANGED);
                    floss808->callback((Fl_Callback*)cb_floss808);
                } // Fl_Counter* floss808
                {
                    choiceAI = new Fl_Choice(245, 0, 84, 25);
                    choiceAI->when(FL_WHEN_CHANGED);
                } // Fl_Choice* choiceAI
            }
            grp2->end();
        }
        tabs->end();
        o->end();
        return o;
	}
    UserInterfasus()
    {
        window = open();
    }
};

//muss weil sonst tickt compiler aus mit read only session und static/non static gedöns
static UserInterfasus ui;

void UserInterfasus::b_selectMods(Fl_Button*, void*) {
    Mods::buildModList(ui.modList);
}
void UserInterfasus::b_unselectAllMods(Fl_Button*, void*){
    for(int i=1;i<=ui.modList->nitems();i++)
    {
        ui.modList->checked(i,0);
    }
}

int main(int argc, char **argv) {
    ui = UserInterfasus();
    Mods::getMods(ui.modList);
    ui.window->show(argc, argv);
    return Fl::run();
}