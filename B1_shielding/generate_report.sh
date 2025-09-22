#!/bin/bash

# 报告生成脚本
echo "=== 中子伽马复合屏蔽玻璃模拟报告生成 ==="

# 检查是否在正确的目录
if [ ! -f "exampleB1.cc" ]; then
    echo "错误：请在B1_shielding目录下运行此脚本"
    exit 1
fi

# 检查可执行文件
if [ ! -f "build/exampleB1" ]; then
    echo "错误：找不到可执行文件 build/exampleB1"
    echo "请先编译项目："
    echo "  mkdir -p build && cd build && cmake .. && make -j8"
    exit 1
fi

# 创建报告输出目录
mkdir -p report_images

echo "--- 运行伽马源模拟 ---"
cd build
../build/exampleB1 ../gamma_shielding.mac
if [ $? -ne 0 ]; then
    echo "错误：伽马源模拟失败"
    exit 1
fi
cd ..

echo "--- 生成几何结构图 ---"
root -l -b -q geometry_plot.C
if [ $? -ne 0 ]; then
    echo "错误：几何图生成失败"
    exit 1
fi

echo "--- 生成数据分析图 ---"
root -l -b -q report_analysis.C
if [ $? -ne 0 ]; then
    echo "错误：数据分析图生成失败"
    exit 1
fi

echo "--- 生成统计摘要 ---"
echo "模拟完成时间: $(date)" > report_images/report_summary.txt
echo "伽马源: 241Am (59.5 keV)" >> report_images/report_summary.txt
echo "屏蔽材料: 复合屏蔽玻璃" >> report_images/report_summary.txt
echo "几何尺寸: 20cm x 20cm x 7.5cm" >> report_images/report_summary.txt

echo "=== 报告生成完成 ==="
echo "输出文件："
echo "  - report_images/geometry_plot.png/pdf"
echo "  - report_images/report_analysis.png/pdf"
echo "  - report_images/report_summary.txt"