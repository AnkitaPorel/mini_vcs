#include "mygit.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    if(argc<2)
    {
        cout<<"\nInvalid number of arguments\n";
        return 0;
    }
    string command=argv[1];
    if(command=="init")
    {
        if(argc>2)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        initRepo();
    }
    else if(command=="hash-object")
    {
        if(argc<3 || argc>4)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        bool writeBlob=(argc==4 && string(argv[2])=="-w");
        string filename=argv[writeBlob?3:2];
        string sha=hashObject(filename,writeBlob);
        cout<<"\nSHA1- "<<sha<<"\n";
    }
    else if(command=="cat-file")
    {
        if(argc!=4)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        string flag=argv[2];
        string fileSha=argv[3];
        catFile(flag,fileSha);
    }
    else if(command=="write-tree")
    {
        if(argc!=2)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        string str=writeTree();
        cout<<"\n"<<str<<"\n";
    }
    else if(command=="ls-tree")
    {
        if(argc<3 || argc>4)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        bool nameOnly=(argc==4 && string(argv[2])=="--name-only");
        string treeSha=argv[nameOnly?3:2];
        lsTree(treeSha,nameOnly);
    }
    else if(command=="add")
    {
        if(argc<3)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        vector<string> files(argv+2,argv+argc);
        addFiles(files);
    }
    else if(command=="commit")
    {
        string message="Default commit message";
        if(argc==4 && string(argv[2])=="-m")
            message=argv[3];
        else if(argc>2)
        {
            cout<<"\nInvalid arguments. Use: ./mygit commit -m \"Commit message\"\n";
            return 0;
        }
        commitChanges(message);
    }
    else if(command=="log")
    {
        if(argc!=2)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        showLog();
    }
    else if(command=="checkout")
    {
        if(argc!=3)
        {
            cout<<"\nInvalid number of arguments\n";
            return 0;
        }
        string commitSha=argv[2];
        checkout(commitSha);
    }
    else
        cout<<"\nUnknown command: "<<command<<"\n";
    return 0;
}