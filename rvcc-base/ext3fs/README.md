```shell
创建块
dd bs=512 if=/dev/zero of=hd.img count=204800

设置成回环设备
sudo losetup /dev/loop1988 hd.img

格式化

sudo mkfs.ext3 -q /dev/loop1988  

挂载硬盘文件
sudo mount -o loop ./hd.img ./hdisk/
```