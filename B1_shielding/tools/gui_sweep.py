#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import tempfile
import subprocess
import threading
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SWEEP_SCRIPT = os.path.join(ROOT_DIR, 'tools', 'sweep_recipes.py')


class ComponentRow:
    def __init__(self, parent, name_default="", min_default="0", max_default="100", step_default="5"):
        self.frame = ttk.Frame(parent)
        self.name = tk.StringVar(value=name_default)
        self.minv = tk.StringVar(value=min_default)
        self.maxv = tk.StringVar(value=max_default)
        self.step = tk.StringVar(value=step_default)

        ttk.Entry(self.frame, width=16, textvariable=self.name).grid(row=0, column=0, padx=2, pady=2)
        ttk.Entry(self.frame, width=8, textvariable=self.minv).grid(row=0, column=1, padx=2, pady=2)
        ttk.Entry(self.frame, width=8, textvariable=self.maxv).grid(row=0, column=2, padx=2, pady=2)
        ttk.Entry(self.frame, width=8, textvariable=self.step).grid(row=0, column=3, padx=2, pady=2)

        self.btn_del = ttk.Button(self.frame, text="删除", command=self.frame.destroy)
        self.btn_del.grid(row=0, column=4, padx=2, pady=2)

    def grid(self, **kwargs):
        self.frame.grid(**kwargs)

    def to_dict(self):
        try:
            return {
                "name": self.name.get().strip(),
                "min": float(self.minv.get()),
                "max": float(self.maxv.get()),
                "step": float(self.step.get()),
            }
        except Exception:
            return None


class SourceRow:
    def __init__(self, parent, name_default="", macro_default=""):
        self.frame = ttk.Frame(parent)
        self.name = tk.StringVar(value=name_default)
        self.macro = tk.StringVar(value=macro_default)

        ttk.Entry(self.frame, width=16, textvariable=self.name).grid(row=0, column=0, padx=2, pady=2)
        ttk.Entry(self.frame, width=48, textvariable=self.macro).grid(row=0, column=1, padx=2, pady=2)
        ttk.Button(self.frame, text="浏览", command=self.browse).grid(row=0, column=2, padx=2, pady=2)
        self.btn_del = ttk.Button(self.frame, text="删除", command=self.frame.destroy)
        self.btn_del.grid(row=0, column=3, padx=2, pady=2)

    def browse(self):
        path = filedialog.askopenfilename(initialdir=os.path.join(ROOT_DIR, 'macros'), title='选择宏文件 (.mac)', filetypes=[('Macro', '*.mac'), ('All', '*.*')])
        if path:
            rel = os.path.relpath(path, ROOT_DIR)
            self.macro.set(rel if not rel.startswith('..') else path)

    def grid(self, **kwargs):
        self.frame.grid(**kwargs)

    def to_dict(self):
        name = self.name.get().strip()
        macro = self.macro.get().strip()
        if not name or not macro:
            return None
        return {"name": name, "macro": macro}


class SweepGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("配方批量扫描 - GUI")
        self.geometry("900x620")

        # 顶部参数
        top = ttk.Frame(self)
        top.pack(fill=tk.X, padx=8, pady=6)

        ttk.Label(top, text="总和容差(%)").grid(row=0, column=0, sticky=tk.W, padx=4)
        self.tol = tk.StringVar(value="0.5")
        ttk.Entry(top, width=8, textvariable=self.tol).grid(row=0, column=1, padx=4)

        ttk.Label(top, text="每次事件数").grid(row=0, column=2, sticky=tk.W, padx=12)
        self.events = tk.StringVar(value="1000000")
        ttk.Entry(top, width=12, textvariable=self.events).grid(row=0, column=3, padx=4)

        self.dry_run = tk.BooleanVar(value=True)
        ttk.Checkbutton(top, text="试运行(不真正执行)", variable=self.dry_run).grid(row=0, column=4, padx=12)

        # 组件表头
        comp_frame = ttk.LabelFrame(self, text="组件(材料名 与 百分比范围)")
        comp_frame.pack(fill=tk.X, padx=8, pady=6)
        header = ttk.Frame(comp_frame)
        header.pack(fill=tk.X)
        ttk.Label(header, text="材料名", width=16).grid(row=0, column=0)
        ttk.Label(header, text="最小%", width=8).grid(row=0, column=1)
        ttk.Label(header, text="最大%", width=8).grid(row=0, column=2)
        ttk.Label(header, text="步长%", width=8).grid(row=0, column=3)

        self.comp_rows_container = ttk.Frame(comp_frame)
        self.comp_rows_container.pack(fill=tk.X)
        self.comp_rows = []

        def add_comp(name="", mi="0", ma="100", st="5"):
            row = ComponentRow(self.comp_rows_container, name, mi, ma, st)
            row.grid(sticky=tk.W)
            self.comp_rows.append(row)

        btns = ttk.Frame(comp_frame)
        btns.pack(fill=tk.X)
        ttk.Button(btns, text="添加组件", command=lambda: add_comp()).pack(side=tk.LEFT, padx=4, pady=4)
        ttk.Button(btns, text="示例(玻璃)", command=lambda: self.fill_example_components()).pack(side=tk.LEFT, padx=4, pady=4)

        # 源配置
        src_frame = ttk.LabelFrame(self, text="源配置")
        src_frame.pack(fill=tk.X, padx=8, pady=6)
        src_header = ttk.Frame(src_frame)
        src_header.pack(fill=tk.X)
        ttk.Label(src_header, text="源名称", width=16).grid(row=0, column=0)
        ttk.Label(src_header, text="宏路径", width=48).grid(row=0, column=1)

        self.src_rows_container = ttk.Frame(src_frame)
        self.src_rows_container.pack(fill=tk.X)
        self.src_rows = []

        def add_src(name="", macro=""):
            row = SourceRow(self.src_rows_container, name, macro)
            row.grid(sticky=tk.W)
            self.src_rows.append(row)

        src_btns = ttk.Frame(src_frame)
        src_btns.pack(fill=tk.X)
        ttk.Button(src_btns, text="添加源", command=lambda: add_src()).pack(side=tk.LEFT, padx=4, pady=4)
        ttk.Button(src_btns, text="示例(Cf-252)", command=lambda: add_src("cf252", os.path.join('macros','Cf252_neutron_test.mac'))).pack(side=tk.LEFT, padx=4, pady=4)

        # 底部控制
        bottom = ttk.Frame(self)
        bottom.pack(fill=tk.BOTH, expand=True, padx=8, pady=6)
        self.log = tk.Text(bottom, height=12)
        self.log.pack(fill=tk.BOTH, expand=True)

        run_bar = ttk.Frame(self)
        run_bar.pack(fill=tk.X, padx=8, pady=6)
        ttk.Button(run_bar, text="开始扫描", command=self.start_sweep).pack(side=tk.LEFT, padx=4)
        ttk.Button(run_bar, text="退出", command=self.destroy).pack(side=tk.RIGHT, padx=4)

    def append_log(self, text):
        self.log.insert(tk.END, text + "\n")
        self.log.see(tk.END)

    def fill_example_components(self):
        for r in self.comp_rows:
            r.frame.destroy()
        self.comp_rows.clear()
        samples = [
            ("sio2", "40", "50", "5"),
            ("al2o3", "10", "20", "5"),
            ("b2O3",  "10", "25", "5"),
            ("Gd2O3", "5",  "10", "5"),
            ("li2o",  "0",  "10", "5"),
            ("ceO2",  "0",  "10", "5"),
        ]
        for item in samples:
            row = ComponentRow(self.comp_rows_container, *item)
            row.grid(sticky=tk.W)
            self.comp_rows.append(row)

    def build_config(self):
        try:
            tol = float(self.tol.get())
            events = int(self.events.get())
        except Exception:
            messagebox.showerror("错误", "容差或事件数格式不正确")
            return None

        comps = []
        for r in self.comp_rows:
            d = r.to_dict()
            if not d or not d['name']:
                continue
            if d['min'] > d['max'] or d['step'] <= 0:
                messagebox.showerror("错误", f"组件 {d['name']} 的范围或步长有误")
                return None
            comps.append(d)
        if not comps:
            messagebox.showerror("错误", "请至少添加一个组件")
            return None

        srcs = []
        for s in self.src_rows:
            d = s.to_dict()
            if not d:
                continue
            macro = d['macro']
            macro_abs = macro if os.path.isabs(macro) else os.path.join(ROOT_DIR, macro)
            if not os.path.isfile(macro_abs):
                messagebox.showwarning("警告", f"未找到源宏: {macro}")
            srcs.append(d)
        if not srcs:
            messagebox.showerror("错误", "请至少添加一个源配置")
            return None

        return {
            "components": comps,
            "sum_tolerance": tol,
            "events_per_run": events,
            "sources": srcs,
            "dry_run": self.dry_run.get(),
        }

    def start_sweep(self):
        cfg = self.build_config()
        if not cfg:
            return

        # 写临时配置文件
        tmpdir = tempfile.mkdtemp(prefix="sweep_cfg_")
        cfg_path = os.path.join(tmpdir, 'config.json')
        with open(cfg_path, 'w') as f:
            json.dump(cfg, f, indent=2)

        # 后台线程执行
        def run():
            self.append_log(f"[INFO] 配置保存: {cfg_path}")
            cmd = ["python3", SWEEP_SCRIPT, cfg_path]
            self.append_log("[RUN] " + " ".join(cmd))
            try:
                proc = subprocess.Popen(cmd, cwd=ROOT_DIR, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                for line in proc.stdout:
                    self.append_log(line.rstrip())
                rc = proc.wait()
                self.append_log(f"[DONE] 返回码: {rc}")
                if rc != 0:
                    messagebox.showwarning("完成", f"扫描结束，返回码: {rc}")
                else:
                    messagebox.showinfo("完成", "扫描结束")
            except Exception as e:
                self.append_log(f"[ERROR] {e}")
                messagebox.showerror("错误", str(e))

        threading.Thread(target=run, daemon=True).start()


if __name__ == '__main__':
    app = SweepGUI()
    app.mainloop()


