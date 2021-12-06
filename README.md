# file_syetem

北方工业大学操作系统实验3

  

此实验基于模拟linux文件系统实现了

## 主要功能和行为包括：

1.	用户：创建，登录，查询

2. 文件：创建，删除，读写，查找，硬链接，支持多级目录

3. 文件操作接口均考虑了操作权限。


## 技术方案概览：

系统环境：跨平台，已在Mac（Darwin KKdeMacBook-Pro.local 20.6.0 Darwin Kernel Version 20.6.0: Wed Jun 23 00:26:31 PDT 2021; root:xnu-7195.141.2~5/RELEASE_X86_64 x86_64）和Windows 10家庭版（21H1,19043.1348, Windows Feature Experience Pack 120.2212.3920.0）平台测试。下文所述版本号为“Mac上的已测试版本号/Win10上的已测试版本号”格式。

语言：C语言，CMake，doxygen

开发环境：VSCode 1.62.2

已测试的工具版本：

CMake：3.19.4/3.20.1

GNU make：3.81（仅Mac）

<Nmake和Cl工具集：14.27.29112.0版本可执行文件，14.27.29110版本支持库，Windows SDK 10.0.18362.0版本（仅Win10）

doxygen：1.9.1/1.9.2（后者为自行编译版本）

clang：Apple clang version 13.0.0 (clang-1300.0.29.3)（仅Mac）

pdflatex：MiKTeX-pdfTeX 4.0.1 (MiKTeX 20.6.29)（细节省略。仅Win10，Mac上存在字体缺失，不能正确生成文档）

texworks：0.6.5 (MiKTeX 20.6.29) [r.649699a0, 2020/3/26 2:49]（仅Win10，Mac上未能正确生成文档）

## 程序构建流程：

新建build目录，切换工作目录至其中（CMake脚本禁止源码树内构建）

CMake <源码目录>，或者使用CMake-GUI工具

执行make（Mac下）或nmake（Win10下）


## 文档构建流程：

文档已生成

# 文件结构
.  
├── README.md 
├── cmjfs  
│   ├── CMakeLists.txt 
│   ├── doxygen_config 
│   ├── include 
│   │   ├── config.h  
│   │   ├── doc.h  
│   │   ├── shell  
│   │   │   ├── cmd.h  
│   │   │   ├── io.h  
│   │   │   └── shell.h  
│   │   └── sys  
│   │       ├── fs 
│   │       │   ├── fsops.h  
│   │       │   ├── mediactrl.h  
│   │       │   └── types 
│   │       │       ├── block.h  
│   │       │       ├── dir.h  
│   │       │       ├── dirent.h  
│   │       │       ├── dirents.h  
│   │       │       ├── inode.h  
│   │       │       └── stat.h  
│   │       ├── permission.h  
│   │       └── user.h  
│   └── src  
│       ├── main.c  
│       ├── shell  
│       │   ├── cmd.c  
│       │   ├── io.c  
│       │   └── shell.c 
│       └── sys 
│           ├── fs 
│           │   ├── fsops.c  
│           │   └── mediactrl.c  
│           ├── permission.c  
│           └── user.c 
└── 设计文档.pdf 