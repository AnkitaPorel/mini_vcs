#include "mygit.h"
#include <iostream>
#include <ctime>
#include <openssl/sha.h>
#include <sstream>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <iomanip>

using namespace std;

string computeSha1(const string &data)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()),data.size(),hash);
    stringstream ss;
    for(int i=0;i<SHA_DIGEST_LENGTH;i++)
        ss<<hex<<setw(2)<<setfill('0')<<static_cast<int>(hash[i]);
    return ss.str();
}

string getCurrentTimestamp()
{
    time_t now=time(0);
    char buf[80];
    strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",localtime(&now));
    return string(buf);
}

string getParentCommit()
{
    ifstream headFile(".mygit/HEAD");
    string parentCommit;
    if(headFile.is_open() && getline(headFile,parentCommit) && parentCommit.size()==40)
    {
        headFile.close();
        return parentCommit;
    }
    return "";
}

unordered_set<string> loadStagedFiles()
{
    unordered_set<string> stagedFiles;
    ifstream stagedFile(".mygit/staging/staged_files.txt");
    string filePath;
    while(getline(stagedFile,filePath))
        stagedFiles.insert(filePath);
    return stagedFiles;
}

string getTreeHashFromCommit(const string &commitSha)
{
    string commitPath=".mygit/objects/"+commitSha.substr(0,2)+"/"+commitSha.substr(2);
    ifstream commitFile(commitPath);
    if(!commitFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Could not read latest commit object.\n";
        return "";
    }
    string line,treeHash;
    while(getline(commitFile,line))
    {
        if(line.rfind("tree",0)==0)
        {
            treeHash=line.substr(5);
            break;
        }
    }
    commitFile.close();
    return treeHash;
}

string getLatestTreeHash()
{
    ifstream indexFile(".mygit/index");
    if(!indexFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Unable to open index file.\n";
        return "";
    }
    string lastLine,line;
    while(getline(indexFile,line))
        lastLine=line;
    indexFile.close();
    if(!lastLine.empty())
    {
        size_t pos=lastLine.find_last_of(' ');
        if(pos!=string::npos)
            return lastLine.substr(pos+1);
    }
    cout<<"\x1B[31m\nfatal: No tree hash found in index file.\n";
    return "";
}

void commitChanges(const string &message)
{
    const char *repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        return;
    }
    unordered_set<string> stagedFiles=loadStagedFiles();
    if(stagedFiles.empty())
    {
        cout<<"\x1B[31m\nNothing in staging area.\n";
        return;
    }
    unordered_set<string> committedFiles;
    string parentSha1=getParentCommit();
    if(!parentSha1.empty())
    {
        ifstream commitFilesList(".mygit/commits/"+parentSha1+"_files.txt");
        string line;
        while(getline(commitFilesList,line))
        {
            size_t spacePos=line.find(" ");
            if (spacePos!=string::npos)
            {
                string filePath=line.substr(0,spacePos);
                committedFiles.insert(filePath);
            }
        }
        commitFilesList.close();
    }
    unordered_set<string> newFiles;
    for(const auto &file:stagedFiles)
    {
        if(committedFiles.find(file)==committedFiles.end())
            newFiles.insert(file);
    }
    if(newFiles.empty())
    {
        cout<<"\x1B[31m\nNo new changes to commit. All staged files are already committed.\n";
        return;
    }
    string treeSha1=getLatestTreeHash();
    if(treeSha1.empty())
    {
        cout<<"\nfatal: No tree object found for commit.\n";
        return;
    }
    string timestamp=getCurrentTimestamp();
    string user=getpwuid(getuid())->pw_name;
    string committer="Author: "+user;
    stringstream commitContent;
    commitContent<<"tree"<<treeSha1<<"\n";
    if(!parentSha1.empty())
        commitContent<<"parent "<<parentSha1<<"\n";
    commitContent<<committer<<"\n";
    commitContent<<"Date: "<<timestamp<<"\n";
    commitContent<<"\n";
    commitContent<<message<<"\n";
    string commitSerialized=commitContent.str();
    string commitSha1=computeSha1(commitSerialized);
    string commitDir=".mygit/objects/"+commitSha1.substr(0,2);
    string commitPath=commitDir+"/"+commitSha1.substr(2);
    filesystem::create_directories(commitDir);
    ofstream commitFile(commitPath,ios::binary);
    commitFile<<commitSerialized;
    commitFile.close();
    ofstream headFile(".mygit/HEAD",ios::trunc);
    headFile<<commitSha1;
    headFile.close();
    cout<<"Commit SHA-1: "<<commitSha1<<"\n";
    filesystem::create_directories(".mygit/commits");
    ofstream newCommitFilesList(".mygit/commits/"+commitSha1+"_files.txt");
    for(string file:stagedFiles)
    {
        ifstream fileName(file,ios::binary);
        size_t fileSize=filesystem::file_size(file);
        string fileHash=hashSha1(fileName,fileSize);
        hashObject(file,true);
        newCommitFilesList<<file<<" "<<fileHash<<"\n";
    }
    ofstream stagedFileOut(".mygit/staging/staged_files.txt",ios::app);
    for(const auto &file:stagedFiles)
    {
        if(newFiles.find(file)==newFiles.end())
            stagedFileOut<<file<<"\n";
    }
    stagedFileOut.close();
    cout<<"Files committed:\n";
    for(string file:newFiles)
        cout<<file<<"\n";
    newCommitFilesList.close();
}