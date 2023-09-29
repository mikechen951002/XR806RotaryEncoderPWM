# 简介  
基于XR806 FreeRTOS SDK，测试旋转编码器，并实现PWM输出

# 使用方法  
先将源代码复制到 `~/tools/xr806_sdk/project/demo/tryre` 目录下，然后make
``` bash
$ cp project/demo/tryre/gcc/defconfig .config
$ make build -j
```
再将镜像 xr_system.img 下载到XR806即可
