### notice

just copy from "https://github.com/ksco/rvld.git".

just for learn.

### env

```shell
# 拉取镜像
docker pull golang:1.19

# 安装依赖并重新commit镜像
docker run -u root -it --rm golang:1.19
apt update
apt install -y gcc-10-riscv64-linux-gnu qemu-user
ln -sf /usr/bin/riscv64-linux-gnu-gcc-10 /usr/bin/riscv64-linux-gnu-gcc
docker  ps | grep "golang:1.19" | awk '{ print $1}' | xargs -I {} docker commit {} golang:1.19-dev

# 使用新的镜像进行开发
docker run -u root -it --rm -v $PWD:/code/rvld -w /code/rvld golang:1.19-dev
```


### REFER
(一)
[https://en.wikipedia.org/wiki/Executable_and_Linkable_Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)
[https://fasterthanli.me/content/series/making-our-own-executable-packer/part-1/assets/elf64-file-header.bfa657ccd8ab3a7d.svg](https://fasterthanli.me/content/series/making-our-own-executable-packer/part-1/assets/elf64-file-header.bfa657ccd8ab3a7d.svg)

(二)
[https://en.wikipedia.org/wiki/Executable_and_Linkable_Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)
[https://refspecs.linuxbase.org/elf/gabi4+/ch4.sheader.html](https://refspecs.linuxbase.org/elf/gabi4+/ch4.sheader.html)


### Other's Reffer
[计算机原理系列之二 ——– 详解ELF文件](https://luomuxiaoxiao.com/?p=139)
[ELF 文件](https://ctf-wiki.org/executable/elf/structure/basic-info/)
[ELF文件格式第三讲，节头（section header)](https://www.cnblogs.com/AndroidBinary/p/15366466.html)
