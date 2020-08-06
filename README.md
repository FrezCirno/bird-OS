## 警告

项目随时大修改, 本地备份最稳妥

## 怎么运行

### 构建
```
$ make build
```

### dump
```
$ make dump
```

### 清理
```
$ make clean
```

### 使用qemu调试
```
$ make qemu
```
会打开qemu以及gdb
然后在gdb交互式窗口中输入, 开启调试
```
target remote:1234
```

### 使用bochs调试
```
$ make bochs
```

### 默认操作 = clean build dump qemu
```
$ make
```