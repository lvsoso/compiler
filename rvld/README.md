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