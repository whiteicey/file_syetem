#include <iostream>
#include <cstdio>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <string.h>


//依据linux下stat函数，其中mode结构存取的文件类型
#define S_IFREG 0x0100000//普通文件
#define S_IFDIR 0x0040000//目录文件
#define S_IRUSR 0x00400 //文件所有者具可读取权限
#define S_IWUSR 0x00200
#define S_IXUSR 0x00100
#define S_IRGRP 0x00040
#define S_IWGRP 0x00020
#define S_IXGRP 0x00010
#define S_IROTH 0x00004
#define S_IWOTH 0x00002
#define S_IXOTH 0x00001
#define R_OK 0x04
#define W_OK 0x02
#define X_OK 0x01
#define CMD_N (sizeof(cmd_table) / sizeof(cmd_table[0]))
#define RECURSIVE 0x800
#define INODE_NUM 512
#define BLK_NUM 512
#define NOTDEFAULTDIR(name) (strcmp(name,".") != 0 && strcmp(name,"..") != 0)
#define get_page(ino,page) page >= 10 ? BLK[inode[ino].blk2].index[page-10] : inode[ino].blk1[page]
#define get_char(ino,page,offset) BLK[get_page(ino,page)].str[offset]

#define MAX_DIRLIST 20
using namespace std;

char* readline(char* str){
    printf("%s",str);
    char ch = getchar();
    if(ch != '\n') ungetc(ch,stdin);
    char* p = (char*)malloc(sizeof(char) * 100);
    fgets(p,100,stdin);
    int len = strlen(p);
    if(p[len-1] == '\n') p[len-1] = '\0';
    return p;
}
void add_history(char* p){
    return ;
}
static char* rl_gets(){
    static char* line_read = NULL;
    if(line_read){
        free(line_read);
        line_read = NULL;
    }
    line_read = readline("myshell>");
    if(line_read && *line_read){
        add_history(line_read);
    }
    return line_read;
}

void cmd_pwd(char*);
void cmd_ls(char*);
void cmd_mkdir(char*);
void cmd_cd(char*);
void cmd_rmdir(char*);
void cmd_su(char*);
void cmd_whoami(char*);
void cmd_useradd(char*);
void cmd_creat(char*);
void cmd_read(char*);
void cmd_write(char*);
void cmd_rm(char*);
void cmd_ln(char*);
void cmd_find(char*);
static struct{
    char* name;
    char* description;
    void (*handler)(char* args);
} cmd_table[] = {
    {"ls","show catalog items",cmd_ls},
    {"pwd","show current directory",cmd_pwd},
    {"cd","enter a directory",cmd_cd},
    {"rmdir","remove a directory",cmd_rmdir},
    {"mkdir","creat a directory",cmd_mkdir},
    {"su","change users",cmd_su},
    {"whoami","show current user name",cmd_whoami},
    {"useradd","add new user",cmd_useradd},
    {"creat","creat a new file",cmd_creat},
    {"read","read a file",cmd_read},
    {"write","write a file",cmd_write},
    {"rm","remove a file",cmd_rm},
    {"ln","hard link",cmd_ln},
    {"find","find a file",cmd_find}
};

typedef struct State{
    int st_ino;//节点号
    int st_mode;//文件类型
    int st_nlink;//硬连接数
    int st_size;//文件大小
    int st_uid;//文件所有者
    int st_gid;//文件所有者对应的组
    int st_blksize;//文件块大小
    int st_blocks;//文件块数量
    time_t st_atime;//文件最后被访问时间
    time_t st_mtime;//文件最后被修改时间
    time_t st_ctime;//文件状态被修改时间
}stat;

//目录
typedef struct Dirent
{
    int d_ino;
    char d_name[20];
}dirent;

//目录结构
typedef struct Dir
{
    char dir_name[20];
    int dir_list_size;
    dirent dir_list[20];
}dir;

//扩展目录结构
typedef struct Dirents{
    int dir_list_size;
    dirent dir_list[21];
}dirents;

//数据块 512字节
typedef struct Block{
    union{
        char str[512];//数据
        dir dentry;//目录
        int index[128];//索引结构
        dirents dentrys; //扩展目录结构
    };
}block;

//i节点 索引
typedef struct Inode
{
    stat i_stat;
    int blk1[10];//一级索引
    int blk2;//二级索引
}i_node;

struct User{
    char pw_name[10];
    char pw_password[10];
    int pw_uid;
    int pw_gid;
}user;

int workspace;
int idle_uid = 101;
i_node inode[INODE_NUM];
block BLK[BLK_NUM];
#define SET_BLK_FLAG(i) (blk_flag[i / 8] |= 0x1 << (i % 8))
#define LOOK_BLK_FLAG(i) (blk_flag[i / 8] & 0x1 << (i % 8) ? 1 : 0)
#define CLEAR_BLK_FLAG(i) (blk_flag[i / 8] ^= 0x1 << (i % 8))
char blk_flag[BLK_NUM / 8];

int creat_blk(){
    for(int i = 0; i<BLK_NUM; i++){
        if(LOOK_BLK_FLAG(i) == 0){
            SET_BLK_FLAG(i);
            memset(&BLK[i],0,sizeof(block));
            return i;
        }
    }
}

int add_blk_for_file(int ino){
    int num = creat_blk();
    if(num == -1){
        return -1;
    }
    //将st_blocks+1
    if(inode[ino].i_stat.st_blocks < 10){
        inode[ino].blk1[inode[ino].i_stat.st_blocks++] = num;
    } 
    else if(inode[ino].i_stat.st_blocks == 10 && inode[ino].i_stat.st_mode & S_IFDIR == 0){
        //文件一级索引已满，此时文件不能是目录
        inode[ino].blk2 = num;
        num = creat_blk();
        if((BLK[inode[ino].blk2].index[inode[ino].i_stat.st_blocks++ - 10] = num)== -1){
            return -1;
        }
    } 
    else {
        //此时需要将新分配的块加入到二级索引
        BLK[inode[ino].blk2].index[inode[ino].i_stat.st_blocks++ - 10] = num;
    }
    return num;
}

int access(int ino, int mode){
    if(user.pw_uid == 0){
        return 0;
    }
    if(user.pw_uid == inode[ino].i_stat.st_uid){
        mode*=0x100;
    }
    if(user.pw_uid == inode[ino].i_stat.st_gid){
        mode*=0x10;
    }
    if(inode[ino].i_stat.st_mode & mode != 0){
        return 0;
    }
    return -1;
}

// 查找ino所指向的目录中是否存在 name 名称的文件 成功返回文件ino,失败返回-1
int find_name_in_dir(int ino, char* name,int mode){
    //判断ino所指向的文件是否为目录
    if(inode[ino].i_stat.st_mode & S_IFDIR == 0){
        printf("Is not a DIR\n");
        return -1;
    }
    // 查找目录块
    int num = inode[ino].i_stat.st_blocks;
    if(num > 10){
        printf("没有实现二级索引");
    }
    for(int l = 0; l < num; l++){
        dir* d = &BLK[inode[ino].blk1[l]].dentry;
        for(int i = 0; i < d->dir_list_size; i++){
            if(mode & RECURSIVE){
                if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR && NOTDEFAULTDIR(d->dir_list[i].d_name)){
                    return find_name_in_dir(d->dir_list[i].d_ino,name,mode);
                }
            }
            if(strcmp(d->dir_list[i].d_name, name) == 0){
                return mode & RECURSIVE ? ino : d->dir_list[i].d_ino;
            }
        }
    }
    return -1;
}

int creat_stat(int ino, int mode){
    inode[ino].i_stat.st_ino = ino;
    inode[ino].i_stat.st_mode = mode | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP ;
    inode[ino].i_stat.st_nlink = 1;
    inode[ino].i_stat.st_size = 0;
    inode[ino].i_stat.st_uid = user.pw_uid;
    inode[ino].i_stat.st_gid = user.pw_gid;
    inode[ino].i_stat.st_ctime = inode[ino].i_stat.st_atime = inode[ino].i_stat.st_mtime = time(NULL);
    inode[ino].i_stat.st_blksize = 512;
    inode[ino].i_stat.st_blocks = 0;
    return 0;
}

int creat_ino(){
    for(int i = 0 ;i<INODE_NUM;i++){
        if(inode[i].i_stat.st_blocks <= 0){
            memset(&inode[i],0,sizeof(i_node));
            return i;
        }
    }
    return -1;
}

int creat_dirent(int fino,int ino,char* name){
    for(int i = 0; i < inode[fino].i_stat.st_blocks; i++){
        int num = inode[fino].blk1[i];
        if(BLK[num].dentry.dir_list_size != MAX_DIRLIST){
            int size = BLK[num].dentry.dir_list_size++;
            BLK[num].dentry.dir_list[size].d_ino = ino;
            strcpy(BLK[num].dentry.dir_list[size].d_name,name);
            return 0;
        }
    }
    //新分配一个BLK 并将dir装入
    if(inode[fino].i_stat.st_blocks == 10){//目录存储已满
        printf("无法新增目录项");
        return -1;
    }
    int num;
    if((num = add_blk_for_file(ino)) == -1){
        return -1;
    }
    int size = BLK[num].dentry.dir_list_size++;
    BLK[num].dentry.dir_list[size].d_ino = ino;
    strcpy(BLK[num].dentry.dir_list[size].d_name,name);
    return 0;
}


int creat_dir(int ino,int f_ino,char* dir_name){
    //为dir分配内存
    if(add_blk_for_file(ino) == -1){
        printf("无可用空间\n");
        return -1;
    }
    int num = inode[ino].blk1[0];
    dir* d = &BLK[num].dentry;
    //将名字保存在结构中
    strcpy(d->dir_name,dir_name);
    d->dir_list_size = 0;
    creat_dirent(ino,ino,".");
    creat_dirent(ino,f_ino,"..");
    return 0;
}

int creat(char* args,int mode){
    //检查当前用户在该文件夹内是否拥有写权限
    if(access(workspace,W_OK) != 0){
        printf("mkdir: access error\n");
        return 0;
    }
    //查找inode空项，若inode数组已满则返回
    int ino = creat_ino();
    if(ino == -1){
        printf("inode 数组已满\n");
        return -1;
    }
    // 设置文件stat相关信息
    if(creat_stat(ino, mode) == -1){
        return -1;
    }
    //创建目录数据
    if(mode == S_IFDIR) {
        if(creat_dir(ino,workspace,args) == -1){
            return -1;
        }
    } 
    else if(mode == S_IFREG) {
        //分配普通文件的空间，初始分配一个块
        if(add_blk_for_file(ino) == -1){
            printf("无可用空间\n");
            return -1;
        }
    }   
    //若不为根节点，在当前创建一个指向创建目录inode的条目
    if(ino != 0) {
        creat_dirent(workspace,ino,args);
    }
    return ino;
}

#define have_access(ino,mode,ch) (inode[ino].i_stat.st_mode & mode ? ch : '-')

void ls(char* args){
    if(args == NULL){
        for(int i = 0; i < inode[workspace].i_stat.st_blocks; i++){
            int num = inode[workspace].blk1[1];
            dir* d = &BLK[num].dentry;
            for(int j = 0; j < d->dir_list_size; j++){
                int ino = d->dir_list[j].d_ino;
                printf("%c%c%c%c%c%c%c%c%c%c",have_access(ino,S_IFDIR,'d'),have_access(ino,S_IRUSR,'r'),have_access(ino,S_IWUSR,'w'),have_access(ino,S_IXUSR,'x'),have_access(ino,S_IRGRP,'r'),have_access(ino,S_IWGRP,'w'),have_access(ino,S_IXGRP,'x'),have_access(ino,S_IROTH,'r'),have_access(ino,S_IWOTH,'w'),have_access(ino,S_IXOTH,'x'));
                printf("%s ",ctime(&inode[ino].i_stat.st_ctime));
                printf("%s\n",d->dir_list[j].d_name);
            }
        }
    }
}

void pwd(char* buf,int ino){
    if(ino == 0){
        strcpy(buf,"/");
        return;
    }
    dir* tmp = &BLK[inode[ino].blk1[0]].dentry;
    pwd(buf,tmp->dir_list[1].d_ino);
    strcpy(buf+strlen(buf),tmp->dir_name);
    if(ino!=workspace){
        strcpy(buf+strlen(buf),"/");
    }
    return;
}

int mkdir(char* dir_name,int mode){
    //printf("1\n");
    if(strcmp(dir_name,"/") != 0 && find_name_in_dir(workspace,dir_name,0) != -1){
        printf("mkdir: cannot create directory '%s': File exists",dir_name);
        return -1;
    }
    if(creat(dir_name,S_IFDIR) != 0){
        return -1;
    }
    return 0;
}

void creat_file(char* args){
    if(find_name_in_dir(workspace,args,0)!= -1){
        printf("creat: cannot create file '%s': File exists",args);
        return ;
    }
    creat(args,S_IFREG);
    return ;
}

int open_file(char* args){
    if(access(workspace,R_OK)!=0){
        printf("write: access r error\n");
        return -1;
    }
    int ino;
    ino = find_name_in_dir(workspace,args,0);
    if(ino == -1){
        printf("no file in dir\n");
        return -1;
    }
    if(inode[ino].i_stat.st_mode & S_IFREG == 0){
        printf("%s: Not a regfile\n",args);
        return -1;
    }
    return ino;
}

void cd(char* dir_name){
    int ino;
    if((ino = find_name_in_dir(workspace,dir_name,0)) == -1){//文件不存在
        printf("cd: %s: No such file or directory\n", dir_name);
        return;
    }
    if(inode[ino].i_stat.st_mode & S_IFDIR == 0){//选择的文件名不是目录
        printf("cd: %s: Not a directory\n", dir_name);
        return;
    }
    //当前用户对该目录没有读权限
    if(access(ino,R_OK) != 0){
        printf("cd: access error\n");
        return ;
    }
    workspace = ino;//工作目录转移
}


int find_user(char* args){
    if(*args == '\0'){
    return -1;
    }
    FILE* fp;
    if((fp = fopen("passwd","r")) == NULL){
        printf("No user data\n");
        return -1;
    }
    char tmp[100];
    while(fgets(tmp,100,fp) != NULL){
        char* p = strtok(tmp,":");
        if(strcmp(args,p) == 0){
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}


void useradd(char* args){
    if(idle_uid > 198){
        printf("用户数量已达到最大\n");
        return ;
    }
    //当前用户必须为root
    if(user.pw_uid != 0){
        printf("权限不足\n");
        return ;
    }
    //打开文件passwd
    FILE* fp;
    if((fp = fopen("passwd","r+")) == NULL){
        printf("No user data\n");
        return ;
    }
    //搜索文件查找用户是否重复
    if(find_user(args) == 0){
        printf("Username already exists\n");
        return ;
    }
    //在末尾添加用户
    if(fseek(fp,0,SEEK_END) != 0) {
        printf("fseek error\n");
        fclose(fp);
        return ;
    }
    printf("passwd>");
    char newpasswd[20];
    scanf("%s",newpasswd);
    fputs(args,fp);
    fputs(":",fp);
    fputs(newpasswd,fp);
    fputs(":",fp);
    sprintf(newpasswd,"%d",idle_uid);
    fputs(newpasswd,fp);
    fputs(":",fp);
    fputs("100\n",fp);
    fclose(fp);
}


int login(char* args){
    if(args == NULL){
        printf("login:");
        scanf("%s",user.pw_name);
    }
    else{
        strcpy(user.pw_name,args);
    }

    FILE* fp;
    if((fp = fopen("passwd","r")) == NULL){
        printf("no user data\n");
        fclose(fp);
        return -1;
    }
    if(fseek(fp,0,SEEK_SET)!=0){
        printf("fseek error\n");
        fclose(fp);
        return -1;
    }
    char tmp[100];
    while (fgets(tmp,100,fp) != NULL)
    {
        char* p = strtok(tmp,":");
        if (strcmp(user.pw_name,p) == 0)
        {
            printf("passwd:");
            scanf("%s",user.pw_password);
            p = strtok(NULL,":");
            if(strcmp(user.pw_password,p) == 0){
                p = strtok(NULL,":");
                user.pw_uid = atoi(p);
                p = strtok(NULL,":");
                user.pw_gid = atoi(p);
                fclose(fp);
                return 0;
            }
            else{
                printf("passwd error\n");
                fclose(fp);
                return -1;
            }
        }   
    }
    printf("no user\n");
    fclose(fp);
    return -1;
}

void cmd_ls(char* args){
    ls(NULL);
    return ;
}

void cmd_pwd(char* args){
    char str[100];
    pwd(str,workspace);
    printf("%s\n",str);
    return ;
}

void cmd_cd(char* args){
    //printf("0\n");
    if(*args == '\0'){
        printf("cd miss elements\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p){
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    cd(argv[0]);
    return ;
}

//删除指定目录结构中的第n项
int rm_dir_item(dir* d, int n){
    for(int i = n + 1;i < d->dir_list_size; i++){
        d->dir_list[i - 1].d_ino = d->dir_list[i].d_ino;
        strcpy(d->dir_list[i - 1].d_name,d->dir_list[i].d_name);
    }
    d->dir_list_size--;
    return 0;
}

#define get_blkn(ino,n) BLK[inode[ino].blk1[n]]

void rm(char* args, int wokeIno, int mode){
    int ino;
    ino = find_name_in_dir(workspace,args,0);
    if(ino == -1){
        printf("rmdir: %s:No such file or directory\n");
        return;
    }
    if(ino == 0){
        printf("can't remove root dir\n");
        return ;
    }
    if(inode[ino].i_stat.st_mode & mode == 0){
        printf("rmdir: %s: Not a dir\n");
        return ;
    }
    if(access(workspace,W_OK)!=0){
        printf("rmdir: %s :access error\n");
        return ;
    }
        if(--inode[ino].i_stat.st_nlink == 0){
        // 删除文件夹
        if(mode & S_IFDIR){
            dir* d2 = &BLK[inode[ino].blk1[0]].dentry;
            if(mode & RECURSIVE){
                //开始递归删除
                int num = inode[ino].i_stat.st_blocks;
                for(int l = 0; l < num; l++){
                    dir* d = &BLK[inode[ino].blk1[l]].dentry;
                    //遍历ino 各个块查找name
                    for(int i = 0; i < d->dir_list_size; i++){
                        if(strcmp(d->dir_list[i].d_name,".") == 0 || strcmp(d->dir_list[i].d_name,"..") == 0)
                            continue;
                        if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR){
                        //删除目录，递归删除
                            rm(d->dir_list[i].d_name,ino,mode);
                        } 
                        else {
                            rm(d->dir_list[i].d_name,ino,S_IFREG);
                        }
                    }   
                 }       
            }
            else if(d2->dir_list_size != 2 || inode[ino].i_stat.st_blocks != 1){
                //检查当前文件夹是否为空
                printf("rmdir: Directory not empty\n");
                return ;
            }
        }  
        //释放所有blk,先删除二级索引
        int blockNum = inode[ino].i_stat.st_blocks;
        if(blockNum > 10){
            for(int i = 0; i < blockNum - 10; i++){
                CLEAR_BLK_FLAG(BLK[inode[ino].blk2].index[i]);
            }
        }
        for(int i = 0; i < 10; i++){
            if(inode[ino].blk1[i] != 0){
            CLEAR_BLK_FLAG(inode[ino].blk1[i]);
            inode[ino].blk1[i] = 0;
        }
    }
    //将stat信息清空
    memset(&inode[ino].i_stat,0,sizeof(stat));
    }
    //从目录中一处待删除目录项
    for(int i = 0; i < inode[workspace].i_stat.st_blocks; i++){
        dir* d = &get_blkn(workspace,i).dentry;
        for(int j = 0; j < d->dir_list_size; j++){
            if(d->dir_list[j].d_ino == ino){
                rm_dir_item(d,j);
            }
        }
    }
    return ;
}

void rmdir(char* dir_name,int mode){
    if(mode & RECURSIVE){
        rm(dir_name,workspace,S_IFDIR|RECURSIVE);
        return ;
    }
    else{
        rm(dir_name,workspace,S_IFDIR);
        return ;
    }
}

void cmd_rmdir(char* args){
    if(*args == '\0'){
        printf("rmdir wrong : miss elements\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(strcmp(argv[0],"-r") == 0 && n == 2){
        rmdir(argv[1],RECURSIVE);
    }
    else{
        rmdir(argv[0],0);
    }
    return ;
}

void cmd_mkdir(char* args){
    if(*args == '\0'){
        printf("mkdir wrong : miss elements\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    //printf("0\n");
    mkdir(argv[0],0);
}


void cmd_su(char* args){
    login(args);
    return ;
}

void cmd_whoami(char* args){
    printf("%s\n",user.pw_name);
    return ;
}

void cmd_useradd(char* args){
    if(*args == '\0'){
        printf("useradd wrong : miss element\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    useradd(argv[0]);
    return ;
}

void cmd_creat(char* args){
    if(*args == '\0'){
        printf("creat wrong : miss element");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p){
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    creat_file(argv[0]);
    return ;
}

int read_file(char* argv[]){
    int ino = open_file(argv[0]);
    if(ino == -1){
        return -1;
    }
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    while(len--){
        int page = start / 512;
        if(start >= inode[ino].i_stat.st_size || get_char(ino,page,start - page * 512) == '\0'){
            printf(".");
        }
        else{
            printf("%c",get_char(ino,page,start - page * 512));
        }
        start++;
    }
    printf("\n");
    inode[ino].i_stat.st_atime = time(NULL);
    return 0;
}

void cmd_read(char* args){
    if(*args == '\0'){
        printf("no args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(n!=3){
        printf("args error : read [offset] [len]\n");
        return ;
    }
    read_file(argv);
    return ;
}

int write_file(char* argv[]){
    int ino = open_file(argv[0]);
    if(ino == -1){
        return -1;
    }
    if(access(ino,W_OK)!=0){
        printf("write: access w error\n");
        return -1;
    }
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    int page = (start + len) / 512 + 1;
    while(inode[ino].i_stat.st_blocks < page){
        if(add_blk_for_file(ino) == -1){
            printf("no spare space\n");
            return -1;
        }
    }
    if(start + len > inode[ino].i_stat.st_size){
        inode[ino].i_stat.st_size = start + len;
    }
    char* newdata = readline(">");
    int i = 0;
    while (len--)
    {
        page = start / 512;
        get_char(ino,page,start - page * 512) = newdata[i++];
        start++;
    }
    free(newdata);
    inode[ino].i_stat.st_atime = inode[ino].i_stat.st_mtime = time(NULL);
    return 0;
}

void cmd_write(char* args){
    if(*args == '\0'){
        printf("no args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(n!=3){
        printf("args error : write[offset] [len]\n");
        return ;
    }
    write_file(argv);
    return ;
}

void cmd_rm(char* args){
    if(*args == '\0'){
        printf("no args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    rm(argv[0],workspace,S_IFREG);
    return ;
}

int linkFile(char* argv[]){
    if(find_name_in_dir(workspace,argv[1],0)!=-1){
        printf("creat: cannot create file '%s': File exists",argv[1]);
        return -1;
    }
    int ino;
    ino = find_name_in_dir(workspace,argv[0],0);
    if(ino == -1){
        printf("no such in dir\n");
        return -1;
    }
    if(creat_dirent(workspace,ino,argv[1])!=-1){
        inode[ino].i_stat.st_nlink++;
        return 0;
    }
    return -1;
}
void cmd_ln(char* args){
    if (*args == '\0')
    {
        printf("no args\n");
        return ;
    }
    int n = 0;
    int i = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    linkFile(argv);
}

void findFile(char* args, int ino){
    int num = inode[ino].i_stat.st_blocks;
    for(int i = 0; i<num;i++){
        dir* d = &BLK[inode[ino].blk1[i]].dentry;
        for(int j = 0; j < d->dir_list_size;j++){
            if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR && NOTDEFAULTDIR(d->dir_list[i].d_name)){
                findFile(args,d->dir_list[i].d_ino);
            }
            if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFREG && strcmp(d->dir_list[i].d_name,args) == 0){
                char str[100];
                pwd(str,ino);
                printf("%s%s\n",str, args);
            }
        }
    }
}

void cmd_find(char* args){
    if (*args == '\0')
    {
        printf("no args\n");
        return ;
    }
    int n = 0;
    int i = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while (p)
    {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    findFile(argv[0],workspace);
}


int main(){
    printf("begin\n");
    workspace = 0;
    if(mkdir("/",0) != 0){
        printf("cannot creat a root\n");
        return 0;
    }
    while(login(NULL) != 0);
    while(1){
        char* str = rl_gets();
        char* cmd = strtok(str," ");
        if(cmd == NULL) continue;
        int i;
        for(i = 0; i < CMD_N; i++){
            if(strcmp(cmd,cmd_table[i].name) == 0){//找到命令
                char* args = cmd + strlen(cmd) + 1;
                cmd_table[i].handler(args);
                break;
            }
        }
    }
    return 0;
}