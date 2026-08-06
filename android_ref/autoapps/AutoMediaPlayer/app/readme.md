### <font color="#eeccaa">资源池添加注意</font><br/>
#### <font color="#00cccc">没有模块负责人同意 <font color="#ee3300">[ 禁止 ] </font> 添加新的资源 <font><br/>
```
Module Owner: youwj
Office Email: youwenjie@hcn2000.com
```

### <font color="#eeccaa">安装包编译说明</font><br/>
#### <font color="#00cccc">不同的平台编译在 <font color="#cc9933">[ platform.gradle ]</font> 配置</font><br/>
* <font color="#cccccc">点击</font> [ file: [platform.gradle](../config/platform.gradle) ] <font color="#cccccc">跳转到文件.</font><br/>
```
project.ext {
    // Configure build hardware platform
    // support platform {"ac8227l", "ac8257", "uis8581", "mt8163", "mt8321", "sun50iw9p1"}
    hardwareConfig="mt8163"
}
```

#### <font color="#00cccc">不同平台编译资源 <font color="#cc9933">[ config_res.gradle ]</font> 配置</font><br/>
* <font color="#cccccc">点击</font> [ file: [config_res.gradle](./config_res.gradle) ] <font color="#cccccc">跳转到文件.</font><br/>
```
common_resDirs
  定义所有平台的通用打包资源
 
mt8163_resDirs
  定义 mt8163 平台默认的打包资源

mt8321_resDirs
  定义 mt8321 平台默认的打包资源

...
```

#### <font color="#00cccc">编译警告问题: </font><font color="#cc9933">[ MissingDefaultResource ]</font><br/>
```
第一步删除 lint-baseline.xml 文件，并 Clean Project - Reload from Disk；
第二步执行 Make Module 'HMediaPlayer.app' （它会生成新的  int-baseline.xml 文件）
```

#### <font color="#00cccc">编译多媒体安装包 <font color="#cc9933">[ app-release.apk ] </font> 的方法 <font><br/>
```
第一步: Build -- Clean Project
第二步: Generate Signed Bundle or APK (选择 APK) -- Next -- Next -- 选择 automotiveRelease -- Finish
       签名文件在 ${roottDir}/security, 密码别名在 ${roottDir}/app/build.gradle 文件中
第三步: 生成完后去 ${roottDir}/output 目录拷贝目标安装包

参考定义：
    def roottDir = rootProject.getRootDir().getAbsolutePath();
```

#### <font color="#00cccc">平台安装包提交说明<font><br/>
```
MT8163:    
    git checkout MT2712AppGit/MT8163-P 
    
MT8321:
    git checkout MT2712AppGit/MT8321-P 
    
AC8257:
    git checkout MT2712AppGit/AC8257     
    
AC8227L:
    git checkout MT2712AppGit/AC8227L     
    
UIS8581:
    git checkout MT2712AppGit/RELEASE_COMMON     
    
sun50iw9p1:
    git checkout MT2712AppGit/RELEASE_T5
```