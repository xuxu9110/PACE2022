# PACE2022
## 编译
``g++ topo.h topo.cpp main.cpp -o main -std=c++17``
## 执行
``.\main {filepath}``  
若没有filepath则在命令行输入生成图
## 输入格式
第一行第一个整数为点的个数N  
接下来N行中的第i行表示从点i起始的边的终点编号，以空格分开  
点的编号为1~N  
带%开头的行将被视为注释而被忽略  
例：
> % some comment  
> 5  
> 2 3 4  
> 3 5  
>   
> 1 2  
> 4  
> % this is the end  
## 输出格式
一系列整数，表示找到的最小反馈集所包含的点编号
