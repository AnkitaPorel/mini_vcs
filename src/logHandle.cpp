#include "mygit.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>

using namespace std;

void showLog()
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    ifstream headFile(".mygit/HEAD");
    string currentCommit;
    if(!headFile.is_open() || !getline(headFile,currentCommit) || currentCommit.size()!=40)
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: No commits found.\n";
        return;
    }
    while(!currentCommit.empty())
    {
        string commitDir=".mygit/objects/"+currentCommit.substr(0,2);
        string commitPath = commitDir+"/"+currentCommit.substr(2);
        ifstream commitFile(commitPath);
        if(!commitFile.is_open())
        {
            cout<<"\x1B[31m";
            cout<<"\nfatal: Could not open commit "<<currentCommit<<"\n";
            break;
        }
        cout<<"Commit: "<<currentCommit<<"\n";
        string line;
        string parentCommit;
        while(getline(commitFile,line))
        {
            if(line.find("parent")==0)
                parentCommit=line.substr(7);
            else if(line.find("Date")==0)
                cout<<line<<"\n";
            else if(line.find("Author")==0)
                cout<<line<<"\n";
            else if(line.find("tree")==0)
                continue;
            else if(line.empty())
            {
                getline(commitFile,line);
                cout<<"Message: "<<line<<"\n\n";
                break;
            }
        }
        currentCommit=parentCommit;
    }
}
