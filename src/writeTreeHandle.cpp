#include "mygit.h"
#include <iostream>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

void updateIndex(const string &entry)
{
    ofstream indexFile;
    indexFile.open(".mygit/index",ios::trunc);
    if(!indexFile.is_open())
    {
        cout<<"\x1B[31m";
        cout<<"\nUnable to open index file for writing."<<"\n";
        exit(1);
    }
    indexFile<<entry<<"\n";
}

void storeObject(const string &objectHash,const string &content,const string &type)
{
    string objectDir=".mygit/objects/"+objectHash.substr(0,2);
    string objectFile=objectDir+"/"+objectHash.substr(2);
    filesystem::create_directories(objectDir);
    ofstream outFile(objectFile,ios::binary);
    if(!outFile)
    {
        cout<<"\x1B[31m";
        cout<<"\nUnable to write object: "<<objectFile<<"\n";
        exit(1);
    }
    string header=type+" "+to_string(content.size())+'\0';
    outFile<<header<<content;
    outFile.close();
}

string hashTreeSha(const string &treeContent)
{
    stringstream header;
    header<<"tree "<<treeContent.size()<<'\0';
    SHA_CTX sha1Context;
    SHA1_Init(&sha1Context);
    string headerStr=header.str();
    SHA1_Update(&sha1Context,headerStr.c_str(),headerStr.size());
    SHA1_Update(&sha1Context,treeContent.c_str(),treeContent.size());
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash,&sha1Context);
    stringstream hexStream;
    for(int i=0;i<SHA_DIGEST_LENGTH;i++)
        hexStream<<hex<<setw(2)<<setfill('0')<<static_cast<int>(hash[i]);
    return hexStream.str();
}

string createBlob(const string &filePath)
{
    ifstream file(filePath,ios::binary);
    if(!file.is_open())
    {
        cout<<"\x1B[31m";
        cout<<"\nUnable to open file: "+filePath<<"\n";
        return "";
    }
    stringstream buffer;
    buffer<<file.rdbuf();
    string fileContent=buffer.str();
    string blobHash=computeSha1(fileContent);
    string blobDir=".mygit/objects/"+blobHash.substr(0,2);
    filesystem::create_directories(blobDir);
    string blobPath=blobDir+"/"+blobHash.substr(2);
    if(!filesystem::exists(blobPath))
    {
        ofstream blobFile(blobPath,ios::binary);
        blobFile<<fileContent;
        blobFile.close();
    }
    return blobHash;
}

string recTree(const filesystem::path &dirPath)
{
    vector<string> entries;
    for(const auto &entry:filesystem::directory_iterator(dirPath))
    {
        if(entry.path().filename()==".mygit")
            continue;
        string entryHash;
        if(filesystem::is_regular_file(entry.path()))
        {
            entryHash=createBlob(entry.path().string());
            string fileEntry="100644 "+entry.path().filename().string()+'\0'+entryHash;
            entries.push_back(fileEntry);
            updateIndex(fileEntry);
        }
        else if(filesystem::is_directory(entry.path()))
        {
            entryHash=recTree(entry.path());
            string dirEntry="40000 "+entry.path().filename().string()+'\0'+entryHash;
            entries.push_back(dirEntry);
            updateIndex(dirEntry);
        }
    }
    stringstream treeContent;
    for(const auto &entry:entries)
        treeContent<<entry;
    string serializedTree=treeContent.str();
    string treeHash=hashTreeSha(serializedTree);
    storeObject(treeHash,serializedTree,"tree");
    return treeHash;
}

string writeTree()
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nError: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    ofstream indexFile;
    indexFile.open(".mygit/index",ios::trunc);
    if(!indexFile)
    {
        cout<<"\x1B[31m";
        cout<<"\nUnable to create index file.\n";
        exit(1);
    }
    return recTree(filesystem::current_path());
}