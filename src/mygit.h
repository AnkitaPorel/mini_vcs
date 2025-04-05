#ifndef MYGIT_H
#define MYGIT_H

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_set>

void initRepo();
std::string hashObject(std::string &filename,bool writeBlob);
void catFile(std::string &flag,std::string &fileSha);
std::string writeTree();
std::string computeSha1(const std::string &data);
void lsTree(const std::string &treeSha,bool nameOnly);
void addFiles(const std::vector<std::string>& files);
void commitChanges(const std::string &message);
void showLog();
void checkout(const std::string &commitSha);
std::string hashSha1(std::ifstream &file,size_t fileSize);
std::string recTree(const std::filesystem::path &dirPath);
std::string createBlob(const std::string &filePath);
std::string getTreeHashFromCommit(const std::string &commitSha);
std::string getParentCommit();
std::string getLatestTreeHash();
void updateIndex(const std::string &entry);
std::unordered_set<std::string> loadStagedFiles();

#endif
