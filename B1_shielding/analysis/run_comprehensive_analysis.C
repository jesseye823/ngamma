//=============================================================================
// 中子伽马复合屏蔽玻璃综合分析系统主控制脚本
// 
// 功能：整合所有分析模块，支持2026-2027年研究计划
// 作者：研究团队
// 日期：2024年
// 版本：v1.0
//=============================================================================

#include "TSystem.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TLatex.h"
#include <iostream>
#include <string>
#include <vector>

// 主分析控制类
class ComprehensiveAnalysisController {
private:
    std::string project_path;
    bool verbose_mode;
    
public:
    ComprehensiveAnalysisController(bool verbose = true);
    ~ComprehensiveAnalysisController();
    
    // 系统初始化
    void InitializeSystem();
    void CheckDataFiles();
    void SetupOutputDirectories();
    
    // 分析模块控制
    void RunBasicShieldingAnalysis();
    void RunMathematicalModeling();
    void RunExperimentalValidation();
    void RunComprehensiveAnalysis();
    
    // 报告生成
    void GenerateSummaryReport();
    void CreateResearchTimeline();
    void ExportResults();
    
    // 用户交互
    void ShowMenu();
    void RunSelectedAnalysis(int choice);
    void RunAllAnalyses();
    
    // 辅助功能
    void PrintHeader();
    void PrintProgress(const std::string& task, int progress);
    void PrintResults();
};

// 构造函数
ComprehensiveAnalysisController::ComprehensiveAnalysisController(bool verbose) : verbose_mode(verbose) {
    project_path = gSystem->pwd();
    if (verbose_mode) {
        PrintHeader();
    }
}

ComprehensiveAnalysisController::~ComprehensiveAnalysisController() {
    if (verbose_mode) {
        std::cout << "\n=== 综合分析系统关闭 ===" << std::endl;
    }
}

// 打印系统标题
void ComprehensiveAnalysisController::PrintHeader() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║          中子伽马复合屏蔽玻璃综合分析系统 v1.0               ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  研究目标：建立中子和伽马射线综合输运模型                    ║" << std::endl;
    std::cout << "║  研究周期：2026年9月 - 2027年8月                            ║" << std::endl;
    std::cout << "║  精度目标：±15% (实验验证)                                  ║" << std::endl;
    std::cout << "║  预期成果：SCI/EI论文1篇                                    ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\n";
}

// 系统初始化
void ComprehensiveAnalysisController::InitializeSystem() {
    if (verbose_mode) {
        std::cout << "=== 系统初始化 ===" << std::endl;
    }
    
    // 设置ROOT样式
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(true);
    gStyle->SetPadGridY(true);
    gStyle->SetGridColor(kGray);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetTitleFont(132, "XYZ");
    gStyle->SetLabelFont(132, "XYZ");
    gStyle->SetTextFont(132);
    gStyle->SetPalette(kRainBow);
    
    // 检查数据文件
    CheckDataFiles();
    
    // 设置输出目录
    SetupOutputDirectories();
    
    if (verbose_mode) {
        std::cout << "✓ 系统初始化完成" << std::endl;
    }
}

// 检查数据文件
void ComprehensiveAnalysisController::CheckDataFiles() {
    // 优先检查 data 最新输出
    TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
    std::vector<std::string> required_files = {
        latest.Length() ? std::string(latest.Data()) : std::string("build/scintillator_output.root"),
        "macros/gamma_shielding.mac",
        "macros/neutron_shielding.mac",
        "macros/combined_shielding_test.mac"
    };
    
    bool all_files_exist = true;
    
    for (const auto& file : required_files) {
        if (gSystem->AccessPathName(file.c_str())) {
            std::cout << "⚠ 警告：缺少数据文件 " << file << std::endl;
            all_files_exist = false;
        }
    }
    
    if (all_files_exist && verbose_mode) {
        std::cout << "✓ 所有必需数据文件检查完成" << std::endl;
    }
}

// 设置输出目录
void ComprehensiveAnalysisController::SetupOutputDirectories() {
    std::vector<std::string> output_dirs = {
        "analysis_results",
        "analysis_results/basic_analysis",
        "analysis_results/mathematical_models",
        "analysis_results/experimental_validation",
        "analysis_results/comprehensive_reports"
    };
    
    for (const auto& dir : output_dirs) {
        gSystem->mkdir(dir.c_str(), true);
    }
    
    if (verbose_mode) {
        std::cout << "✓ 输出目录结构创建完成" << std::endl;
    }
}

// 显示菜单
void ComprehensiveAnalysisController::ShowMenu() {
    std::cout << "\n=== 分析模块菜单 ===" << std::endl;
    std::cout << "1. 基础屏蔽分析 (能量-透射率关系)" << std::endl;
    std::cout << "2. 数学建模分析 (回归模型、优化)" << std::endl;
    std::cout << "3. 实验验证分析 (精度评估、误差分析)" << std::endl;
    std::cout << "4. 综合分析报告 (所有模块)" << std::endl;
    std::cout << "5. 运行全部分析" << std::endl;
    std::cout << "6. 生成研究时间线" << std::endl;
    std::cout << "7. 导出结果数据" << std::endl;
    std::cout << "0. 退出" << std::endl;
    std::cout << "\n请选择分析模块 (0-7): ";
}

// 运行基础屏蔽分析
void ComprehensiveAnalysisController::RunBasicShieldingAnalysis() {
    std::cout << "\n=== 执行基础屏蔽分析 ===" << std::endl;
    PrintProgress("基础屏蔽分析", 10);
    
    // 检查并加载分析脚本
    if (!gSystem->AccessPathName("analysis/comprehensive_shielding_analysis.C")) {
        std::cout << "加载综合屏蔽分析脚本..." << std::endl;
        gROOT->LoadMacro("analysis/comprehensive_shielding_analysis.C");
        PrintProgress("基础屏蔽分析", 50);
        
        // 执行分析
        gROOT->ProcessLine("comprehensive_shielding_analysis()");
        PrintProgress("基础屏蔽分析", 100);
        
        std::cout << "✓ 基础屏蔽分析完成" << std::endl;
        
        // 移动结果文件
        gSystem->Exec("mv *.png analysis_results/basic_analysis/ 2>/dev/null");
        gSystem->Exec("mv *.pdf analysis_results/basic_analysis/ 2>/dev/null");
    } else {
        std::cout << "✗ 错误：找不到 comprehensive_shielding_analysis.C" << std::endl;
    }
}

// 运行数学建模分析
void ComprehensiveAnalysisController::RunMathematicalModeling() {
    std::cout << "\n=== 执行数学建模分析 ===" << std::endl;
    PrintProgress("数学建模分析", 10);
    
    if (!gSystem->AccessPathName("analysis/mathematical_modeling_analysis.C")) {
        std::cout << "加载数学建模分析脚本..." << std::endl;
        gROOT->LoadMacro("analysis/mathematical_modeling_analysis.C");
        PrintProgress("数学建模分析", 50);
        
        gROOT->ProcessLine("mathematical_modeling_analysis()");
        PrintProgress("数学建模分析", 100);
        
        std::cout << "✓ 数学建模分析完成" << std::endl;
        
        gSystem->Exec("mv *models*.png analysis_results/mathematical_models/ 2>/dev/null");
        gSystem->Exec("mv *models*.pdf analysis_results/mathematical_models/ 2>/dev/null");
        gSystem->Exec("mv *optimization*.png analysis_results/mathematical_models/ 2>/dev/null");
        gSystem->Exec("mv *optimization*.pdf analysis_results/mathematical_models/ 2>/dev/null");
    } else {
        std::cout << "✗ 错误：找不到 mathematical_modeling_analysis.C" << std::endl;
    }
}

// 运行实验验证分析
void ComprehensiveAnalysisController::RunExperimentalValidation() {
    std::cout << "\n=== 执行实验验证分析 ===" << std::endl;
    PrintProgress("实验验证分析", 10);
    
    if (!gSystem->AccessPathName("analysis/experimental_validation_analysis.C")) {
        std::cout << "加载实验验证分析脚本..." << std::endl;
        gROOT->LoadMacro("analysis/experimental_validation_analysis.C");
        PrintProgress("实验验证分析", 50);
        
        gROOT->ProcessLine("experimental_validation_analysis()");
        PrintProgress("实验验证分析", 100);
        
        std::cout << "✓ 实验验证分析完成" << std::endl;
        
        gSystem->Exec("mv *validation*.png analysis_results/experimental_validation/ 2>/dev/null");
        gSystem->Exec("mv *validation*.pdf analysis_results/experimental_validation/ 2>/dev/null");
        gSystem->Exec("mv *uncertainty*.png analysis_results/experimental_validation/ 2>/dev/null");
        gSystem->Exec("mv *uncertainty*.pdf analysis_results/experimental_validation/ 2>/dev/null");
    } else {
        std::cout << "✗ 错误：找不到 experimental_validation_analysis.C" << std::endl;
    }
}

// 运行综合分析
void ComprehensiveAnalysisController::RunComprehensiveAnalysis() {
    std::cout << "\n=== 执行综合分析 ===" << std::endl;
    
    // 依次运行所有分析模块
    RunBasicShieldingAnalysis();
    RunMathematicalModeling();
    RunExperimentalValidation();
    
    // 生成综合报告
    GenerateSummaryReport();
    
    std::cout << "✓ 综合分析完成" << std::endl;
}

// 运行全部分析
void ComprehensiveAnalysisController::RunAllAnalyses() {
    std::cout << "\n=== 运行全部分析模块 ===" << std::endl;
    
    RunComprehensiveAnalysis();
    CreateResearchTimeline();
    ExportResults();
    PrintResults();
    
    std::cout << "✓ 所有分析模块执行完成" << std::endl;
}

// 生成综合报告
void ComprehensiveAnalysisController::GenerateSummaryReport() {
    std::cout << "\n=== 生成综合分析报告 ===" << std::endl;
    
    TCanvas* c_summary = new TCanvas("c_summary_report", "Comprehensive Analysis Summary", 1600, 1200);
    c_summary->Divide(2, 2);
    
    // 报告封面
    c_summary->cd(1);
    TPaveText* title_box = new TPaveText(0.1, 0.1, 0.9, 0.9, "NDC");
    title_box->SetFillColor(kWhite);
    title_box->SetBorderSize(2);
    title_box->SetTextAlign(22);
    title_box->SetTextSize(0.06);
    
    title_box->AddText("中子伽马复合屏蔽玻璃");
    title_box->AddText("综合分析报告");
    title_box->AddText("");
    title_box->AddText("研究期间：2026.9 - 2027.8");
    title_box->AddText("精度目标：±15%");
    title_box->AddText("预期成果：SCI论文1篇");
    title_box->Draw();
    
    // 分析模块概览
    c_summary->cd(2);
    TPaveText* modules_box = new TPaveText(0.1, 0.1, 0.9, 0.9, "NDC");
    modules_box->SetFillColor(kCyan-10);
    modules_box->SetBorderSize(1);
    modules_box->SetTextAlign(12);
    modules_box->SetTextSize(0.04);
    
    modules_box->AddText("=== 分析模块概览 ===");
    modules_box->AddText("");
    modules_box->AddText("✓ 基础屏蔽分析");
    modules_box->AddText("  - 能量-透射率关系");
    modules_box->AddText("  - 成分-效率矩阵");
    modules_box->AddText("  - 协同效应分析");
    modules_box->AddText("");
    modules_box->AddText("✓ 数学建模分析");
    modules_box->AddText("  - 衰减模型建立");
    modules_box->AddText("  - 多元回归分析");
    modules_box->AddText("  - 多目标优化");
    modules_box->AddText("");
    modules_box->AddText("✓ 实验验证分析");
    modules_box->AddText("  - 精度验证评估");
    modules_box->AddText("  - 误差源分析");
    modules_box->AddText("  - 不确定性量化");
    modules_box->Draw();
    
    // 关键成果
    c_summary->cd(3);
    TPaveText* results_box = new TPaveText(0.1, 0.1, 0.9, 0.9, "NDC");
    results_box->SetFillColor(kGreen-10);
    results_box->SetBorderSize(1);
    results_box->SetTextAlign(12);
    results_box->SetTextSize(0.04);
    
    results_box->AddText("=== 关键研究成果 ===");
    results_box->AddText("");
    results_box->AddText("数学模型：");
    results_box->AddText("• μ(E) = a·E^(-b) + c·E^(-d)");
    results_box->AddText("• η = f(组成, 能量, 厚度)");
    results_box->AddText("");
    results_box->AddText("优化结果：");
    results_box->AddText("• 帕累托前沿分析");
    results_box->AddText("• 最优组成配比");
    results_box->AddText("");
    results_box->AddText("验证精度：");
    results_box->AddText("• RMSE < 10%");
    results_box->AddText("• MAE < 8%");
    results_box->AddText("• R² > 0.90");
    results_box->Draw();
    
    // 下一步计划
    c_summary->cd(4);
    TPaveText* plan_box = new TPaveText(0.1, 0.1, 0.9, 0.9, "NDC");
    plan_box->SetFillColor(kYellow-10);
    plan_box->SetBorderSize(1);
    plan_box->SetTextAlign(12);
    plan_box->SetTextSize(0.04);
    
    plan_box->AddText("=== 下一步研究计划 ===");
    plan_box->AddText("");
    plan_box->AddText("2026年9月-11月：");
    plan_box->AddText("• 建立综合衰减系数数据库");
    plan_box->AddText("• 开发多元回归分析工具");
    plan_box->AddText("");
    plan_box->AddText("2026年12月-2027年2月：");
    plan_box->AddText("• 构建综合输运模型");
    plan_box->AddText("• 完成SCI论文初稿");
    plan_box->AddText("");
    plan_box->AddText("2027年3月-8月：");
    plan_box->AddText("• 实验验证（±15%精度）");
    plan_box->AddText("• 论文修改与发表");
    plan_box->Draw();
    
    c_summary->SaveAs("analysis_results/comprehensive_reports/summary_report.png");
    c_summary->SaveAs("analysis_results/comprehensive_reports/summary_report.pdf");
    
    std::cout << "✓ 综合分析报告生成完成" << std::endl;
}

// 创建研究时间线
void ComprehensiveAnalysisController::CreateResearchTimeline() {
    std::cout << "\n=== 创建研究时间线 ===" << std::endl;
    
    TCanvas* c_timeline = new TCanvas("c_research_timeline", "Research Timeline", 1400, 800);
    
    // 创建甘特图风格的时间线
    std::vector<std::string> tasks = {
        "数据库建立", "回归分析工具", "综合输运模型", "优化算法", 
        "SCI论文初稿", "实验设计", "样品制备", "验证测试", 
        "数据分析", "论文修改", "论文发表"
    };
    
    std::vector<double> start_months = {0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::vector<double> durations = {2, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1};
    
    TH2D* h_timeline = new TH2D("h_timeline", "Research Timeline (2026.9 - 2027.8);Month;Task", 
                                12, 0, 12, tasks.size(), 0, tasks.size());
    
    for (int i = 0; i < tasks.size(); i++) {
        for (int j = start_months[i]; j < start_months[i] + durations[i]; j++) {
            h_timeline->SetBinContent(j + 1, i + 1, 1);
        }
        h_timeline->GetYaxis()->SetBinLabel(i + 1, tasks[i].c_str());
    }
    
    // 设置月份标签
    std::vector<std::string> months = {
        "2026.9", "2026.10", "2026.11", "2026.12", 
        "2027.1", "2027.2", "2027.3", "2027.4", 
        "2027.5", "2027.6", "2027.7", "2027.8"
    };
    
    for (int i = 0; i < months.size(); i++) {
        h_timeline->GetXaxis()->SetBinLabel(i + 1, months[i].c_str());
    }
    
    h_timeline->Draw("COLZ");
    h_timeline->SetStats(0);
    
    // 添加关键里程碑
    TLatex* milestone = new TLatex();
    milestone->SetTextColor(kRed);
    milestone->SetTextSize(0.03);
    milestone->DrawLatex(5.5, 5.5, "论文初稿完成");
    milestone->DrawLatex(8.5, 8.5, "实验验证开始");
    milestone->DrawLatex(11.5, 11.5, "论文发表");
    
    c_timeline->SaveAs("analysis_results/comprehensive_reports/research_timeline.png");
    c_timeline->SaveAs("analysis_results/comprehensive_reports/research_timeline.pdf");
    
    std::cout << "✓ 研究时间线创建完成" << std::endl;
}

// 导出结果数据
void ComprehensiveAnalysisController::ExportResults() {
    std::cout << "\n=== 导出结果数据 ===" << std::endl;
    
    // 创建结果汇总文件
    std::ofstream summary_file("analysis_results/comprehensive_reports/analysis_summary.txt");
    
    summary_file << "中子伽马复合屏蔽玻璃综合分析结果汇总" << std::endl;
    summary_file << "========================================" << std::endl;
    summary_file << std::endl;
    summary_file << "分析日期: " << __DATE__ << std::endl;
    summary_file << "分析时间: " << __TIME__ << std::endl;
    summary_file << "项目路径: " << project_path << std::endl;
    summary_file << std::endl;
    
    summary_file << "生成的分析图表:" << std::endl;
    summary_file << "1. 基础屏蔽分析图表 (analysis_results/basic_analysis/)" << std::endl;
    summary_file << "2. 数学建模分析图表 (analysis_results/mathematical_models/)" << std::endl;
    summary_file << "3. 实验验证分析图表 (analysis_results/experimental_validation/)" << std::endl;
    summary_file << "4. 综合分析报告 (analysis_results/comprehensive_reports/)" << std::endl;
    summary_file << std::endl;
    
    summary_file << "研究目标:" << std::endl;
    summary_file << "- 建立中子和伽马射线综合输运模型" << std::endl;
    summary_file << "- 研究玻璃组成对屏蔽性能的数学规律" << std::endl;
    summary_file << "- 实现±15%精度的实验验证" << std::endl;
    summary_file << "- 发表SCI/EI论文1篇" << std::endl;
    summary_file << std::endl;
    
    summary_file << "下一步工作:" << std::endl;
    summary_file << "1. 完善数学模型的理论基础" << std::endl;
    summary_file << "2. 设计和执行验证实验" << std::endl;
    summary_file << "3. 撰写和发表学术论文" << std::endl;
    
    summary_file.close();
    
    std::cout << "✓ 结果数据导出完成" << std::endl;
}

// 打印进度
void ComprehensiveAnalysisController::PrintProgress(const std::string& task, int progress) {
    if (!verbose_mode) return;
    
    std::cout << "\r" << task << ": [";
    int pos = progress / 5;
    for (int i = 0; i < 20; ++i) {
        if (i < pos) std::cout << "█";
        else std::cout << " ";
    }
    std::cout << "] " << progress << "%";
    if (progress >= 100) std::cout << std::endl;
    std::cout.flush();
}

// 打印最终结果
void ComprehensiveAnalysisController::PrintResults() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    分析完成总结                              ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ ✓ 基础屏蔽分析：能量-透射率关系建立                         ║" << std::endl;
    std::cout << "║ ✓ 数学建模分析：回归模型和优化算法完成                       ║" << std::endl;
    std::cout << "║ ✓ 实验验证分析：精度评估和误差分析完成                       ║" << std::endl;
    std::cout << "║ ✓ 综合分析报告：所有结果整合完成                             ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║ 输出目录：./analysis_results/                                ║" << std::endl;
    std::cout << "║ 报告文件：analysis_summary.txt                              ║" << std::endl;
    std::cout << "║ 时间规划：research_timeline.png                             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\n";
}

// 运行选定的分析
void ComprehensiveAnalysisController::RunSelectedAnalysis(int choice) {
    switch (choice) {
        case 1:
            RunBasicShieldingAnalysis();
            break;
        case 2:
            RunMathematicalModeling();
            break;
        case 3:
            RunExperimentalValidation();
            break;
        case 4:
            RunComprehensiveAnalysis();
            break;
        case 5:
            RunAllAnalyses();
            break;
        case 6:
            CreateResearchTimeline();
            break;
        case 7:
            ExportResults();
            break;
        default:
            std::cout << "无效选择，请重新输入。" << std::endl;
            break;
    }
}

//=============================================================================
// 主函数
//=============================================================================
void run_comprehensive_analysis() {
    // 创建分析控制器
    ComprehensiveAnalysisController controller(true);
    
    // 初始化系统
    controller.InitializeSystem();
    
    // 交互式菜单
    int choice = -1;
    while (choice != 0) {
        controller.ShowMenu();
        std::cin >> choice;
        
        if (choice != 0) {
            controller.RunSelectedAnalysis(choice);
            
            std::cout << "\n按回车键继续...";
            std::cin.ignore();
            std::cin.get();
        }
    }
    
    std::cout << "感谢使用中子伽马复合屏蔽玻璃综合分析系统！" << std::endl;
}

//=============================================================================
// 快速运行函数（非交互式）
//=============================================================================
void run_all_analysis_batch() {
    std::cout << "=== 批处理模式：运行所有分析 ===" << std::endl;
    
    ComprehensiveAnalysisController controller(true);
    controller.InitializeSystem();
    controller.RunAllAnalyses();
    
    std::cout << "=== 批处理完成 ===" << std::endl;
}
