#include "mygit.h"
#include <iostream>
#include <unordered_map>
#include <sys/stat.h>

using namespace std;

void restoreBlobFromHash(const string &blobSha,const string &filePath)
{
    string blobPath=".mygit/objects/"+blobSha.substr(0,2)+"/"+blobSha.substr(2);
    ifstream blobFile(blobPath,ios::binary);
    if(!blobFile.is_open())
    {
        cout <<"\x1B[31m\nfatal: Could not read blob object " <<blobSha <<"\n";
        return;
    }
    string header;
    getline(blobFile,header,'\0');
    if(header.rfind("blob",0)!=0)
    {
        cout<<"\x1B[31m\nfatal: Corrupt blob object: invalid header\n";
        blobFile.close();
        return;
    }
    ofstream outFile(filePath,ios::trunc | ios::binary);
    if(!outFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Could not write to file "<<filePath<<"\n";
        blobFile.close();
        return;
    }
    outFile<<blobFile.rdbuf();
    cout<<"\x1B[92mRestored file: "<<filePath<<"\n";
    blobFile.close();
    outFile.close();
}

unordered_map<string,string> loadCommitFileHashes(const string &commitSha)
{
    unordered_map<string,string> fileHashes;
    string commitFileListPath=".mygit/commits/"+commitSha+"_files.txt";
    ifstream commitFilesList(commitFileListPath);
    if(!commitFilesList.is_open())
    {
        cout<<"\x1B[31m\nfatal: Could not load files for commit "<<commitSha<<"\n";
        return fileHashes;
    }
    string line;
    while(getline(commitFilesList,line))
    {
        size_t spacePos=line.find(' ');
        if(spacePos!=string::npos)
        {
            string filePath=line.substr(0,spacePos);
            string fileHash=line.substr(spacePos+1);
            fileHashes[filePath]=fileHash;
        }
    }
    commitFilesList.close();
    return fileHashes;
}

void cleanWorkingDirectory()
{
    for(const auto &entry:filesystem::directory_iterator(filesystem::current_path()))
    {
        string relativePath=filesystem::relative(entry.path(),filesystem::current_path()).string();
        if(relativePath.find(".mygit")!=string::npos || entry.path().filename()=="mygit")
            continue;
        else
            filesystem::remove_all(entry);
    }
}

void restoreFilesFromCommit(const string &commitSha)
{
    unordered_map<string,string> fileHashes=loadCommitFileHashes(commitSha);
    for(const auto &fileEntry:fileHashes)
    {
        const string &filePath=fileEntry.first;
        const string &blobSha=fileEntry.second;
        restoreBlobFromHash(blobSha,filePath);
    }
}

void checkout(const string &commitSha)
{
    const char *repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        return;
    }
    ofstream headFile(".mygit/HEAD",ios::trunc);
    if(!headFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Could not update HEAD file.\n";
        return;
    }
    headFile<<commitSha;
    headFile.close();
    cleanWorkingDirectory();
    restoreFilesFromCommit(commitSha);
    cout<<"\x1B[93m\nWorking directory updated to commit "<<commitSha<<"\n";
}