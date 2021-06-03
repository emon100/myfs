#include <cstdlib>
#include <string>
#include <vector>
#include "my.h"
#include "syscall.h"

using std::string;
using std::vector;
static char path[] = "D:\\myfs.hd";

INUMBER currentDir;
vector<string> SplitString(const string& s, const string& c)
{
    vector<string> v;
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
    return v;
}

INUMBER chdir(INUMBER current,string path){
    INode *node = INumber2INode(current);
    if(node==NULL||node->type!=1){
        return -1;
    }
    vector<string> pathlist = SplitString(path, "/");
    Directory *no = pathlist[0].empty()?
                getDirectory(INumber2INode(getFSInfo()->root_inumber)->diskBlockId)
                :
                getDirectory(node->diskBlockCount);

    INUMBER tmp = -1;
    for(int i=1;i<(long long)pathlist.size();i++){
        tmp = find_in_directory(no,pathlist[i].c_str());
        node = INumber2INode(tmp);
        if(node&&node->type==1){
            no = getDirectory(node->diskBlockId);
        }else{
            return -1;
        }
    }
    return tmp;
}

int loadFileSystem(char *path,int32_t FS_SIZE){
    char *rawfs =  transient(path, FS_SIZE);
    if(rawfs == NULL){
        return -1;
    }
    setRawFs(rawfs);
    return 0;
}

int main(int argc, char** argv){
    const int32_t FS_SIZE = _128MB;
    //setRawFs(formatAndActivate(FS_SIZE,BLOCK_SIZE));
    //presistent(path, getRawFs(),FS_SIZE);
    loadFileSystem(path,FS_SIZE);

    FSInfo *fs = getFSInfo();
    printFSInfo(fs);
    INode *rootINode = INumber2INode(fs->root_inumber);
    printINodeInfo(rootINode);

    //放一个空文件a到根目录
    INUMBER inumber = alloc_empty_inode(
        0,
        0,
        0,
        0,
        0777,
        0
    );

    char buf[50] = "Hello world";
    write(inumber,0,buf,50);

    Directory *rootDir = getDirectory(rootINode->diskBlockId);

    const char * name = "a";
    int err = add_directory_entry(rootDir,name, inumber);


    //找到这个文件的inumber
    INUMBER file = find_in_directory(rootDir,name);
    char result[50] = {};
    read(file,0,result,50);

    printf("\n%s\n",result);


    //显示这个文件的INode
    INode *inodefile = INumber2INode(file);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    printINodeInfo(inodefile);
    printINodeInfo(rootINode);

    const char * dirname = "b";
    INUMBER B_Directory = make_directory(0);
    add_directory_entry(rootDir,dirname,B_Directory);
    INode *bnd = INumber2INode(B_Directory);

    INUMBER C_Directory = make_directory(B_Directory);
    add_directory_entry(getDirectory(bnd->diskBlockId),"c",C_Directory);

    //显示根文件夹
    printINodeInfo(rootINode);

    INUMBER b = chdir(0,"/b");
    printINodeInfo(INumber2INode(b));
    INUMBER c = chdir(0,"/b/c");
    printINodeInfo(INumber2INode(c));

    printf("%d",rmdir(rootDir,"/a"));
    printINodeInfo(rootINode);
    printf("%d",rm(rootDir,"/a"));
    printINodeInfo(rootINode);

    printINodeInfo(bnd);
    printf("%d",rmdir(rootDir,"b/c"));
    printINodeInfo(bnd);
}
/*
#include <vector>
#include <string>
#include <iostream>

using namespace std;

//记录当前登陆的账户userid
int64_t currentUserId;

//一个user
typedef struct user{
    int64_t userId;
    string username;
    string password;
    int64_t groupId;
    string groupname;
}user;

//全部user
vector<user> userList;

//显示全部user信息
void displayUsersInfo()
{
    for (auto iter = userList.begin(); iter != userList.end(); iter++)
    {
        cout<<"userId:"<<iter->userId<<endl;;
        cout<<"username:"<<iter->username<<endl;;
        cout<<"groupId:"<<iter->groupId<<endl;
        cout<<"groupname:"<<iter->groupname<<endl;
    }
}


//登录
void login()
{

    cout<<"LOGIN"<<endl;
    cout<<"Please input username:";
    string currentUserName;
    cin>>currentUserName;
    for (auto iter = userList.begin(); iter != userList.end(); iter++)
    {
        if(currentUserName == iter->username)
        {
            cout<<"Please input password:";
            string currentUserPassword;
            cin>>currentUserPassword;
            if(currentUserPassword == iter->password)
            {
                currentUserId=iter->userId;
                cout<<"Login success！"<<endl;
                return;
            }
            else
            {
                cout<<"Incorrect password！Please try again."<<endl;
                login();
                return;
            }
        }
    }
    cout<<"Invalid username.Please try again"<<endl;
    login();
}

void logout()
{
    if(currentUserId == -1)
    {
        cout<<"No login user."<<endl;
    }
    else
    {
        currentUserId = -1;
        cout<<"Logout success."<<endl;
    }
}

int main()
{
    ////////////////////////////////
    user user1;
    user1.userId=1;
    user1.username="emon";
    user1.password="emon";
    user1.groupId=1;
    user1.groupname="root";
    ////////////////////////////////
    userList.push_back(user1);
    login();
    logout();
    displayUsersInfo();
}
*/
