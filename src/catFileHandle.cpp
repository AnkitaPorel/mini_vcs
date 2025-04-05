#include "mygit.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;

void catFile(string &flag,string &fileSha)
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    string objectDir=".mygit/objects/"+fileSha.substr(0,2);
    string objectPath=objectDir+"/"+fileSha.substr(2);
    if(!filesystem::exists(objectPath))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Object with SHA "<<fileSha<<" not found.\n";
        return;
    }
    ifstream objectFile(objectPath,ios::binary | ios::ate);
    if(!objectFile.is_open())
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Could not open object file "<<objectPath<<"\n";
        return;
    }
    streampos totalFileSize=objectFile.tellg();
    objectFile.seekg(0,ios::beg);
    string header;
    getline(objectFile,header,'\0');
    if(flag=="-t")
    {
        if(header.rfind("blob",0)==0)
            cout<<"blob\n";
        else if(header.rfind("tree",0)==0)
            cout<<"tree\n";
        else
        {
            cout<<"\x1B[31m";
            cout<<"fatal: Unknown object type\n";
        }
    }
    else if(flag=="-p")
    {
        if(header.rfind("blob",0)==0)
            cout<<objectFile.rdbuf()<<"\n";
        else if(header.rfind("tree",0)==0)
            lsTree(fileSha,true);
        else
        {
            cout<<"\x1B[31m";
            cout<<"\nfatal: -p flag only supports blob and tree objects\n";
        }
    }
    else if(flag=="-s")
    {
        streampos fileSize=objectFile.tellg();
        objectFile.seekg(0,ios::end);
        streampos contentSize=objectFile.tellg()-static_cast<streamoff>(header.size()+1);
        cout<<contentSize<<" bytes\n";
    }
    else
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Invalid flag "<<flag<<"\n";
    }
    objectFile.close();
}