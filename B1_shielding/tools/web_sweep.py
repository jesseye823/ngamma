#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import uuid
import time
import threading
import subprocess
from datetime import datetime
from flask import Flask, request, jsonify, send_from_directory, Response

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SWEEP_SCRIPT = os.path.join(ROOT_DIR, 'tools', 'sweep_recipes.py')
JOBS_DIR = os.path.join(ROOT_DIR, 'tools', 'web_jobs')
os.makedirs(JOBS_DIR, exist_ok=True)

app = Flask(__name__)

jobs = {}
# jobs[job_id] = {
#   'config_path': str,
#   'log_path': str,
#   'proc': Popen or None,
#   'start_time': str,
#   'end_time': str or None,
#   'returncode': int or None
# }


INDEX_HTML = r"""
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>配方批量扫描 - Web</title>
  <style>
    body { font-family: sans-serif; margin: 12px; }
    fieldset { margin-bottom: 12px; }
    input[type=text], input[type=number] { padding: 4px; }
    table { border-collapse: collapse; }
    td, th { border: 1px solid #ddd; padding: 6px; }
    .btn { padding: 6px 10px; margin: 4px; }
    .row { margin: 6px 0; }
    .logbox { white-space: pre-wrap; background: #111; color: #0f0; padding: 8px; height: 300px; overflow: auto; }
  </style>
  <script>
    function addComponentRow(name='', mi=0, ma=100, st=5){
      const tbody = document.getElementById('components');
      const tr = document.createElement('tr');
      tr.innerHTML = `<td><input type="text" value="${name}"/></td>
                      <td><input type="number" step="0.1" value="${mi}"/></td>
                      <td><input type="number" step="0.1" value="${ma}"/></td>
                      <td><input type="number" step="0.1" value="${st}"/></td>
                      <td><button onclick="this.parentNode.parentNode.remove()">删除</button></td>`;
      tbody.appendChild(tr);
    }
    function addSourceRow(name='', macro=''){
      const tbody = document.getElementById('sources');
      const tr = document.createElement('tr');
      tr.innerHTML = `<td><input type="text" value="${name}"/></td>
                      <td><input type="text" size="60" value="${macro}"/></td>
                      <td><button onclick="this.parentNode.parentNode.remove()">删除</button></td>`;
      tbody.appendChild(tr);
    }
    function fillExample(){
      document.getElementById('components').innerHTML='';
      addComponentRow('sio2',40,50,5);
      addComponentRow('al2o3',10,20,5);
      addComponentRow('b2O3',10,25,5);
      addComponentRow('Gd2O3',5,10,5);
      addComponentRow('li2o',0,10,5);
      addComponentRow('ceO2',0,10,5);
      document.getElementById('sources').innerHTML='';
      addSourceRow('cf252_neutron','macros/Cf252_neutron_test.mac');
      addSourceRow('am241_gamma','macros/Am241_gamma_point.mac');
    }
    async function loadRecipes(){
      const resp = await fetch('/recipes');
      const data = await resp.json();
      const sel = document.getElementById('recipeSel');
      sel.innerHTML = '';
      data.forEach(r=>{
        const opt = document.createElement('option');
        opt.value = r.path;
        opt.textContent = r.name;
        opt.dataset.components = JSON.stringify(r.components);
        sel.appendChild(opt);
      });
    }
    function applySelectedRecipe(){
      const sel = document.getElementById('recipeSel');
      const opt = sel.options[sel.selectedIndex];
      if(!opt){ alert('无可用配方'); return; }
      const comps = JSON.parse(opt.dataset.components || '[]');
      if(!comps.length){ alert('配方为空'); return; }
      document.getElementById('components').innerHTML='';
      comps.forEach(c=>{ addComponentRow(c.name, c.pct, c.pct, 1); });
    }
    async function startRun(){
      const tol = parseFloat(document.getElementById('tol').value);
      const events = parseInt(document.getElementById('events').value);
      const dry = document.getElementById('dry').checked;
      const comps=[]; const srcs=[];
      document.querySelectorAll('#components tr').forEach(tr=>{
        const tds = tr.querySelectorAll('td');
        const name = tds[0].querySelector('input').value.trim();
        const mi = parseFloat(tds[1].querySelector('input').value);
        const ma = parseFloat(tds[2].querySelector('input').value);
        const st = parseFloat(tds[3].querySelector('input').value);
        if(name){ comps.push({name:name, min:mi, max:ma, step:st}); }
      });
      document.querySelectorAll('#sources tr').forEach(tr=>{
        const tds = tr.querySelectorAll('td');
        const name = tds[0].querySelector('input').value.trim();
        const macro = tds[1].querySelector('input').value.trim();
        if(name && macro){ srcs.push({name:name, macro:macro}); }
      });
      const body = { components: comps, sum_tolerance: tol, events_per_run: events, sources: srcs, dry_run: dry };
      const resp = await fetch('/start', { method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify(body)});
      const data = await resp.json();
      if(!resp.ok){ alert(data.error||'启动失败'); return; }
      document.getElementById('jobid').textContent = data.job_id;
      window.currentJob = data.job_id;
      pollStatus();
    }
    async function pollStatus(){
      if(!window.currentJob) return;
      const resp = await fetch('/status/'+window.currentJob);
      const data = await resp.json();
      document.getElementById('log').textContent = data.log || '';
      document.getElementById('rc').textContent = data.returncode===null? '运行中' : data.returncode;
      if(data.returncode===null){ setTimeout(pollStatus, 1000); }
    }
  </script>
</head>
<body>
  <h2>配方批量扫描 - Web</h2>

  <fieldset>
    <legend>全局参数</legend>
    <div class="row">总和容差(%) <input id="tol" type="number" step="0.1" value="0.5" />
      &nbsp;&nbsp; 每次事件数 <input id="events" type="number" value="1000000" />
      &nbsp;&nbsp; <label><input id="dry" type="checkbox" checked /> 试运行</label>
    </div>
  </fieldset>

  <fieldset>
    <legend>组件(材料 与 百分比范围)</legend>
    <div class="row">
      <button class="btn" onclick="loadRecipes()">刷新已有配方</button>
      <select id="recipeSel" style="min-width:300px;">
        <option value="">(请选择已有配方)</option>
      </select>
      <button class="btn" onclick="applySelectedRecipe()">加载所选配方到组件</button>
    </div>
    <table>
      <thead><tr><th>材料名</th><th>最小%</th><th>最大%</th><th>步长%</th><th>操作</th></tr></thead>
      <tbody id="components"></tbody>
    </table>
    <button class="btn" onclick="addComponentRow()">添加组件</button>
    <button class="btn" onclick="fillExample()">示例(玻璃)</button>
  </fieldset>

  <fieldset>
    <legend>源配置</legend>
    <table>
      <thead><tr><th>源名称</th><th>宏路径</th><th>操作</th></tr></thead>
      <tbody id="sources"></tbody>
    </table>
    <button class="btn" onclick="addSourceRow()">添加源</button>
  </fieldset>

  <div class="row">
    <button class="btn" onclick="startRun()">开始扫描</button>
    <span>JobID: <b id="jobid">-</b> &nbsp; 返回码: <b id="rc">-</b></span>
  </div>

  <div class="row">
    <div class="logbox" id="log"></div>
  </div>

  <script>fillExample();</script>
</body>
</html>
"""


def _run_job(job_id, cfg):
    cfg_path = os.path.join(JOBS_DIR, f"{job_id}.json")
    log_path = os.path.join(JOBS_DIR, f"{job_id}.log")
    with open(cfg_path, 'w') as f:
        json.dump(cfg, f, indent=2)

    jobs[job_id] = {
        'config_path': cfg_path,
        'log_path': log_path,
        'proc': None,
        'start_time': datetime.now().isoformat(timespec='seconds'),
        'end_time': None,
        'returncode': None
    }

    cmd = ["python3", SWEEP_SCRIPT, cfg_path]
    with open(log_path, 'w') as lf:
        lf.write("[RUN] " + " ".join(cmd) + "\n")
        lf.flush()
        try:
            proc = subprocess.Popen(cmd, cwd=ROOT_DIR, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            jobs[job_id]['proc'] = proc
            for line in proc.stdout:
                lf.write(line)
                lf.flush()
            rc = proc.wait()
            jobs[job_id]['returncode'] = rc
        except Exception as e:
            lf.write(f"[ERROR] {e}\n")
            jobs[job_id]['returncode'] = -1
        finally:
            jobs[job_id]['end_time'] = datetime.now().isoformat(timespec='seconds')


@app.get('/')
def index():
    return Response(INDEX_HTML, mimetype='text/html')


@app.post('/start')
def start():
    try:
        cfg = request.get_json(force=True)
        # 简单校验
        assert isinstance(cfg.get('components'), list) and len(cfg['components']) > 0
        assert isinstance(cfg.get('sources'), list) and len(cfg['sources']) > 0
        float(cfg.get('sum_tolerance', 0.5))
        int(cfg.get('events_per_run', 1000000))
    except Exception as e:
        return jsonify({ 'error': f'参数错误: {e}' }), 400

    job_id = uuid.uuid4().hex[:12]
    t = threading.Thread(target=_run_job, args=(job_id, cfg), daemon=True)
    t.start()
    return jsonify({ 'job_id': job_id })


@app.get('/status/<job_id>')
def status(job_id):
    job = jobs.get(job_id)
    if not job:
        return jsonify({ 'error': 'job not found' }), 404
    log_text = ''
    if os.path.isfile(job['log_path']):
        try:
            with open(job['log_path'], 'r') as f:
                log_text = f.read()[-20000:]  # tail last 20k chars
        except Exception:
            pass
    return jsonify({
        'returncode': job['returncode'],
        'start_time': job['start_time'],
        'end_time': job['end_time'],
        'log': log_text
    })


@app.get('/recipes')
def list_recipes():
    recipes_dir = os.path.join(ROOT_DIR, 'macros', 'recipes')
    results = []
    if os.path.isdir(recipes_dir):
        for fn in sorted(os.listdir(recipes_dir)):
            if not fn.endswith('.txt'): continue
            path = os.path.join(recipes_dir, fn)
            comps = []
            try:
                with open(path, 'r') as f:
                    for line in f:
                        line=line.strip()
                        if not line or line.startswith('#'): continue
                        parts = line.split()
                        if len(parts) >= 2:
                            name = parts[0]
                            try:
                                pct = float(parts[1])
                            except:
                                continue
                            comps.append({ 'name': name, 'pct': pct })
            except Exception:
                pass
            results.append({ 'name': fn, 'path': path, 'components': comps })
    return jsonify(results)



def main():
    port = int(os.environ.get('PORT', '5000'))
    app.run(host='127.0.0.1', port=port, debug=False)


if __name__ == '__main__':
    main()


