#include "mygit.h"
#include <iostream>
#include <pwd.h>
#include <unistd.h>

using namespace std;

void initRepo()
{
    string repoDir=".mygit";
    if(filesystem::exists(repoDir))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: destination path "<<repoDir<<" already exists and is not an empty directory."<<"\n";
        return;
    }
    char cwd[256];
    if(getcwd(cwd,sizeof(cwd))==nullptr)
    {
        cout<<"\nError";
        return;
    }
    filesystem::create_directory(".mygit");
    filesystem::create_directory(".mygit/branches");
    filesystem::create_directory(".mygit/hooks");
    filesystem::create_directory(".mygit/info");
    filesystem::create_directory(".mygit/objects");
    filesystem::create_directory(".mygit/objects/info");
    filesystem::create_directory(".mygit/objects/pack");
    filesystem::create_directory(".mygit/refs");
    filesystem::create_directory(".mygit/refs/heads");
    filesystem::create_directory(".mygit/refs/tags");
    ofstream config(".mygit/config");
    config<<"[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n";
    config.close();
    ofstream description(".mygit/description");
    description<<"Unnamed repository; edit this file 'description' to name the repository.\n";\
    description.close();
    ofstream head(".mygit/HEAD");
    head<<"ref: refs/heads/master\n";
    head.close();
    ofstream exclude(".mygit/info/exclude");
    exclude<<"# git automatically ignores certain files by default\n";
    exclude<<"*.o\n";
    exclude<<"*.obj\n";
    exclude<<"*~\n";
    exclude<<"# Add other default excludes here\n";
    exclude.close();
    cout<<"\x1B[93m";
    cout<<"hint: Using 'master' as the name for the initial branch. This default branch name\n";
    cout<<"hint: is subject to change. To configure the initial branch name to use in all\n";
    cout<<"hint: of your new repositories, which will suppress this warning, call:\n";
    cout<<"hint:\n";
    cout<<"hint:   mygit config --global init.defaultBranch <name>\n";
    cout<<"hint:\n";
    cout<<"hint: Names commonly chosen instead of 'master' are 'main', 'trunk' and\n";
    cout<<"hint: 'development'. The just-created branch can be renamed via this command:\n";
    cout<<"hint:\n";
    cout<<"hint:   mygit branch -m <name>\n";
    cout<<"\033[0m";
    cout<<"Initialized empty repository in "<<cwd<<"/.mygit/\n";
}