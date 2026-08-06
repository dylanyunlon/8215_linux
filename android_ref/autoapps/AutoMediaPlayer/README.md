### <font color="#eeccaa">HMediaPlayer </font><img src="docs/image/badge.svg" align="center" width="121" height="20"><br/>
<img src="docs/image/media.png" align="center" width="800" height="450"><br/>

### <font color="#eeccaa">媒体工程编译环境说明</font><br/>
#### <font color="#00cccc">1、工具要求</font><br/>
```
   Android Studio 最低版本：Android Studio 4.2.1
                  建议版本：Android Studio Dolphin 
   
   Android Gradle Plugin（APG）版本：7.3.1
   APG 版本对应的配置文件：rootProject.getRootDir()/build.gradle
                  
   Gradle 版本：7.4 
   Gradle 版本对应配置文件：gradle/wrapper/gradle-wrapper.properties
  
   APG 对应的 JAVA 版本：Java 11
```
#### <font color="#00cccc">2、更多配置要求说明</font><br/>
- 请参见官方网址: <br/>
  https://developer.android.google.cn/studio/releases/gradle-plugin?hl=zh-cn#updating-plugin

### <font color="#eeccaa">媒体工程修改编译提交说明</font><br/>

#### <font color="#00cccc">1、COMMON 分支只需要提交代码，无需提交 apk 文件;</font><br/>
```
   git fetch MT2712AppGit COMMON
   git checkout MT2712AppGit COMMON
   git pull MT2712AppGit COMMON
   git add ./${file-path}
   git commit -m "[fix][HMediaPlayer] ..."
   git push MT2712AppGit HEAD:refs/for/COMMON
```
#### <font color="#00cccc">2、修改已出货的代码，需要配置编译变量（后续会自动化）</font><br/>
- <font color="#cccccc">点击</font> [ file: [platform.gradle](buildConfig/platform.gradle) ] <font color="#cccccc">跳转到文件.</font><br/>
```
   e.g. MT8163/MT8321
        检查修改 platform.gradle 文件的配置
           hardwareConfig=mt8163
           hardwareConfig=mt8321
           
        然后点击执行 Sync Now 同步配置环境；
        
        最后编译 app 模块（Build -- Generate Signed Apk/签名文件在 security 下/用户名称和密码在 app/build.gradle 下）；
        如果平台无特殊要求，默认都使用 platform_8163.jks 签名文件就好；
        编译完成后，文件生成在 output/app-release.apk;
        
        注意：一定要验证 apk 的硬件平台配置是否配置正确；
             设置 -- 应用和通知 -- 应用信息 -- 音乐 -- 高级（版本）
             应用的版本号后会带平台信息，例如：1.2.20230510-mt8163
             
```
#### <font color="#00cccc">3、出货的修改需要提交到对应的分支</font><br/>
```
   MT8163 平台产品更新分支 MT2712AppGit/MT8163-P 
      git fetch MT2712AppGit MT8163-P
      git checkout MT2712AppGit MT8163-P
      git pull MT2712AppGit MT8163-P
      git add ./${file-path} 
      git commit -m "[fix][HMediaPlayer] ..."
      git push MT2712AppGit HEAD:refs/for/MT8163-P
   
   MT8321 平台产品更新分支 MT2712AppGit/MT8321-P 
      同上操作逻辑，只是分支不同；
```
#### <font color="#00cccc">4、如果要修改编译 app 的版本号</font><br/>
```
   打开 projectConfig.gradle 文件，修改如下参数就好
      appVersionCode = 2
      appVersionName = "1.3"
   记得点击执行 Sync Now 同步配置环境后再编译；
```

#### <font color="#00cccc">5、发布了稳定的版本，仓库需要打 TAG </font><br/>
```
   语法：git tag -a ${tagname} -m ${note message}
          -a 取 annotated 的首字母, 设置标签名
          -m 取 message 的首字母，带注解，用来注释标签的说明;
          
   tagname 格式: VER.2023.06.14.stable
```