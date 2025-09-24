#!/usr/bin/env python3
import os
import sys
import json
import math
import itertools
import subprocess
from datetime import datetime

"""
批量配方扫描脚本：
- 输入：JSON 配置文件，示例见同目录 sweep_recipes.example.json
- 功能：
  1) 在给定元素百分比范围与步长内生成所有配方，筛选总和≈100%
  2) 写入 macros/recipes/<recipe_name>.txt
  3) 为每个“源配置”生成运行宏并执行 exampleB1，每次 beamOn = events_per_run

JSON 配置字段：
{
  "components": [
    {"name": "sio2", "min": 40, "max": 50, "step": 5},
    {"name": "al2o3", "min": 10, "max": 20, "step": 5},
    {"name": "b2O3",  "min": 10, "max": 25, "step": 5},
    {"name": "Gd2O3", "min": 5,  "max": 10, "step": 5},
    {"name": "li2o",  "min": 0,  "max": 10, "step": 5},
    {"name": "ceO2",  "min": 0,  "max": 10, "step": 5}
  ],
  "sum_tolerance": 0.5,
  "events_per_run": 1000000,
  "sources": [
    {"name": "cf252", "macro": "macros/Cf252_neutron_test.mac"},
    {"name": "gamma662keV", "macro": "macros/gamma_shielding.mac"}
  ],
  "dry_run": false
}

说明：
- 组件名称需为 Geant4 NIST 或项目中可 FindOrBuild 的材料名。
- sum_tolerance：允许配方总和与100%的偏差（%）。
- sources.macro：一个可直接喂给 exampleB1 的宏文件。
"""

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MACROS_DIR = os.path.join(ROOT_DIR, 'macros')
RECIPES_DIR = os.path.join(MACROS_DIR, 'recipes')
BUILD_EXE = os.path.join(ROOT_DIR, 'build', 'exampleB1')

def load_config(path):
    with open(path, 'r') as f:
        return json.load(f)

def frange(min_v, max_v, step):
    v = min_v
    while v <= max_v + 1e-9:
        yield round(v, 6)
        v += step

def generate_combinations(components, sum_tolerance):
    # 逐分量网格，按总和接近100筛选
    ranges = [list(frange(c['min'], c['max'], c['step'])) for c in components]
    names = [c['name'] for c in components]
    for values in itertools.product(*ranges):
        s = sum(values)
        if abs(s - 100.0) <= sum_tolerance:
            yield dict(zip(names, values))

def write_recipe(recipe_dict, tag):
    os.makedirs(RECIPES_DIR, exist_ok=True)
    fname = f"recipe_{tag}.txt"
    path = os.path.join(RECIPES_DIR, fname)
    with open(path, 'w') as f:
        for name, pct in recipe_dict.items():
            f.write(f"{name} {pct}\n")
    return path

def build_macro_for_source(base_macro_path, recipe_path, events):
    # 生成一个临时宏：先设置配方，再 include 源宏，最后覆盖 beamOn
    ts = datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    out_macro = os.path.join(MACROS_DIR, f"auto_run_{ts}.mac")
    with open(out_macro, 'w') as f:
        f.write(f"/det/glass/compositionFile {recipe_path}\n")
        # 源宏内容包含发射器等设置
        f.write(f"/control/execute {base_macro_path}\n")
        f.write(f"/run/beamOn {events}\n")
    return out_macro

def run_sim(macro_path, dry_run=False):
    if dry_run:
        print(f"[DRY] {BUILD_EXE} {macro_path}")
        # 试运行模式也删除临时文件
        try:
            os.remove(macro_path)
            print(f"[CLEAN] Removed temporary macro: {macro_path}")
        except:
            pass
        return 0
    print(f"[RUN] {BUILD_EXE} {macro_path}")
    proc = subprocess.run([BUILD_EXE, macro_path])
    # 运行完成后删除临时宏文件
    try:
        os.remove(macro_path)
        print(f"[CLEAN] Removed temporary macro: {macro_path}")
    except:
        pass
    return proc.returncode

def cleanup_old_auto_macros():
    """清理旧的auto_run_*.mac临时文件"""
    import glob
    auto_macros = glob.glob(os.path.join(MACROS_DIR, "auto_run_*.mac"))
    if auto_macros:
        print(f"[CLEAN] Found {len(auto_macros)} old temporary macro files")
        for macro in auto_macros:
            try:
                os.remove(macro)
                print(f"[CLEAN] Removed: {os.path.basename(macro)}")
            except:
                pass

def main():
    if len(sys.argv) < 2:
        print("Usage: sweep_recipes.py <config.json>")
        sys.exit(1)
    
    # 开始前清理旧的临时文件
    cleanup_old_auto_macros()
    
    cfg = load_config(sys.argv[1])

    components = cfg.get('components', [])
    tol = float(cfg.get('sum_tolerance', 0.5))
    events = int(cfg.get('events_per_run', 1000000))
    sources = cfg.get('sources', [])
    dry_run = bool(cfg.get('dry_run', False))

    combos = list(generate_combinations(components, tol))
    print(f"[INFO] Generated {len(combos)} recipes (sum within ±{tol}%)")

    if not os.path.isfile(BUILD_EXE):
        print(f"[ERROR] Not found executable: {BUILD_EXE}")
        sys.exit(2)

    for idx, recipe in enumerate(combos, 1):
        tag = f"{idx:04d}"
        recipe_path = write_recipe(recipe, tag)
        print(f"[RECIPE] {recipe_path}")
        for src in sources:
            name = src.get('name', 'src')
            macro = src.get('macro')
            if not macro or not os.path.isfile(os.path.join(ROOT_DIR, os.path.relpath(macro, ROOT_DIR)) if not os.path.isabs(macro) else macro):
                print(f"[WARN] Skip source '{name}', macro not found: {macro}")
                continue
            macro_abs = macro if os.path.isabs(macro) else os.path.join(ROOT_DIR, macro)
            auto_macro = build_macro_for_source(macro_abs, recipe_path, events)
            rc = run_sim(auto_macro, dry_run)
            if rc != 0:
                print(f"[WARN] Simulation returned non-zero ({rc}) for recipe {tag}, source {name}")

if __name__ == '__main__':
    main()


