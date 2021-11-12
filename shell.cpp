#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
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
#define MAX_DIRLIST 20
#define get_blkn(ino,n) BLK[inode[ino].blk1[n]]
#define RECURSIVE 0x800
#define INODE_NUM 512
#define BLK_NUM 512
#define CMD_N (sizeof(cmd_table) / sizeof(cmd_table[0]))
#define NOTDEFAULTDIR(name) (strcmp(name,".") != 0 && strcmp(name,"..") != 0)


char* creat_reg(int ino,int len);
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
//用户信息
struct User{
    char pw_name[10];
    char pw_passwd[20];
    int pw_uid;
    int pw_gid;
}user;
// 文件信息
typedef struct Stat{
    int st_ino;//节点号
    int st_mode;//文件类型
    int st_nlink;//硬链接数
    int st_size;//文件大小
    int st_uid;//文件所有者
    int st_gid;//文件所有者对应的组
    int st_blksize;//文件块大小
    int st_blocks;//文件块数量
    time_t st_atime;//文件最后被访问时间
    time_t st_mtime;//文件最后被修改时间
    time_t st_ctime;//文件状态改变时间
}stat;

// 目录项
typedef struct Dirent{
    int d_ino;
    char d_name[20];
}dirent;

// 目录结构 dir_list释放时需要逐条释放，否则会内存泄漏
typedef struct Dir{
    char dir_name[28];
    int dir_list_size;
    dirent dir_list[20];
}dir;

// 扩展目录结构
typedef struct Dirents{
    int dir_list_size;
    dirent dirlist[21];
}dirents;

//数据块结构 每个 512 字节
typedef struct Block{
    union{
        char str[512]; //数据
        dir dentry; //目录
        int index[128]; //索引结构
        dirents dentrys;//扩展目录结构
    };
}block;

//i节点
typedef struct Inode{
    stat i_stat;
    int blk1[10];// 10个一级索引
    int blk2;// 1个二级索引
}i_node;

//存放所有inode
i_node inode[INODE_NUM];
//存放当前目录ino
int workspace;
int idle_uid = 101;
block BLK[BLK_NUM];//全局块 每块512字节一共512*100 = 50K;
// int blk_flag[BLK_NUM];
#define SET_BLK_FLAG(i) (blk_flag[i / 8] |= 0x1 << (i % 8))
#define LOOK_BLK_FLAG(i) (blk_flag[i / 8] & 0x1 << (i % 8) ? 1 : 0)
#define CLEAR_BLK_FLAG(i) (blk_flag[i / 8] ^= 0x1 << (i % 8))
char blk_flag[BLK_NUM / 8];


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
// 读取输入字符
static char* rl_gets() {
    static char *line_read = NULL;
    if (line_read) {
        free(line_read);
        line_read = NULL;
    }
    line_read = readline("shell>");
    if (line_read && *line_read) {
        add_history(line_read);
    }
    return line_read;
}

//请求一个空闲块，返回请求到的空闲块号 请求失败返回-1
int creat_blk(){
    for(int i = 0; i < BLK_NUM; i++){
        if(LOOK_BLK_FLAG(i) == 0){
            SET_BLK_FLAG(i);//分配
            memset(&BLK[i],0,sizeof(block));//初始化块
            return i;
        }
    }
    return -1;
}

//为一个文件新增一个数据块
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

//权限检查,若有权限则返回0 若无相应权限则返回-1
int access(int ino,int mode){
    //若当前用户为root，直接返回0
    if(user.pw_uid == 0){
        return 0;
    }
    if(user.pw_uid == inode[ino].i_stat.st_uid){//是文件所有者
        mode *= 0x100;
    } 
    else if(user.pw_uid == inode[ino].i_stat.st_gid){//是组成员
        mode *= 0x10;
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
        //遍历ino 各个块查找name
        for(int i = 0; i < d->dir_list_size; i++){
            if(mode & RECURSIVE){
                //当前为递归查找模式
                if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR && NOTDEFAULTDIR(d->dir_list[i].d_name)){
                    return find_name_in_dir(d->dir_list[i].d_ino,name,mode);
                }
            }
            if(strcmp(d->dir_list[i].d_name, name) == 0){
                //当处于递归查找时，返回文件所在目录ino号
                return mode & RECURSIVE ? ino : d->dir_list[i].d_ino;
            }
        }
    }
    return -1;
}

/*------------------creat系列操作---------------------*/
//设置文件stat相关信息
int creat_stat(int ino, int mode){
    inode[ino].i_stat.st_ino = ino;
    inode[ino].i_stat.st_mode = mode | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |S_IXGRP;
    inode[ino].i_stat.st_nlink = 1;
    inode[ino].i_stat.st_size = 0;
    inode[ino].i_stat.st_uid = user.pw_uid;
    inode[ino].i_stat.st_gid = user.pw_gid;
    inode[ino].i_stat.st_ctime = inode[ino].i_stat.st_atime =
    inode[ino].i_stat.st_mtime = time(NULL);
    inode[ino].i_stat.st_blksize = 512;
    inode[ino].i_stat.st_blocks = 0;//创建时st_blocks默认设置为0
    return 0;
}


//在fino中创建指向ino的目录项
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

// 创建一个dir结构 并将本目录与父目录装入 返回分配的blk号
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

//分配一个ino,成功返回分配的ino，失败返回-1
int creat_ino(){
    for(int i = 0; i < INODE_NUM; i++){
        if(inode[i].i_stat.st_blocks <= 0){
        memset(&inode[i],0,sizeof(i_node));
        return i;
        }
    }
    return -1;
}

//创建文件
int creat(char* args,int mode){
    //检查当前用户在该文件夹内是否拥有写权限
    if(access(workspace,W_OK) != 0){
        printf("mkdir: access error\n");
        return 0;
    }
    //查找inode空项， 若inode数组已满则返回
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



/*------------------mkdir---------------*/
// 创建目录
int mkdir(char* dir_name,int mode){
    //检查当前目录中是否存在同名文件
    if(strcmp(dir_name,"/") != 0 && find_name_in_dir(workspace,dir_name,0) != -1){
        printf("mkdir: cannot create directory '%s': File exists",dir_name);
        return -1;
    }
    //创建目录
    if(creat(dir_name,S_IFDIR) != 0){
        return -1;
    }
    return 0;
}


void cmd_mkdir(char* args){
    //没有输入参数,返回
    if(*args == '\0'){
        printf("mkdir: missing operand\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    mkdir(argv[0],0);
}

/*-------------------------------ls-----------------------------*/
#define have_access(ino,mode,ch) (inode[ino].i_stat.st_mode & mode ? ch : '-')
// 查看目录信息(默认为当前目录)
void ls(char* args) {
    if(args == NULL){
    //遍历块
        for(int i = 0; i < inode[workspace].i_stat.st_blocks; i++){
            int num = inode[workspace].blk1[i];
            dir* d = &BLK[num].dentry;
            for(int j = 0; j < d->dir_list_size; j++){
                int ino = d->dir_list[j].d_ino;
                printf("%c%c%c%c%c%c%c%c%c%c",have_access(ino,S_IFDIR,'d'),have_access(ino,S_IRUSR,'r'),\
                    have_access(ino,S_IWUSR,'w'),have_access(ino,S_IXUSR,'x'),have_access(ino,S_IRGRP,'r'),\
                    have_access(ino,S_IWGRP,'w'),have_access(ino,S_IXGRP,'x'),have_access(ino,S_IROTH,'r'),\
                    have_access(ino,S_IWOTH,'w'),have_access(ino,S_IXOTH,'x'));
                printf("%s ",ctime(&inode[ino].i_stat.st_ctime));
                printf("%s\n",d->dir_list[j].d_name);
            }
        }
    }
}
void cmd_ls(char* args){
    ls(NULL);
}

/*------------------------------pwd-----------------------------*/
//pwd函数调用
void pwd(char* buf, int ino){
    if(ino == 0){
        strcpy(buf,"/");
        return;
    }
    dir* tmp = &BLK[inode[ino].blk1[0]].dentry;
    pwd(buf,tmp->dir_list[1].d_ino);
    strcpy(buf+strlen(buf),tmp->dir_name);
    if(ino != workspace){
        strcpy(buf+strlen(buf), "/");
    }
    return ;
}
void cmd_pwd(char* args){
    char str[100];
    pwd(str,workspace);
    printf("%s\n",str);
}


/*------------------------------------------cd----------------------------------------*/
//cd调用 跳转到目标文件夹
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
void cmd_cd(char* args){
    if(*args == '\0'){
       return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    cd(argv[0]);
}



/*-----------------------------------rm操作系列-------------------------------*/
//删除指定目录结构中的第n项
int rm_dir_item(dir* d, int n){
    for(int i = n + 1;i < d->dir_list_size; i++){
        d->dir_list[i - 1].d_ino = d->dir_list[i].d_ino;
        strcpy(d->dir_list[i - 1].d_name,d->dir_list[i].d_name);
    }
    d->dir_list_size--;
    return 0;
}

void rm(char* args,int workIno,int mode){
    int ino;
    //文件不存在
    if((ino = find_name_in_dir(workIno,args,0)) == -1){
        printf("rmdir: %s: No such file or directory\n",args);
        return;
    }
    //根目录不能删除
    if(ino == 0){
        printf("can not remove root\n");
        return;
    }
    //选择的文件名与mode不匹配
    if(inode[ino].i_stat.st_mode & mode == 0){
        printf("rmdir: %s: Not a directory\n",args);
        return;
    }
    //检查当前用户有没有执行权限
    if(access(workspace,W_OK) != 0){
        printf("rmdir: access error\n");
        return ;
    }
    if(--inode[ino].i_stat.st_nlink == 0){
        // 要删除的时文件夹
        if(mode & S_IFDIR){
            dir* d2 = &BLK[inode[ino].blk1[0]].dentry;
            if(mode & RECURSIVE){
                //直接开始递归删除
                int num = inode[ino].i_stat.st_blocks;
                for(int l = 0; l < num; l++){
                    dir* d = &BLK[inode[ino].blk1[l]].dentry;
                    //遍历ino 各个块查找name
                    for(int i = 0; i < d->dir_list_size; i++){
                        if(strcmp(d->dir_list[i].d_name,".") == 0 || strcmp(d->dir_list[i].d_name,"..") == 0)
                            continue;
                        if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR){
                        //当前待删除的是目录，递归删除
                            rm(d->dir_list[i].d_name,ino,mode);//进入到目录中删除其文件
                        } 
                        else {
                            rm(d->dir_list[i].d_name,ino,S_IFREG);
                        }
                    }   
                }       
            }
            else if(d2->dir_list_size != 2 || inode[ino].i_stat.st_blocks != 1){
                //检查当前文件夹是否为空，若不为空则不能删除
                printf("rmdir: Directory not empty\n");
                return ;
            }
        }   
        //释放所有blk
        //如果有二级索引则先删除二级索引
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

//rmdir调用，删除指定文件
void rmdir(char* dir_name,int mode){
    if(mode & RECURSIVE){
        rm(dir_name,workspace,S_IFDIR | RECURSIVE);
        return ;
    } 
    else {
        //递归删除整个目录
        rm(dir_name,workspace,S_IFDIR);
        return ;
    }
}

void cmd_rmdir(char* args){
    if(*args == '\0'){
        printf("rmdir: missing operand\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(strcmp(argv[0],"-r") == 0 && n == 2){
        //递归删除整个目录
        rmdir(argv[1],RECURSIVE);
    } 
    else {
        rmdir(argv[0],0);
    }
}

/*----------------------------------rm-------------------------------------*/


void cmd_rm(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    rm(argv[0],workspace,S_IFREG);
}


/*------------------------用户操作-----------------------*/
int login(char* args){
    if(args == NULL){
        printf("login:");
        scanf("%s",user.pw_name);
    }
    else{
        strcpy(user.pw_name,args);
    }
    //打开存放用户数据的文件passwd
    FILE* fp;
    if((fp = fopen("passwd","r")) == NULL){
        printf("No user data\n");
        return -1;
    }
    if(fseek(fp,0,SEEK_SET) != 0) {
        printf("fseek error\n");
        fclose(fp);
        return -1;
    }
    char tmp[100];
    while(fgets(tmp,100,fp) != NULL){
        char* p = strtok(tmp,":");
        if(strcmp(user.pw_name,p) == 0){
            printf("passwd:");
            scanf("%s",user.pw_passwd);
            p = strtok(NULL,":");
            if(strcmp(user.pw_passwd,p) == 0){//登录成功
                p = strtok(NULL,":");
                user.pw_uid = atoi(p);
                p = strtok(NULL,":");
                user.pw_gid = atoi(p);
                fclose(fp);
                return 0;
            } 
            else {
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

//查找用户
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
//添加用户
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

//重新登录
void cmd_su(char* args){
    login(args);
}

void cmd_whoami(char* args){
    printf("%s\n",user.pw_name);
    return ;
}

//只有root用户可以创建新用户
void cmd_useradd(char* args){
    //判断args是否为NULL
    if(*args == '\0'){
        printf("useradd: No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    useradd(argv[0]);
}



/*---------------------------------creat---------------------------------*/
//创建文件
void creat_file(char* args){
    //检查当前目录中是否存在同名文件
    if(find_name_in_dir(workspace,args,0) != -1){
        printf("creat: cannot create file '%s': File exists",args);
        return ;
    }
    creat(args,S_IFREG);
    return ;
}
// 打开文件，返回文件ino,出错返回-1
int open_file(char* args){
    //首先查看用户对该目录是否有读权限
    if(access(workspace,R_OK) != 0){
        printf("write: access r error\n");
        return -1;
    }
    //首先查找文件是否存在
    int ino;//文件的ino
    if((ino = find_name_in_dir(workspace,args,0)) == -1){
        printf("no file in dir\n");
        return -1;
    }
    //查看文件是否为reg类型
    if(inode[ino].i_stat.st_mode & S_IFREG == 0){
        printf("%s: Not a regfile\n", args);
        return -1;
    }
    return ino;
}

void cmd_creat(char* args){
    //检查args是否为空
    if(*args == '\0'){
        printf("creat: No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    creat_file(argv[0]);
}

/*----------------------------read-----------------------------*/
//获得page的实际页号
#define get_page(ino,page) page >= 10 ? BLK[inode[ino].blk2].index[page-10] : inode[ino].blk1[page]
#define get_char(ino,page,offset) BLK[get_page(ino,page)].str[offset]
int read_file(int argc,char* argv[]){
    //打开文件
    int ino = open_file(argv[0]);
    if(ino == -1){
        return -1;
    }
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    while(len--){
        //通过起始数得出需要访问的起始页
        int page = start / 512;
        if(start >= inode[ino].i_stat.st_size || get_char(ino,page,start - page * 512) == '\0'){
            printf("%c",'.');
        } 
        else {
            printf("%c",get_char(ino,page,start - page * 512));
        }
        start++;
    }
    printf("\n");
    inode[ino].i_stat.st_atime = time(NULL);
    return 0;
}
void cmd_read(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(n != 3) {
        printf("args error:read [offset] [len]\n");
        return;
    }
    read_file(n,argv);
}



/*--------------------------------write--------------------------------*/
//写入文件
int write_file(int argc,char* argv[]){
    int ino = open_file(argv[0]);
    if(ino == -1){
        return -1;
    }
    //查看文件写权限
    if(access(ino,W_OK) != 0){
        printf("write: access w error\n");
        return -1;
    }
    //检查文件空间是否够用
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    int page = (start + len) / 512 + 1;
    //为文件分配空间
    while(inode[ino].i_stat.st_blocks < page) {
        if(add_blk_for_file(ino) == -1){
            printf("存储空间已满");
            return -1;
        }
    }
    //更改文件长度
    if(start + len > inode[ino].i_stat.st_size){
        inode[ino].i_stat.st_size = start + len;
    }
    char* newdata = readline("input:");
    // 写入文件
    int i = 0;
    while(len--){
        //通过起始数得出需要访问的起始页
        page = start / 512;
        get_char(ino,page,start - page * 512) = newdata[i++];
        start++;
    }
    free(newdata);
    //更改stat中的时间
    inode[ino].i_stat.st_atime = inode[ino].i_stat.st_mtime = time(NULL);
    return 0;
}
void cmd_write(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(n > 3) {
        printf("args error:write [offset] [len]\n");
        return;
    }
    write_file(n,argv);
}


/*-----------------------------------link-----------------------*/
int linkFile(int argc,char* argv[]){
    //检查当前目录中是否存在同名文件
    if(find_name_in_dir(workspace,argv[1],0) != -1){
        printf("creat: cannot create file '%s': File exists",argv[1]);
        return -1;
    }
    //首先查找文件是否存在
    int ino;//文件的ino
    if((ino = find_name_in_dir(workspace,argv[0],0)) == -1){
        printf("no file in dir\n");
        return -1;
    }
    //添加一个目录项
    if(creat_dirent(workspace,ino,argv[1]) != -1){
        //为ino添加一个硬链接
        inode[ino].i_stat.st_nlink++;
        return 0;
    }
    return -1;
}

void cmd_ln(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    int i = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    linkFile(n,argv);
}


/*---------------------------find-------------------*/
//查找操作
void findFile(char* args,int ino){
    int num = inode[ino].i_stat.st_blocks;
    for(int l = 0; l < num; l++){
        dir* d = &BLK[inode[ino].blk1[l]].dentry;
        //遍历ino 各个块查找name
        for(int i = 0; i < d->dir_list_size; i++){
            if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR &&
                NOTDEFAULTDIR(d->dir_list[i].d_name)){
                findFile(args,d->dir_list[i].d_ino);
            }
            if(inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFREG && strcmp(d->dir_list[i].d_name,args) == 0){
                char str[100];
                pwd(str,ino);
                printf("%s%s\n",str,args);
            }
        }
    }
}

void cmd_find(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    int i = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    findFile(argv[0],workspace);
}



int main(){
    printf("file system is running\n");
    //创建根节点
    workspace = 0;
    if(mkdir("/",0) != 0){
        printf("cannot creat a root\n");
        return 0;
    }
    //根目录创建成功，开始登陆
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
}

