#include "mygit.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

using namespace std;

struct TreeEntry {
    string mode;
    string name;
    string sha1Hex;
    bool isTree;
};

void lsTree(const string &treeSha,bool nameOnly=false)
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    if(treeSha.length()<40)
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Invalid SHA length: "<<treeSha<<"\n";
        return;
    }
    string objectDir=".mygit/objects/"+treeSha.substr(0,2);
    string objectPath=objectDir+"/"+treeSha.substr(2);
    if(!filesystem::exists(objectPath))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Tree object with SHA "<<treeSha<<" not found.\n";
        exit(1);
    }
    ifstream treeFile(objectPath,ios::binary);
    if(!treeFile.is_open())
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Could not open tree object file "<<objectPath<<"\n";
        exit(1);
    }
    string header;
    getline(treeFile,header,'\0');
    if(header.rfind("tree",0)!=0)
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Object with SHA "<<treeSha<<" is not a tree.\n";
        return;
    }
    streampos lastPos;
    string mode,name;
    vector<TreeEntry> entries;
    unsigned char sha1Binary[20];
    while(treeFile.peek()!=EOF)
    {
        lastPos=treeFile.tellg();
        getline(treeFile,mode,' ');
        getline(treeFile,name,'\0');
        if(name.empty() || mode.empty())
        {
            treeFile.clear();
            treeFile.seekg(lastPos);
            break;
        }
        treeFile.read(reinterpret_cast<char *>(sha1Binary),20);
        ostringstream sha1Hex;
        for(int i=0;i<20;++i)
            sha1Hex<<hex<<setw(2)<<setfill('0')<<static_cast<int>(sha1Binary[i]);
        entries.push_back({mode,name,sha1Hex.str(),mode=="40000"});
    }
    for(size_t i=0;i<entries.size()-1;i++)
    {
        const auto &entry=entries[i];
        if(nameOnly)
            cout<<entry.name<<"\n";
        else
        {
            string name=entry.name;
            struct stat fileStat;
            if(stat(name.c_str(),&fileStat)==-1)
            {
                perror("\nError: ");
                continue;
            }
            cout<<((S_ISDIR(fileStat.st_mode))?"d":"-");
            cout<<((fileStat.st_mode & S_IRUSR)?"r":"-");
            cout<<((fileStat.st_mode & S_IWUSR)?"w":"-");
            cout<<((fileStat.st_mode & S_IXUSR)?"x":"-");
            cout<<((fileStat.st_mode & S_IRGRP)?"r":"-");
            cout<<((fileStat.st_mode & S_IWGRP)?"w":"-");
            cout<<((fileStat.st_mode & S_IXGRP)?"x":"-");
            cout<<((fileStat.st_mode & S_IROTH)?"r":"-");
            cout<<((fileStat.st_mode & S_IWOTH)?"w":"-");
            cout<<((fileStat.st_mode & S_IXOTH)?"x":"-");
            cout<<" "<<name<<" "<<(entry.mode=="40000"?"tree":"blob")<<" SHA-1: "<<entry.sha1Hex<<"\n";
        }
    }
}