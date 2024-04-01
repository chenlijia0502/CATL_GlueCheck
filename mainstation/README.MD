# 2020  软件主站开发框架

## 拉取方式
由于使用了git submodule功能，直接使用git clone并不会同时下载子模块，使用以下命令

`git clone git@192.168.1.151:base-development/framework.git --recursive`

如果忘记加上`--recursive`，使用以下命令下载子模块

`git submodule init && git submodule update`

## 开发流程

删除原本的远程仓库
`git remote remove origin`

更新远程子模块
`git submodule update --remote`

在gitlab创建新的仓库，添加新仓库的链接
```
git remote add origin 仓库地址
git add .
git commit -m "Initial commit"
git push -u origin master
```

##  子模块更新

### 当远程的子模块内容更新时，使用以下命令进行更新

`git submodule update --remote`

### 当远程添加了新的子模块时

1. 先拉取主仓库，合并。
   ```bash
    #状态：当前处于主分支master，且环境干净，否则进行一次commit操作
    #将远程的master创建到本地的一个新分支temp上
    git fetch origin master:temp
    #然后合并temp
    git merge temp
    #正常情况下不会冲突，否则解决冲突并提交修改
    #git commit -am "update base framework"
    #删除temp分支
    git branch -d temp
   ```
   此时会出现新的子模块名称的空白文档

2. 使用命令下载子模块

   `git submodule init && git submodule update`


## NOTE: 每次开发完完项目后记得在readme标注使用的子模块对应的版本号