// DPA模型配置和选择
#ifndef DPA_MODEL_CONFIG_HH
#define DPA_MODEL_CONFIG_HH

#include "G4SystemOfUnits.hh"
#include "G4String.hh"

namespace B1 {

// DPA模型类型枚举
enum class DPAModelType {
    NRT,        // NRT模型（默认）
    SRIM        // SRIM模型（高精度，计算时间长）
};

// DPA模型配置类
class DPAModelConfig {
public:
    static DPAModelType GetCurrentModel() { return fCurrentModel; }
    static void SetModel(DPAModelType model) { fCurrentModel = model; }
    
    // 模型描述
    static G4String GetModelDescription(DPAModelType model) {
        switch (model) {
            case DPAModelType::NRT:
                return "NRT (Norgett-Robinson-Torrens) Model (Recommended)";
            case DPAModelType::SRIM:
                return "SRIM/TRIM Style Model (High precision, longer computation time)";
            default:
                return "Unknown Model";
        }
    }
    
    // 模型精度评估
    static G4double GetModelAccuracy(DPAModelType model) {
        switch (model) {
            case DPAModelType::NRT:
                return 0.85; // 85% 精度
            case DPAModelType::SRIM:
                return 0.9;  // 90% 精度
            default:
                return 0.85; // 默认使用NRT精度
        }
    }
    
    // 计算复杂度评估
    static G4int GetModelComplexity(DPAModelType model) {
        switch (model) {
            case DPAModelType::NRT:
                return 3;  // 中等
            case DPAModelType::SRIM:
                return 5;  // 复杂
            default:
                return 3;  // 默认使用NRT复杂度
        }
    }

private:
    static DPAModelType fCurrentModel;
};

// 默认使用NRT模型
DPAModelType DPAModelConfig::fCurrentModel = DPAModelType::NRT;

// DPA模型选择器
class DPAModelSelector {
public:
    // 设置当前使用的模型
    static void SetModel(DPAModelType model) {
        DPAModelConfig::SetModel(model);
        G4cout << "DPA Model switched to: " << DPAModelConfig::GetModelDescription(model) << G4endl;
    }
    
    // 获取当前模型信息
    static void PrintModelInfo() {
        DPAModelType currentModel = DPAModelConfig::GetCurrentModel();
        G4cout << "Current DPA Model: " << DPAModelConfig::GetModelDescription(currentModel) << G4endl;
        G4cout << "Accuracy: " << DPAModelConfig::GetModelAccuracy(currentModel) * 100 << "%" << G4endl;
        G4cout << "Complexity: " << DPAModelConfig::GetModelComplexity(currentModel) << "/5" << G4endl;
    }
    
    // 模型性能对比
    static void CompareModels() {
        G4cout << "=== DPA Model Comparison ===" << G4endl;
        G4cout << "NRT Model:" << G4endl;
        G4cout << "  - Accuracy: " << DPAModelConfig::GetModelAccuracy(DPAModelType::NRT) * 100 << "%" << G4endl;
        G4cout << "  - Complexity: " << DPAModelConfig::GetModelComplexity(DPAModelType::NRT) << "/5" << G4endl;
        G4cout << "  - Best for: General research, balanced accuracy and speed" << G4endl;
        
        G4cout << "SRIM Model:" << G4endl;
        G4cout << "  - Accuracy: " << DPAModelConfig::GetModelAccuracy(DPAModelType::SRIM) * 100 << "%" << G4endl;
        G4cout << "  - Complexity: " << DPAModelConfig::GetModelComplexity(DPAModelType::SRIM) << "/5" << G4endl;
        G4cout << "  - Best for: High-precision calculations, neutron-gamma composite materials" << G4endl;
    }
};

} // namespace B1

#endif // DPA_MODEL_CONFIG_HH 