# PACE2022
本算法来自团队Hust_Huawei的成员Junzhou Xu，为PACE2022的参赛之作。（[PACE2022官网](https://pacechallenge.org/2022/)）  
The algorithm made by Junzhou Xu in team Hust_Huawei is a submission for PACE2022. ([PACE2022 Official Website](https://pacechallenge.org/2022/))  

## 算法说明 Algorithm Description
6月15日前补充pdf链接。  
Add pdf link by June 15.  
  
## 编译 Compilation
``g++ single\main.cpp -o Huawei_TCS_DFVS_Solver -std=c++17 -O3``  
编译生成符合[optil](https://www.optil.io/optilion/problem/3198)要求的程序。  
Generate program that meet the requirements of [optil](https://www.optil.io/optilion/problem/3198).  
  
``g++ *.h *.cpp -o main -std=c++17 -O3``  
编译生成适合experiment.py实验的程序。  
Generate program that suit experiment.py.  
  
## 执行 Execution
``.\Huawei_TCS_DFVS_Solver``  
接受命令行输入，将结果输出至命令行中。  
Take input from command lines and output to command lines.  
  
``.\main {filepath} {outputName}``  
结果输出至`result`文件夹，统计数据输出至`statistic`文件夹；若没有`{filepath}`则在命令行输入生成图。  
Take input from `{filepath}` (or command line if no `{filepath}`). Output result in `.\result` and statistic in `.\statistic`.  
  
``python3 .\experiment.py {datapath} {outfile}``  
对`{datapath}`下的所有图进行实验；`{datapath}`默认为`data`，`{outfile}`默认为`out.csv`。  
Doing experiments to all graphs in `{datapath}`. `{datapath}` defaults to `data` and `{outfile}` defaults to `out.csv`.  
  
## 输入格式 Input Format
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
  
## 输出格式 Output Format
一系列整数，表示找到的最小反馈集所包含的点编号
