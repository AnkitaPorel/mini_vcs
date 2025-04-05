#include "mygit.h"
#include <iostream>
#include <sstream>
#include <openssl/sha.h>
#include <iomanip>
#include <sys/stat.h>

using namespace std;

string hashSha1(ifstream &file,size_t fileSize)
{
    stringstream header;
    header<<"blob "<<fileSize<<'\0';
    SHA_CTX sha1Context;
    SHA1_Init(&sha1Context);
    string headerStr=header.str();
    SHA1_Update(&sha1Context,headerStr.c_str(),headerStr.size());
    size_t bufferSize=512*1024;
    char *buffer=new char[bufferSize];
    while(file.read(buffer,bufferSize))
        SHA1_Update(&sha1Context,buffer,file.gcount());
    SHA1_Update(&sha1Context,buffer,file.gcount());
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash,&sha1Context);
    delete[] buffer;
    stringstream ss;
    for(int i=0;i<SHA_DIGEST_LENGTH;i++)
        ss<<hex<<setw(2)<<setfill('0')<<(int)hash[i];
    return ss.str();
}

string hashObject(string &filename,bool writeBlob)
{
    const char* repoDir=".mygit";
    struct stat sb;
    if(stat(repoDir,&sb)!=0 || !S_ISDIR(sb.st_mode))
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: .mygit directory not found. Did you run 'mygit init'?\n";
        exit(1);
    }
    ifstream file(filename,ios::binary);
    if(!file.is_open())
    {
        cout<<"\x1B[31m";
        cout<<"\nfatal: Could not open file "<<filename<<"\n";
        exit(1);
    }
    size_t fileSize=filesystem::file_size(filename);
    file.seekg(0,ios::beg);
    string sha1=hashSha1(file,fileSize);
    if(writeBlob)
    {
        string objectDir=".mygit/objects/"+sha1.substr(0,2);
        string objectPath=objectDir+"/"+sha1.substr(2);
        filesystem::create_directory(objectDir);
        ofstream objectFile(objectPath,ios::binary);
        if(!objectFile.is_open())
        {
            cout<<"\x1B[31m";
            cout<<"\nfatal: Could not create blob file "<<objectPath<<"\n";
            exit(1);
        }
        file.clear();
        file.seekg(0,ios::beg);
        string header="blob "+to_string(fileSize)+'\0';
        objectFile.write(header.c_str(),header.size());
        if(objectFile.fail())
        {
            cout<<"\x1B[31m";
            cout<<"\nfatal: Could not write to blob file "<<objectPath<<"\n";
            exit(1);
        }
        objectFile<<file.rdbuf();
        objectFile.close();
    }
    return sha1;
}