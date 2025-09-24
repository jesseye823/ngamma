#!/usr/bin/env python3
import os
import sys
import json
import math
import itertools
import subprocess
from datetime import datetime
import shutil

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
BUILD_DIR = os.path.join(ROOT_DIR, 'build')

def rebuild():
    try:
        if not os.path.isdir(BUILD_DIR):
            os.makedirs(BUILD_DIR, exist_ok=True)
        # Prefer make if present, otherwise try cmake --build
        makefile = os.path.join(BUILD_DIR, 'Makefile')
        if os.path.isfile(makefile):
            print(f"[BUILD] make -C {BUILD_DIR}")
            rc = subprocess.call(["make", "-C", BUILD_DIR, "-j"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
            return rc == 0
        else:
            # Attempt cmake --build
            print(f"[BUILD] cmake --build {BUILD_DIR}")
            rc = subprocess.call(["cmake", "--build", BUILD_DIR, "-j"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
            return rc == 0
    except Exception as e:
        print(f"[BUILD] skipped ({e})")
        return False

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

def _analyze_base_macro(base_macro_path):
    has_gps = False
    has_source_mode = False
    has_init = False
    gps_particle = None
    try:
        with open(base_macro_path, 'r') as bf:
            for line in bf:
                if '/gps/' in line:
                    has_gps = True
                if '/source/mode' in line:
                    has_source_mode = True
                if '/run/initialize' in line:
                    has_init = True
                ls = line.strip()
                if ls.startswith('/gps/particle'):
                    parts = ls.split()
                    if len(parts) >= 2:
                        gps_particle = parts[1]
    except Exception:
        pass
    return has_gps, has_source_mode, has_init, gps_particle

def build_macro_for_source(base_macro_path, recipe_path, events):
    # 生成一个临时宏：
    # 1) 设置玻璃配方（在初始化前）
    # 2) 如目标宏包含GPS命令但未显式设置/source/mode，则强制设为gps
    # 3) include 源宏
    # 4) 覆盖 beamOn 事件数
    ts = datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    out_macro = os.path.join(MACROS_DIR, f"auto_run_{ts}.mac")
    has_gps, has_source_mode, has_init, gps_particle = _analyze_base_macro(base_macro_path)
    need_set_gps_mode = has_gps  # 总是覆盖为gps，避免被基宏或环境覆盖
    with open(out_macro, 'w') as f:
        # 1) 先设配方与源模式
        f.write(f"/det/glass/compositionFile {recipe_path}\n")
        if has_gps:
            f.write("/source/mode gps\n")
        # 2) 写入“过滤后的”基宏内容（去掉 /run/initialize、/run/beamOn、/det/glass/compositionFile）
        try:
            with open(base_macro_path, 'r') as bf:
                for line in bf:
                    ls = line.strip()
                    if not ls:
                        f.write(line)
                        continue
                    if ls.startswith('/run/initialize'):
                        continue
                    if ls.startswith('/run/beamOn'):
                        continue
                    if ls.startswith('/det/glass/compositionFile'):
                        continue
                    # 不过滤 /source/mode，后面我们会重新设置一次
                    f.write(line)
        except Exception:
            # 退化为直接include（不理想，但避免中断）
            f.write(f"/control/execute {base_macro_path}\n")
        # 3) 再次确保源模式与粒子名
        if has_gps:
            f.write("/source/mode gps\n")
            if gps_particle:
                f.write(f"/gps/particle {gps_particle}\n")
        # 4) 最后统一初始化和beamOn
        f.write("/run/initialize\n")
        f.write(f"/run/beamOn {events}\n")
    return out_macro

def run_sim(macro_path, base_macro_path, dry_run=False):
    if dry_run:
        print(f"[DRY] {BUILD_EXE} {macro_path}")
        # 试运行模式也删除临时文件
        try:
            os.remove(macro_path)
            print(f"[CLEAN] Removed temporary macro: {macro_path}")
        except:
            pass
        return 0
    # 推断是否需要强制 GPS 环境，防止外部环境变量覆盖
    has_gps, has_source_mode, _, _ = _analyze_base_macro(base_macro_path)
    env = os.environ.copy()
    if has_gps and not has_source_mode:
        env['NGAMMA_SOURCE_MODE'] = 'gps'
    # Persist a copy of the generated macro for debugging
    try:
        ts = datetime.now().strftime('%Y%m%d_%H%M%S_%f')
        dbg_macro = os.path.join(os.path.dirname(__file__), 'web_jobs', f'used_macro_{ts}.mac')
        shutil.copyfile(macro_path, dbg_macro)
        print(f"[MACRO] Saved a copy to {dbg_macro}")
    except Exception as e:
        print(f"[MACRO] copy failed: {e}")
    # Log detected settings
    print(f"[CFG] base_macro={base_macro_path}")
    print(f"[CFG] env.NGAMMA_SOURCE_MODE={env.get('NGAMMA_SOURCE_MODE')}")
    print(f"[RUN] {BUILD_EXE} {macro_path}")
    proc = subprocess.run([BUILD_EXE, macro_path], env=env)
    # 运行完成后删除临时宏文件
    # Keep the macro for post-mortem; do not delete
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
    # Always try to rebuild to pick up latest code changes
    rebuild()
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
            rc = run_sim(auto_macro, macro_abs, dry_run)
            if rc != 0:
                print(f"[WARN] Simulation returned non-zero ({rc}) for recipe {tag}, source {name}")

if __name__ == '__main__':
    main()


