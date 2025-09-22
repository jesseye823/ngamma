#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TF1.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TLine.h"
#include "TEllipse.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

// 实验验证和精度分析类
class ExperimentalValidationAnalysis {
private:
    std::vector<double> experimental_data;
    std::vector<double> simulation_data;
    std::vector<double> uncertainties;
    std::vector<std::string> test_conditions;
    
public:
    ExperimentalValidationAnalysis();
    ~ExperimentalValidationAnalysis();
    
    // 数据加载和预处理
    void LoadExperimentalData();
    void LoadSimulationResults();
    void CalculateUncertainties();
    
    // 精度验证分析
    void PerformAccuracyValidation();
    void AnalyzeErrorSources();
    void CalculateStatisticalMetrics();
    void PerformUncertaintyPropagation();
    
    // 实验设计优化
    void OptimizeExperimentalDesign();
    void AnalyzeMeasurementSensitivity();
    void PlanValidationExperiments();
    
    // 可视化功能
    void PlotValidationComparison();
    void PlotErrorAnalysis();
    void PlotUncertaintyQuantification();
    void PlotExperimentalDesign();
    
    // 报告生成
    void GenerateValidationReport();
    void ExportValidationData();
    
    // 辅助函数
    double CalculateRMSE(const std::vector<double>& pred, const std::vector<double>& actual);
    double CalculateMAE(const std::vector<double>& pred, const std::vector<double>& actual);
    double CalculateR2(const std::vector<double>& pred, const std::vector<double>& actual);
    bool CheckAccuracyRequirement(double error_percentage, double threshold = 15.0);
};

// 构造函数
ExperimentalValidationAnalysis::ExperimentalValidationAnalysis() {
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(true);
    gStyle->SetPadGridY(true);
    gStyle->SetPalette(kRainBow);
    
    std::cout << "=== 实验验证和精度分析系统初始化 ===" << std::endl;
    std::cout << "目标精度要求: ±15% (2027年考核指标)" << std::endl;
}

ExperimentalValidationAnalysis::~ExperimentalValidationAnalysis() {
    std::cout << "=== 实验验证分析完成 ===" << std::endl;
}

// 加载实验数据
void ExperimentalValidationAnalysis::LoadExperimentalData() {
    std::cout << "\n=== 加载实验验证数据 ===" << std::endl;
    
    // 模拟不同测试条件下的实验数据
    test_conditions = {
        "Am-241 Gamma (59.5 keV)",
        "Cs-137 Gamma (662 keV)", 
        "Co-60 Gamma (1.17/1.33 MeV)",
        "Cf-252 Neutron (Fission)",
        "AmBe Neutron (Mixed)",
        "Thermal Neutron (25.3 meV)",
        "Fast Neutron (2.5 MeV)",
        "Mixed Gamma-Neutron Field",
        "High Energy Gamma (6 MeV)",
        "Epithermal Neutron (1 eV)"
    };
    
    // 模拟实验测得的屏蔽效率数据（%）
    TRandom3 rand(12345);
    std::vector<double> base_efficiencies = {
        78.5, 82.3, 85.1, 76.8, 79.2, 88.9, 74.5, 81.7, 86.4, 83.2
    };
    
    for (int i = 0; i < base_efficiencies.size(); i++) {
        // 添加实验误差和不确定性
        double exp_error = rand.Gaus(0, 1.5); // 实验系统误差
        double statistical_error = rand.Gaus(0, 0.8); // 统计误差
        
        double exp_value = base_efficiencies[i] + exp_error + statistical_error;
        experimental_data.push_back(exp_value);
        
        // 实验不确定性（包括系统误差和统计误差）
        double uncertainty = TMath::Sqrt(1.5*1.5 + 0.8*0.8 + 0.5*0.5); // 设备精度
        uncertainties.push_back(uncertainty);
    }
    
    std::cout << "已加载 " << experimental_data.size() << " 组实验数据" << std::endl;
}

// 加载仿真结果
void ExperimentalValidationAnalysis::LoadSimulationResults() {
    std::cout << "=== 加载仿真结果数据 ===" << std::endl;
    
    // 模拟仿真预测的屏蔽效率数据
    TRandom3 rand_sim(54321);
    std::vector<double> base_sim_efficiencies = {
        80.1, 83.7, 84.2, 78.9, 77.8, 87.3, 76.2, 83.1, 85.8, 81.9
    };
    
    for (int i = 0; i < base_sim_efficiencies.size(); i++) {
        // 添加仿真模型不确定性
        double model_uncertainty = rand_sim.Gaus(0, 1.2);
        double numerical_error = rand_sim.Gaus(0, 0.3);
        
        double sim_value = base_sim_efficiencies[i] + model_uncertainty + numerical_error;
        simulation_data.push_back(sim_value);
    }
    
    std::cout << "已加载 " << simulation_data.size() << " 组仿真数据" << std::endl;
}

// 精度验证分析
void ExperimentalValidationAnalysis::PerformAccuracyValidation() {
    std::cout << "\n=== 执行精度验证分析 ===" << std::endl;
    
    TCanvas* c1 = new TCanvas("c_accuracy_validation", "Accuracy Validation Analysis", 1600, 1200);
    c1->Divide(2, 2);
    
    // 1.1 仿真vs实验对比图
    c1->cd(1);
    
    TGraphErrors* gr_comparison = new TGraphErrors(experimental_data.size());
    TGraph* gr_ideal = new TGraph();
    TGraph* gr_error_band_upper = new TGraph();
    TGraph* gr_error_band_lower = new TGraph();
    
    double min_val = 70, max_val = 95;
    
    for (int i = 0; i < experimental_data.size(); i++) {
        gr_comparison->SetPoint(i, experimental_data[i], simulation_data[i]);
        gr_comparison->SetPointError(i, uncertainties[i], 1.0); // 实验误差棒
        
        min_val = TMath::Min(min_val, TMath::Min(experimental_data[i], simulation_data[i]) - 3);
        max_val = TMath::Max(max_val, TMath::Max(experimental_data[i], simulation_data[i]) + 3);
    }
    
    // 理想线 y=x
    gr_ideal->SetPoint(0, min_val, min_val);
    gr_ideal->SetPoint(1, max_val, max_val);
    
    // ±15%误差带
    for (int i = 0; i <= 100; i++) {
        double x = min_val + i * (max_val - min_val) / 100.0;
        gr_error_band_upper->SetPoint(i, x, x * 1.15);
        gr_error_band_lower->SetPoint(i, x, x * 0.85);
    }
    
    // 绘制误差带
    gr_error_band_upper->SetLineColor(kGray+1);
    gr_error_band_upper->SetLineStyle(2);
    gr_error_band_upper->SetLineWidth(2);
    gr_error_band_lower->SetLineColor(kGray+1);
    gr_error_band_lower->SetLineStyle(2);
    gr_error_band_lower->SetLineWidth(2);
    
    gr_error_band_upper->Draw("AL");
    gr_error_band_lower->Draw("L same");
    
    // 理想线
    gr_ideal->SetLineColor(kBlack);
    gr_ideal->SetLineWidth(3);
    gr_ideal->Draw("L same");
    
    // 数据点
    gr_comparison->SetTitle("Simulation vs Experimental Validation;Experimental Efficiency (%);Simulation Efficiency (%)");
    gr_comparison->SetMarkerStyle(20);
    gr_comparison->SetMarkerColor(kRed);
    gr_comparison->SetMarkerSize(1.2);
    gr_comparison->SetLineColor(kRed);
    gr_comparison->Draw("P same");
    
    TLegend* leg1 = new TLegend(0.15, 0.7, 0.5, 0.9);
    leg1->AddEntry(gr_comparison, "Sim vs Exp Data", "p");
    leg1->AddEntry(gr_ideal, "Ideal Line (y=x)", "l");
    leg1->AddEntry(gr_error_band_upper, "±15% Error Band", "l");
    leg1->Draw();
    
    // 1.2 相对误差分析
    c1->cd(2);
    
    TGraph* gr_relative_error = new TGraph();
    std::vector<double> relative_errors;
    
    for (int i = 0; i < experimental_data.size(); i++) {
        double rel_error = (simulation_data[i] - experimental_data[i]) / experimental_data[i] * 100.0;
        relative_errors.push_back(rel_error);
        gr_relative_error->SetPoint(i, i + 1, rel_error);
    }
    
    gr_relative_error->SetTitle("Relative Error Analysis;Test Case;Relative Error (%)");
    gr_relative_error->SetMarkerStyle(21);
    gr_relative_error->SetMarkerColor(kBlue);
    gr_relative_error->SetLineColor(kBlue);
    gr_relative_error->SetLineWidth(2);
    gr_relative_error->Draw("APL");
    
    // 添加±15%基准线
    TLine* line_upper = new TLine(0.5, 15, experimental_data.size() + 0.5, 15);
    TLine* line_lower = new TLine(0.5, -15, experimental_data.size() + 0.5, -15);
    TLine* line_zero = new TLine(0.5, 0, experimental_data.size() + 0.5, 0);
    
    line_upper->SetLineColor(kRed);
    line_upper->SetLineWidth(2);
    line_upper->SetLineStyle(2);
    line_lower->SetLineColor(kRed);
    line_lower->SetLineWidth(2);
    line_lower->SetLineStyle(2);
    line_zero->SetLineColor(kBlack);
    line_zero->SetLineWidth(1);
    line_zero->SetLineStyle(1);
    
    line_upper->Draw("same");
    line_lower->Draw("same");
    line_zero->Draw("same");
    
    // 标注超出误差范围的点
    for (int i = 0; i < relative_errors.size(); i++) {
        if (TMath::Abs(relative_errors[i]) > 15.0) {
            TLatex* latex_outlier = new TLatex();
            latex_outlier->SetTextColor(kRed);
            latex_outlier->SetTextSize(0.03);
            latex_outlier->DrawLatex(i + 1.1, relative_errors[i], "!");
        }
    }
    
    // 1.3 统计指标分析
    c1->cd(3);
    
    // 计算统计指标
    double rmse = CalculateRMSE(simulation_data, experimental_data);
    double mae = CalculateMAE(simulation_data, experimental_data);
    double r2 = CalculateR2(simulation_data, experimental_data);
    
    // 计算在±15%范围内的数据点百分比
    int within_tolerance = 0;
    for (double error : relative_errors) {
        if (TMath::Abs(error) <= 15.0) within_tolerance++;
    }
    double tolerance_percentage = (double)within_tolerance / relative_errors.size() * 100.0;
    
    TPaveText* stats_box = new TPaveText(0.1, 0.1, 0.9, 0.9, "NDC");
    stats_box->SetFillColor(kWhite);
    stats_box->SetBorderSize(2);
    stats_box->SetTextAlign(12);
    stats_box->SetTextSize(0.05);
    
    stats_box->AddText("=== 精度验证统计指标 ===");
    stats_box->AddText("");
    stats_box->AddText(Form("数据点总数: %d", (int)experimental_data.size()));
    stats_box->AddText(Form("RMSE: %.2f%%", rmse));
    stats_box->AddText(Form("MAE: %.2f%%", mae));
    stats_box->AddText(Form("R²: %.4f", r2));
    stats_box->AddText("");
    stats_box->AddText("=== 精度要求评估 ===");
    stats_box->AddText(Form("±15%%范围内: %d/%d (%.1f%%)", within_tolerance, (int)relative_errors.size(), tolerance_percentage));
    stats_box->AddText("");
    
    bool meets_requirement = (tolerance_percentage >= 80.0 && mae <= 15.0);
    if (meets_requirement) {
        stats_box->AddText("✓ 满足±15%精度要求");
        stats_box->SetTextColor(kGreen+2);
    } else {
        stats_box->AddText("✗ 未满足±15%精度要求");
        stats_box->SetTextColor(kRed);
    }
    
    stats_box->Draw();
    
    // 1.4 误差分布直方图
    c1->cd(4);
    
    TH1D* h_error_dist = new TH1D("h_error_dist", "Error Distribution;Relative Error (%);Frequency", 
                                  20, -25, 25);
    
    for (double error : relative_errors) {
        h_error_dist->Fill(error);
    }
    
    h_error_dist->SetFillColor(kCyan);
    h_error_dist->SetLineColor(kBlue);
    h_error_dist->SetLineWidth(2);
    h_error_dist->Draw("HIST");
    
    // 添加正态分布拟合
    TF1* gauss_fit = new TF1("gauss_fit", "gaus", -25, 25);
    h_error_dist->Fit(gauss_fit, "R");
    gauss_fit->SetLineColor(kRed);
    gauss_fit->SetLineWidth(2);
    gauss_fit->Draw("same");
    
    // 添加±15%区域标记
    TLine* tol_left = new TLine(-15, 0, -15, h_error_dist->GetMaximum());
    TLine* tol_right = new TLine(15, 0, 15, h_error_dist->GetMaximum());
    tol_left->SetLineColor(kGreen);
    tol_left->SetLineWidth(3);
    tol_right->SetLineColor(kGreen);
    tol_right->SetLineWidth(3);
    tol_left->Draw("same");
    tol_right->Draw("same");
    
    TLatex* latex_stats = new TLatex();
    latex_stats->SetNDC();
    latex_stats->SetTextSize(0.04);
    latex_stats->DrawLatex(0.6, 0.8, Form("Mean: %.2f%%", h_error_dist->GetMean()));
    latex_stats->DrawLatex(0.6, 0.75, Form("Std Dev: %.2f%%", h_error_dist->GetRMS()));
    latex_stats->DrawLatex(0.6, 0.7, Form("χ²/ndf: %.2f", gauss_fit->GetChisquare()/gauss_fit->GetNDF()));
    
    c1->SaveAs("accuracy_validation_analysis.png");
    c1->SaveAs("accuracy_validation_analysis.pdf");
    std::cout << "已保存: accuracy_validation_analysis.png/pdf" << std::endl;
}

// 误差源分析
void ExperimentalValidationAnalysis::AnalyzeErrorSources() {
    std::cout << "\n=== 误差源分析 ===" << std::endl;
    
    TCanvas* c2 = new TCanvas("c_error_sources", "Error Sources Analysis", 1600, 1200);
    c2->Divide(2, 2);
    
    // 2.1 误差源贡献分析
    c2->cd(1);
    
    std::vector<std::string> error_sources = {
        "几何建模", "材料参数", "物理模型", "数值精度", 
        "实验设备", "环境条件", "人为因素", "统计涨落"
    };
    
    std::vector<double> error_contributions = {
        2.5, 3.2, 4.1, 1.8, 2.8, 1.5, 1.2, 2.9
    }; // 各误差源的标准差贡献 (%)
    
    TH1D* h_error_sources = new TH1D("h_error_sources", "Error Sources Contribution;Error Source;Contribution (%)", 
                                     error_sources.size(), 0, error_sources.size());
    
    for (int i = 0; i < error_sources.size(); i++) {
        h_error_sources->SetBinContent(i + 1, error_contributions[i]);
        h_error_sources->GetXaxis()->SetBinLabel(i + 1, error_sources[i].c_str());
    }
    
    h_error_sources->SetFillColor(kOrange);
    h_error_sources->SetLineColor(kOrange+2);
    h_error_sources->SetLineWidth(2);
    h_error_sources->Draw("HIST");
    
    // 计算总误差
    double total_error_sq = 0;
    for (double err : error_contributions) {
        total_error_sq += err * err;
    }
    double total_error = TMath::Sqrt(total_error_sq);
    
    TLatex* latex_total = new TLatex();
    latex_total->SetNDC();
    latex_total->SetTextSize(0.04);
    latex_total->DrawLatex(0.6, 0.8, Form("总合成误差: %.2f%%", total_error));
    latex_total->DrawLatex(0.6, 0.75, "计算公式: σ_total = √(Σσᵢ²)");
    
    // 2.2 误差传播分析
    c2->cd(2);
    
    // 模拟误差传播过程
    TMultiGraph* mg_propagation = new TMultiGraph();
    
    std::vector<double> measurement_steps = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<double> cumulative_error = {1.2, 2.1, 3.5, 4.8, 6.2, 7.1, 8.3, total_error};
    
    TGraph* gr_error_prop = new TGraph(measurement_steps.size());
    for (int i = 0; i < measurement_steps.size(); i++) {
        gr_error_prop->SetPoint(i, measurement_steps[i], cumulative_error[i]);
    }
    
    gr_error_prop->SetLineColor(kRed);
    gr_error_prop->SetLineWidth(3);
    gr_error_prop->SetMarkerStyle(20);
    gr_error_prop->SetMarkerColor(kRed);
    gr_error_prop->SetMarkerSize(1.2);
    
    mg_propagation->Add(gr_error_prop);
    mg_propagation->SetTitle("Error Propagation Analysis;Measurement Step;Cumulative Error (%)");
    mg_propagation->Draw("APL");
    
    // 添加15%基准线
    TLine* tolerance_line = new TLine(1, 15, 8, 15);
    tolerance_line->SetLineColor(kGreen);
    tolerance_line->SetLineWidth(2);
    tolerance_line->SetLineStyle(2);
    tolerance_line->Draw("same");
    
    TLegend* leg_prop = new TLegend(0.2, 0.7, 0.5, 0.9);
    leg_prop->AddEntry(gr_error_prop, "累积误差", "lp");
    leg_prop->AddEntry(tolerance_line, "15%容差线", "l");
    leg_prop->Draw();
    
    // 2.3 敏感性分析
    c2->cd(3);
    
    std::vector<std::string> parameters = {"密度", "厚度", "能量", "角度", "温度"};
    std::vector<double> sensitivity_coefficients = {0.85, 1.20, -0.65, 0.35, 0.15};
    
    TH1D* h_sensitivity = new TH1D("h_sensitivity", "Parameter Sensitivity Analysis;Parameter;Sensitivity Coefficient", 
                                   parameters.size(), 0, parameters.size());
    
    for (int i = 0; i < parameters.size(); i++) {
        h_sensitivity->SetBinContent(i + 1, TMath::Abs(sensitivity_coefficients[i]));
        h_sensitivity->GetXaxis()->SetBinLabel(i + 1, parameters[i].c_str());
        
        // 根据敏感性设置颜色
        if (TMath::Abs(sensitivity_coefficients[i]) > 1.0) {
            h_sensitivity->SetFillColor(kRed);
        } else if (TMath::Abs(sensitivity_coefficients[i]) > 0.5) {
            h_sensitivity->SetFillColor(kOrange);
        } else {
            h_sensitivity->SetFillColor(kGreen);
        }
    }
    
    h_sensitivity->SetLineColor(kBlack);
    h_sensitivity->SetLineWidth(2);
    h_sensitivity->Draw("HIST");
    
    // 2.4 改进建议优先级
    c2->cd(4);
    
    std::vector<std::string> improvements = {
        "提高材料参数精度",
        "优化物理模型",
        "改进实验设备",
        "增加统计样本",
        "标准化测试流程",
        "环境控制"
    };
    
    std::vector<double> impact_scores = {8.5, 7.8, 6.2, 5.5, 4.8, 3.2};
    std::vector<double> difficulty_scores = {3.2, 8.1, 6.5, 2.1, 2.8, 4.5};
    
    TGraph* gr_improvement = new TGraph(improvements.size());
    for (int i = 0; i < improvements.size(); i++) {
        gr_improvement->SetPoint(i, difficulty_scores[i], impact_scores[i]);
    }
    
    gr_improvement->SetTitle("Improvement Priority Matrix;Implementation Difficulty;Expected Impact");
    gr_improvement->SetMarkerStyle(20);
    gr_improvement->SetMarkerColor(kBlue);
    gr_improvement->SetMarkerSize(1.5);
    gr_improvement->Draw("AP");
    
    // 添加标签
    for (int i = 0; i < improvements.size(); i++) {
        TLatex* label = new TLatex();
        label->SetTextSize(0.025);
        label->SetTextAlign(12);
        label->DrawLatex(difficulty_scores[i] + 0.1, impact_scores[i], improvements[i].c_str());
    }
    
    // 添加优先级区域
    TEllipse* high_priority = new TEllipse(3, 8, 2, 1.5);
    high_priority->SetLineColor(kGreen);
    high_priority->SetLineWidth(2);
    high_priority->SetFillStyle(0);
    high_priority->Draw("same");
    
    TLatex* priority_label = new TLatex();
    priority_label->SetTextColor(kGreen);
    priority_label->SetTextSize(0.04);
    priority_label->DrawLatex(1.5, 9, "高优先级区域");
    
    c2->SaveAs("error_sources_analysis.png");
    c2->SaveAs("error_sources_analysis.pdf");
    std::cout << "已保存: error_sources_analysis.png/pdf" << std::endl;
}

// 不确定性量化分析
void ExperimentalValidationAnalysis::PerformUncertaintyPropagation() {
    std::cout << "\n=== 不确定性传播分析 ===" << std::endl;
    
    TCanvas* c3 = new TCanvas("c_uncertainty", "Uncertainty Quantification", 1600, 1200);
    c3->Divide(2, 2);
    
    // 3.1 蒙特卡罗不确定性传播
    c3->cd(1);
    
    TH1D* h_mc_results = new TH1D("h_mc_results", "Monte Carlo Uncertainty Propagation;Shielding Efficiency (%);Probability Density", 
                                  50, 75, 90);
    
    TRandom3 rand_mc(98765);
    
    // 蒙特卡罗模拟
    for (int i = 0; i < 10000; i++) {
        // 输入参数的不确定性
        double density_uncertainty = rand_mc.Gaus(1.0, 0.02);    // ±2%
        double thickness_uncertainty = rand_mc.Gaus(1.0, 0.01);  // ±1%
        double composition_uncertainty = rand_mc.Gaus(1.0, 0.03); // ±3%
        
        // 模型不确定性
        double model_uncertainty = rand_mc.Gaus(1.0, 0.025);     // ±2.5%
        
        // 计算输出的不确定性
        double base_efficiency = 82.5;
        double result = base_efficiency * density_uncertainty * thickness_uncertainty * 
                       composition_uncertainty * model_uncertainty;
        
        h_mc_results->Fill(result);
    }
    
    h_mc_results->SetFillColor(kCyan);
    h_mc_results->SetLineColor(kBlue);
    h_mc_results->SetLineWidth(2);
    h_mc_results->Draw("HIST");
    
    // 计算置信区间
    double mean = h_mc_results->GetMean();
    double std_dev = h_mc_results->GetRMS();
    
    TLine* mean_line = new TLine(mean, 0, mean, h_mc_results->GetMaximum());
    TLine* ci_lower = new TLine(mean - 1.96*std_dev, 0, mean - 1.96*std_dev, h_mc_results->GetMaximum()*0.7);
    TLine* ci_upper = new TLine(mean + 1.96*std_dev, 0, mean + 1.96*std_dev, h_mc_results->GetMaximum()*0.7);
    
    mean_line->SetLineColor(kRed);
    mean_line->SetLineWidth(2);
    ci_lower->SetLineColor(kGreen);
    ci_lower->SetLineStyle(2);
    ci_upper->SetLineColor(kGreen);
    ci_upper->SetLineStyle(2);
    
    mean_line->Draw("same");
    ci_lower->Draw("same");
    ci_upper->Draw("same");
    
    TLatex* latex_mc = new TLatex();
    latex_mc->SetNDC();
    latex_mc->SetTextSize(0.04);
    latex_mc->DrawLatex(0.6, 0.8, Form("均值: %.2f%%", mean));
    latex_mc->DrawLatex(0.6, 0.75, Form("标准差: %.2f%%", std_dev));
    latex_mc->DrawLatex(0.6, 0.7, Form("95%% CI: [%.2f, %.2f]", mean-1.96*std_dev, mean+1.96*std_dev));
    
    // 3.2 参数相关性分析
    c3->cd(2);
    
    TH2D* h_correlation = new TH2D("h_correlation", "Parameter Correlation Matrix;Parameter;Parameter", 
                                   5, 0, 5, 5, 0, 5);
    
    std::vector<std::string> param_names = {"密度", "厚度", "成分", "能量", "角度"};
    
    // 相关性矩阵数据
    double corr_matrix[5][5] = {
        {1.00, -0.15, 0.25, 0.10, 0.05},
        {-0.15, 1.00, -0.08, -0.12, 0.20},
        {0.25, -0.08, 1.00, 0.30, -0.05},
        {0.10, -0.12, 0.30, 1.00, -0.25},
        {0.05, 0.20, -0.05, -0.25, 1.00}
    };
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            h_correlation->SetBinContent(i+1, j+1, corr_matrix[i][j]);
        }
        h_correlation->GetXaxis()->SetBinLabel(i+1, param_names[i].c_str());
        h_correlation->GetYaxis()->SetBinLabel(i+1, param_names[i].c_str());
    }
    
    h_correlation->Draw("COLZ TEXT");
    h_correlation->SetMarkerSize(1.2);
    
    // 3.3 敏感性系数分析
    c3->cd(3);
    
    TMultiGraph* mg_sensitivity = new TMultiGraph();
    
    std::vector<double> param_variations = {-10, -5, -2, -1, 0, 1, 2, 5, 10};
    
    TGraph* gr_density_sens = new TGraph();
    TGraph* gr_thickness_sens = new TGraph();
    TGraph* gr_composition_sens = new TGraph();
    
    for (int i = 0; i < param_variations.size(); i++) {
        double var = param_variations[i];
        
        // 敏感性系数 (∂η/∂x) * (x/η)
        double density_response = 82.5 * (1 + 0.85 * var / 100.0);
        double thickness_response = 82.5 * (1 + 1.20 * var / 100.0);
        double composition_response = 82.5 * (1 + 0.65 * var / 100.0);
        
        gr_density_sens->SetPoint(i, var, density_response);
        gr_thickness_sens->SetPoint(i, var, thickness_response);
        gr_composition_sens->SetPoint(i, var, composition_response);
    }
    
    gr_density_sens->SetLineColor(kRed);
    gr_density_sens->SetLineWidth(2);
    gr_thickness_sens->SetLineColor(kBlue);
    gr_thickness_sens->SetLineWidth(2);
    gr_composition_sens->SetLineColor(kGreen);
    gr_composition_sens->SetLineWidth(2);
    
    mg_sensitivity->Add(gr_density_sens);
    mg_sensitivity->Add(gr_thickness_sens);
    mg_sensitivity->Add(gr_composition_sens);
    mg_sensitivity->SetTitle("Parameter Sensitivity Analysis;Parameter Variation (%);Efficiency Response (%)");
    mg_sensitivity->Draw("AL");
    
    TLegend* leg_sens = new TLegend(0.2, 0.7, 0.5, 0.9);
    leg_sens->AddEntry(gr_density_sens, "密度", "l");
    leg_sens->AddEntry(gr_thickness_sens, "厚度", "l");
    leg_sens->AddEntry(gr_composition_sens, "成分", "l");
    leg_sens->Draw();
    
    // 3.4 置信区间预测
    c3->cd(4);
    
    TGraphErrors* gr_prediction_bands = new TGraphErrors();
    
    std::vector<double> energy_points = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
    
    for (int i = 0; i < energy_points.size(); i++) {
        double energy = energy_points[i];
        
        // 预测效率及其不确定性
        double predicted_eff = 85 - 5 * TMath::Log10(energy);
        double prediction_uncertainty = 2.5 + 0.5 * energy; // 随能量增加的不确定性
        
        gr_prediction_bands->SetPoint(i, energy, predicted_eff);
        gr_prediction_bands->SetPointError(i, 0, prediction_uncertainty);
    }
    
    gr_prediction_bands->SetTitle("Prediction Confidence Intervals;Energy (MeV);Predicted Efficiency (%)");
    gr_prediction_bands->SetMarkerStyle(20);
    gr_prediction_bands->SetMarkerColor(kBlue);
    gr_prediction_bands->SetLineColor(kBlue);
    gr_prediction_bands->SetFillColor(kCyan);
    gr_prediction_bands->SetFillStyle(3004);
    gr_prediction_bands->Draw("A3");
    gr_prediction_bands->Draw("PX same");
    
    gPad->SetLogx();
    
    c3->SaveAs("uncertainty_quantification.png");
    c3->SaveAs("uncertainty_quantification.pdf");
    std::cout << "已保存: uncertainty_quantification.png/pdf" << std::endl;
}

// 实验设计优化
void ExperimentalValidationAnalysis::OptimizeExperimentalDesign() {
    std::cout << "\n=== 实验设计优化分析 ===" << std::endl;
    
    TCanvas* c4 = new TCanvas("c_exp_design", "Experimental Design Optimization", 1600, 1200);
    c4->Divide(2, 2);
    
    // 4.1 测量点优化
    c4->cd(1);
    
    // 基于方差最小化的最优测量点选择
    std::vector<double> candidate_energies;
    std::vector<double> information_content;
    
    for (double E = 0.05; E <= 15.0; E += 0.1) {
        candidate_energies.push_back(E);
        
        // 信息含量 = 1/方差，方差与敏感性和测量精度有关
        double sensitivity = TMath::Abs(-0.3 / TMath::Power(E, 0.3)); // dμ/dE
        double measurement_precision = 1.0 / (1.0 + 0.1 * E); // 高能时精度下降
        double information = sensitivity * measurement_precision;
        
        information_content.push_back(information);
    }
    
    TGraph* gr_information = new TGraph(candidate_energies.size());
    for (int i = 0; i < candidate_energies.size(); i++) {
        gr_information->SetPoint(i, candidate_energies[i], information_content[i]);
    }
    
    gr_information->SetTitle("Measurement Point Optimization;Energy (MeV);Information Content");
    gr_information->SetLineColor(kBlue);
    gr_information->SetLineWidth(2);
    gr_information->Draw("AL");
    gPad->SetLogy();
    
    // 标记最优测量点
    std::vector<int> optimal_indices = {10, 25, 50, 80, 120}; // 示例最优点
    TGraph* gr_optimal_points = new TGraph();
    
    for (int i = 0; i < optimal_indices.size(); i++) {
        int idx = optimal_indices[i];
        gr_optimal_points->SetPoint(i, candidate_energies[idx], information_content[idx]);
    }
    
    gr_optimal_points->SetMarkerStyle(23);
    gr_optimal_points->SetMarkerColor(kRed);
    gr_optimal_points->SetMarkerSize(1.5);
    gr_optimal_points->Draw("P same");
    
    TLegend* leg_opt = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg_opt->AddEntry(gr_information, "信息含量", "l");
    leg_opt->AddEntry(gr_optimal_points, "最优测量点", "p");
    leg_opt->Draw();
    
    // 4.2 样本量分析
    c4->cd(2);
    
    std::vector<int> sample_sizes = {10, 20, 50, 100, 200, 500, 1000};
    std::vector<double> statistical_errors;
    std::vector<double> costs;
    
    for (int n : sample_sizes) {
        // 统计误差 ∝ 1/√n
        double stat_error = 5.0 / TMath::Sqrt(n);
        statistical_errors.push_back(stat_error);
        
        // 成本模型（非线性增长）
        double cost = 1000 + 50 * n + 0.1 * n * n;
        costs.push_back(cost / 1000.0); // 转换为千元单位
    }
    
    TMultiGraph* mg_sample = new TMultiGraph();
    
    TGraph* gr_stat_error = new TGraph(sample_sizes.size());
    TGraph* gr_cost = new TGraph(sample_sizes.size());
    
    for (int i = 0; i < sample_sizes.size(); i++) {
        gr_stat_error->SetPoint(i, sample_sizes[i], statistical_errors[i]);
        gr_cost->SetPoint(i, sample_sizes[i], costs[i]);
    }
    
    gr_stat_error->SetLineColor(kRed);
    gr_stat_error->SetLineWidth(2);
    gr_stat_error->SetMarkerStyle(20);
    gr_stat_error->SetMarkerColor(kRed);
    
    gr_cost->SetLineColor(kBlue);
    gr_cost->SetLineWidth(2);
    gr_cost->SetMarkerStyle(21);
    gr_cost->SetMarkerColor(kBlue);
    
    mg_sample->Add(gr_stat_error);
    mg_sample->Add(gr_cost);
    mg_sample->SetTitle("Sample Size Optimization;Sample Size;Value");
    mg_sample->Draw("APL");
    gPad->SetLogx();
    
    TLegend* leg_sample = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg_sample->AddEntry(gr_stat_error, "统计误差 (%)", "lp");
    leg_sample->AddEntry(gr_cost, "成本 (千元)", "lp");
    leg_sample->Draw();
    
    // 标记最优样本量（成本效益平衡点）
    int optimal_sample = 200;
    TLine* optimal_line = new TLine(optimal_sample, 0, optimal_sample, 10);
    optimal_line->SetLineColor(kGreen);
    optimal_line->SetLineWidth(3);
    optimal_line->SetLineStyle(2);
    optimal_line->Draw("same");
    
    // 4.3 测量精度要求分析
    c4->cd(3);
    
    std::vector<double> measurement_precisions = {0.5, 1.0, 2.0, 3.0, 5.0, 8.0, 10.0};
    std::vector<double> total_uncertainties;
    std::vector<double> equipment_costs;
    
    for (double prec : measurement_precisions) {
        // 总不确定性包括测量精度和其他误差源
        double other_errors = 2.5; // 其他固定误差源
        double total_unc = TMath::Sqrt(prec*prec + other_errors*other_errors);
        total_uncertainties.push_back(total_unc);
        
        // 设备成本与精度呈反比关系
        double cost = 100 * TMath::Power(1.0/prec, 1.5);
        equipment_costs.push_back(cost);
    }
    
    TMultiGraph* mg_precision = new TMultiGraph();
    
    TGraph* gr_total_unc = new TGraph(measurement_precisions.size());
    TGraph* gr_equip_cost = new TGraph(measurement_precisions.size());
    
    for (int i = 0; i < measurement_precisions.size(); i++) {
        gr_total_unc->SetPoint(i, measurement_precisions[i], total_uncertainties[i]);
        gr_equip_cost->SetPoint(i, measurement_precisions[i], equipment_costs[i]/10); // 缩放显示
    }
    
    gr_total_unc->SetLineColor(kRed);
    gr_total_unc->SetLineWidth(2);
    gr_total_unc->SetMarkerStyle(20);
    gr_total_unc->SetMarkerColor(kRed);
    
    gr_equip_cost->SetLineColor(kBlue);
    gr_equip_cost->SetLineWidth(2);
    gr_equip_cost->SetMarkerStyle(21);
    gr_equip_cost->SetMarkerColor(kBlue);
    
    mg_precision->Add(gr_total_unc);
    mg_precision->Add(gr_equip_cost);
    mg_precision->SetTitle("Measurement Precision Requirements;Measurement Precision (%);Value");
    mg_precision->Draw("APL");
    
    TLegend* leg_prec = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg_prec->AddEntry(gr_total_unc, "总不确定性 (%)", "lp");
    leg_prec->AddEntry(gr_equip_cost, "设备成本 (万元/10)", "lp");
    leg_prec->Draw();
    
    // 4.4 验证实验时间规划
    c4->cd(4);
    
    std::vector<std::string> experiment_phases = {
        "设备采购", "样品制备", "标定测试", "系统测试", 
        "数据采集", "数据分析", "报告撰写", "论文发表"
    };
    
    std::vector<double> phase_durations = {2, 1, 0.5, 1, 3, 2, 1, 2}; // 月
    std::vector<double> phase_starts;
    
    double cumulative_time = 0;
    for (double duration : phase_durations) {
        phase_starts.push_back(cumulative_time);
        cumulative_time += duration;
    }
    
    TH1D* h_timeline = new TH1D("h_timeline", "Validation Experiment Timeline;Phase;Duration (Months)", 
                                experiment_phases.size(), 0, experiment_phases.size());
    
    for (int i = 0; i < experiment_phases.size(); i++) {
        h_timeline->SetBinContent(i + 1, phase_durations[i]);
        h_timeline->GetXaxis()->SetBinLabel(i + 1, experiment_phases[i].c_str());
        
        // 根据阶段重要性设置颜色
        if (i == 4 || i == 5) { // 数据采集和分析阶段
            h_timeline->SetFillColor(kRed);
        } else if (i == 2 || i == 3) { // 测试阶段
            h_timeline->SetFillColor(kOrange);
        } else {
            h_timeline->SetFillColor(kGreen);
        }
    }
    
    h_timeline->SetLineColor(kBlack);
    h_timeline->SetLineWidth(2);
    h_timeline->Draw("HIST");
    
    // 添加关键里程碑
    TLatex* milestone = new TLatex();
    milestone->SetTextColor(kRed);
    milestone->SetTextSize(0.03);
    milestone->DrawLatex(4.5, phase_durations[4] + 0.2, "关键阶段");
    milestone->DrawLatex(7.5, phase_durations[7] + 0.2, "论文截止");
    
    TLatex* total_time = new TLatex();
    total_time->SetNDC();
    total_time->SetTextSize(0.04);
    total_time->DrawLatex(0.6, 0.8, Form("总时间: %.1f 个月", cumulative_time));
    total_time->DrawLatex(0.6, 0.75, "计划完成: 2027年8月");
    
    c4->SaveAs("experimental_design_optimization.png");
    c4->SaveAs("experimental_design_optimization.pdf");
    std::cout << "已保存: experimental_design_optimization.png/pdf" << std::endl;
}

// 辅助函数实现
double ExperimentalValidationAnalysis::CalculateRMSE(const std::vector<double>& pred, const std::vector<double>& actual) {
    double sum_sq_error = 0;
    for (int i = 0; i < pred.size(); i++) {
        double error = (pred[i] - actual[i]) / actual[i] * 100.0;
        sum_sq_error += error * error;
    }
    return TMath::Sqrt(sum_sq_error / pred.size());
}

double ExperimentalValidationAnalysis::CalculateMAE(const std::vector<double>& pred, const std::vector<double>& actual) {
    double sum_abs_error = 0;
    for (int i = 0; i < pred.size(); i++) {
        double error = TMath::Abs(pred[i] - actual[i]) / actual[i] * 100.0;
        sum_abs_error += error;
    }
    return sum_abs_error / pred.size();
}

double ExperimentalValidationAnalysis::CalculateR2(const std::vector<double>& pred, const std::vector<double>& actual) {
    double mean_actual = 0;
    for (double val : actual) mean_actual += val;
    mean_actual /= actual.size();
    
    double ss_tot = 0, ss_res = 0;
    for (int i = 0; i < pred.size(); i++) {
        ss_tot += (actual[i] - mean_actual) * (actual[i] - mean_actual);
        ss_res += (actual[i] - pred[i]) * (actual[i] - pred[i]);
    }
    
    return 1.0 - ss_res / ss_tot;
}

bool ExperimentalValidationAnalysis::CheckAccuracyRequirement(double error_percentage, double threshold) {
    return error_percentage <= threshold;
}

// 生成验证报告
void ExperimentalValidationAnalysis::GenerateValidationReport() {
    std::cout << "\n=== 生成实验验证综合报告 ===" << std::endl;
    
    // 加载数据
    LoadExperimentalData();
    LoadSimulationResults();
    
    // 执行所有分析
    PerformAccuracyValidation();
    AnalyzeErrorSources();
    PerformUncertaintyPropagation();
    OptimizeExperimentalDesign();
    
    // 计算关键指标
    double rmse = CalculateRMSE(simulation_data, experimental_data);
    double mae = CalculateMAE(simulation_data, experimental_data);
    double r2 = CalculateR2(simulation_data, experimental_data);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "    实验验证与精度分析报告" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "\n关键性能指标:" << std::endl;
    std::cout << Form("RMSE: %.2f%%", rmse) << std::endl;
    std::cout << Form("MAE: %.2f%%", mae) << std::endl;
    std::cout << Form("R²: %.4f", r2) << std::endl;
    
    bool meets_2027_target = (mae <= 15.0 && r2 >= 0.85);
    std::cout << "\n2027年精度目标评估:" << std::endl;
    std::cout << "目标要求: ±15%误差范围" << std::endl;
    std::cout << (meets_2027_target ? "✓ 满足精度要求" : "✗ 需要进一步改进") << std::endl;
    
    std::cout << "\n生成的分析图表:" << std::endl;
    std::cout << "1. accuracy_validation_analysis.png/pdf" << std::endl;
    std::cout << "2. error_sources_analysis.png/pdf" << std::endl;
    std::cout << "3. uncertainty_quantification.png/pdf" << std::endl;
    std::cout << "4. experimental_design_optimization.png/pdf" << std::endl;
    
    std::cout << "\n支撑研究闭环的关键内容:" << std::endl;
    std::cout << "- 模拟-实验对比验证" << std::endl;
    std::cout << "- 误差源识别与量化" << std::endl;
    std::cout << "- 不确定性传播分析" << std::endl;
    std::cout << "- 实验设计优化建议" << std::endl;
    std::cout << "- 精度改进路径规划" << std::endl;
}

// 主函数
void experimental_validation_analysis() {
    std::cout << "=== 中子伽马屏蔽玻璃实验验证分析系统 ===" << std::endl;
    std::cout << "目标: 实现模型搭建-性能模拟-实验验证的完整闭环" << std::endl;
    std::cout << "精度要求: ±15% (2027年考核指标)" << std::endl;
    
    ExperimentalValidationAnalysis analyzer;
    analyzer.GenerateValidationReport();
}
