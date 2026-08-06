### <font color="#00cccc">主题说明</font><br/> 
##### <font color="#eeccaa">这是一个和蓝色相关的皮肤包，主要以蓝色为基调搭配；</font><br/>

|    _资源名称_     | _主题颜色_ |    _资源说明_     | _责任人_ |
|:-------------:|:------:|:-------------:|------:|
|  blue-common  |   蓝白   |   蓝白通用皮肤资源    |   游文杰 |

### <font color="#00cccc">主题代号</font><br/>
> <font color="#ee00ee"> 注意：</font> <br/>
> HMediaPlayer 客户端只预留了 mcc400-mnc100 这套原始经典皮肤包，其它资源全部移动到 HMediaSkinX 仓库中；

```
mcc400-mnc100:
   uis8581 默认皮肤配置；
   资源组成
      res 
      res-compat/share/blue-common
      res-pool/mcc400 
      res-pool/mcc400-mnc100
```

### <font color="#00cccc">编译事项</font><br/>
```
主题编译配置: config/skinxConfig.gradle
   通过配置 skinxBlue01Name 的值可以实现皮肤包定制效果；
   
编译警告问题: [MissingDefaultResource]
   基准文件方式： 
       参见 build.gradle 中的定义，默认是不使用 lint-baseline.xml 规则的；
       ---------------------------------------------------
         lint {
             // If you want to generate baseline.xml
             // baseline = file("lint-baseline.xml")
         }
       ---------------------------------------------------
       你也可以放开它，直接编译通过，但是这样不是我向你们推荐的；
       第一步删除 lint-baseline.xml 文件，并 Clean Project - Reload from Disk；
       第二步执行 Make Module 'HMediaPlayer.skinx-creator' （它会生成新的  lint-baseline.xml 文件）

   欺骗编译方式（推荐）：
       具体请参考 res-compat/build/readme.md 文件说明
```

### <font color="#00cccc">皮肤包使用方法</font><br/>
```
扩展皮肤：
    skinx-media-blue01.apk
    把皮肤包拷贝到 apd/app/ 目录下，重启机器；

配置属性：
    adb shell
    setprop persist.sys.etheme_god 400
    setprop persist.sys.etheme_sub 100
    setprop persist.sys.media.skinx blue01
```

### <font color="#00cccc">皮肤包不生效问题排查</font><br/>
```
adb shell
// 查看路径
pm path com.hcn.media.skinx_blue01
package:/system/app/skinx-media-blue01/skinx-media-blue01.apk

如果没有安装路径说明包安装失败
  基于 DexClassLoader 的加载器必须是安装后才可以访问 packageInfo 对象
  基于 PathClassLoader 的加载器不需要安装也可以访问 packageInfo 对象

如果直接把皮肤包拷贝到 system/app，会安装失败（解压失败）
  ziparchive: Unable to open '/system/app/skinx-media-blue01/skinx-media-blue01.apk': Permission denied
```