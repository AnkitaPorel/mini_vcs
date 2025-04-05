#include "mygit.h"
#include <iostream>
#include <openssl/sha.h>
#include <sstream>
#include <unordered_map>
#include <sys/stat.h>

using namespace std;

string computeFileHash(const string &filePath)
{
    ifstream file(filePath,ios::binary);
    if(!file.is_open())
        return "";
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    char buffer[8192];
    while(file.good())
    {
        file.read(buffer,sizeof(buffer));
        SHA1_Update(&sha1,buffer,file.gcount());
    }
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash,&sha1);
    stringstream ss;
    for(int i=0;i<SHA_DIGEST_LENGTH;i++)
        ss<<hex<<setw(2)<<setfill('0')<<(int)hash[i];
    return ss.str();
}

unordered_map<string,string> loadFileHashes()
{
    unordered_map<string,string> fileHashes;
    ifstream hashFile(".mygit/staging/file_hashes.txt");
    if(!hashFile.is_open())
        return fileHashes;
    string filePath,hash;
    while(hashFile>>filePath>>hash)
        fileHashes[filePath]=hash;
    return fileHashes;
}

void saveFileHashes(const unordered_map<string,string>&fileHashes)
{
    if(!filesystem::exists(".mygit/staging"))
        filesystem::create_directories(".mygit/staging");
    if(!filesystem::exists(".mygit/staging/staged_hashes.txt"))
    {
        ofstream createFile(".mygit/staging/staged_hashes.txt");
        if(!createFile.is_open())
        {
            cout<<"\x1B[31m\nfatal: Unable to create staging hash file.\n";
            return;
        }
        createFile.close();
    }
    ofstream hashFile(".mygit/staging/file_hashes.txt",ios::trunc);
    if(!hashFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Unable to open file hashes file.\n";
        return;
    }
    for(const auto &entry:fileHashes)
        hashFile<<entry.first<<" "<<entry.second<<"\n";
}

void saveStagedFiles(const unordered_set<string>&stagedFiles)
{
    if(!filesystem::exists(".mygit/staging"))
        filesystem::create_directories(".mygit/staging");
    if(!filesystem::exists(".mygit/staging/staged_files.txt"))
    {
        ofstream createFile(".mygit/staging/staged_files.txt");
        if(!createFile.is_open())
        {
            cout<<"\x1B[31m\nfatal: Unable to create staging file.\n";
            return;
        }
        createFile.close();
    }
    ofstream stagedFile(".mygit/staging/staged_files.txt",ios::trunc);
    if(!stagedFile.is_open())
    {
        cout<<"\x1B[31m\nfatal: Unable to open staging file.\n";
        return;
    }
    for(const auto& file:stagedFiles)
        stagedFile<<file<<"\n";
}

void recursiveAddToStaging(const filesystem::path &dirPath,unordered_set<string>&stagedFiles,unordered_map<string,string>&updateHashes)
{
    stringstream dirHashStream;
    for(const auto &entry:filesystem::recursive_directory_iterator(dirPath))
    {
        string relativePath=filesystem::relative(entry.path(),filesystem::current_path()).string();
        if(relativePath.find(".mygit")!=string::npos || entry.path().filename()=="mygit")
            continue;
        if(filesystem::is_regular_file(entry.path()))
        {
            string currentHash=computeFileHash(entry.path().string());
            dirHashStream<<currentHash;
            if(stagedFiles.find(relativePath)==stagedFiles.end())
            {
                unordered_map<string,string> fileHashes=loadFileHashes();
                if(fileHashes.find(relativePath)==fileHashes.end() || fileHashes[relativePath]!=currentHash)
                {
                    stagedFiles.insert(relativePath);
                    updateHashes[relativePath]=currentHash;
                    cout<<relativePath<<"\n";
                }
            }
        }
    }
    string dirHash=computeFileHash(dirHashStream.str());
    string relativeDirPath=filesystem::relative(dirPath,filesystem::current_path()).string();
    unordered_map<string,string> fileHashes=loadFileHashes();
    if(fileHashes[relativeDirPath]!=dirHash)
    {
        updateHashes[relativeDirPath]=dirHash;
        stagedFiles.insert(relativeDirPath);
        cout<<"Directory "<<relativeDirPath<<" staged due to modified content\n";
    }
}

void addFiles(const vector<string>&files)
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    ofstream indexFile;
    indexFile.open(".mygit/index",ios::trunc);
    if(files.size()==1 && files[0]==".")
        recTree(filesystem::current_path());
    else
    {
        for(const auto &file:files)
        {
            string relativePath=filesystem::relative(file,filesystem::current_path()).string();
            if(relativePath.find(".mygit")!=string::npos || relativePath=="mygit")
                continue;
            if(filesystem::is_regular_file(file))
            {
                string blobHash=createBlob(file);
                indexFile<<"100644 "<<file<<" "<<blobHash<<"\n";
            }
            else if(filesystem::is_directory(file))
            {
                recTree(file);
            }
        }
    }
    indexFile.close();
    unordered_map<string,string> fileHashes=loadFileHashes();
    unordered_map<string,string> updateHashes;
    unordered_set<string> stagedFiles=loadStagedFiles();
    cout<<"\x1B[93m\nModified files list:\n";
    if(files.size()==1 && files[0]==".")
        recursiveAddToStaging(filesystem::current_path(),stagedFiles,updateHashes);
    else
    {
        for(const auto &file:files)
        {
            string relativePath=filesystem::relative(file,filesystem::current_path()).string();
            if(filesystem::is_regular_file(file))
            {
                string currentHash=computeFileHash(file);
                if(stagedFiles.find(relativePath)==stagedFiles.end() && (fileHashes.find(relativePath)==fileHashes.end() || fileHashes[relativePath]!=currentHash))
                {
                    stagedFiles.insert(relativePath);
                    updateHashes[relativePath]=currentHash;
                    cout<<relativePath<<"\n";
                }
            }
            else if(filesystem::is_directory(file))
                recursiveAddToStaging(file,stagedFiles,updateHashes);
        }
    }
    saveStagedFiles(stagedFiles);
    saveFileHashes(updateHashes);
}