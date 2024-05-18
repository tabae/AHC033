import os
import joblib
import time
import subprocess
import statistics
import numpy as np
import optuna
from pathlib import Path
from typing import Tuple, List
from enum import Enum

num_testcases = 100
num_parallel = 4

working_dir = Path.cwd() / "tools"
solver = working_dir / "solver"

class ResultType(Enum):
    SUCCESS = 1
    SOLVE_FAILED = 2
    JUDGE_FAILED = 3
    
def is_int(s: str) -> bool:
    try:
        int(s)
    except ValueError:
        return False
    return True

def is_float(s: str) -> bool:
    try:
        float(s)
    except ValueError:
        return False
    return True

def get_value_from_log(log_file: Path):
    res = {}
    with log_file.open('r') as f:
        lines = f.readlines()
        for line in lines:
            if '=' in line and len(line.split('=')) == 2:
                k = line.split('=')[0].strip()
                v = line.split('=')[1].strip()
                if is_int(v):
                    v = int(v)
                elif is_float(v):
                    v = float(v)
                res[k] = v
    return res
    
def run(seed: int, options: List = []) -> Tuple[ResultType, float, float]:
    print(f"Running seed={seed}")
    # 入力ファイル、出力ファイル、ログファイルの場所を指定する
    input_file = working_dir / "in" / f"{seed:04}.txt"
    output_file = working_dir / "out" / f"{seed:04}.txt"
    log_file = working_dir / "log" / f"{seed:04}.txt"
    # ディレクトリがなければ作る
    output_file.parent.mkdir(parents=True, exist_ok=True)
    log_file.parent.mkdir(parents=True, exist_ok=True)
    # 実行コマンドとジャッジコマンドを指定する
    solve_cmd = f"cd {solver.parent} && ./{solver.name} {' '.join(map(str, options))} < {input_file} 1> {output_file} 2> {log_file}"
    judge_cmd = f"cd {working_dir} && cargo run --release --bin vis {input_file} {output_file}"
    # 実行する
    start_time = time.time()
    return_code = os.system(solve_cmd)
    end_time = time.time()
    elapsed_time = end_time - start_time
    if return_code != 0:
        return ResultType.SOLVE_FAILED, 0, elapsed_time
    # ジャッジする
    p = subprocess.Popen(judge_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    with open(log_file, mode='ab') as f:
        f.write(stdout) 
        f.write(stderr) 
    log_dict = get_value_from_log(log_file)
    if 'Score' in log_dict:
        score = get_value_from_log(log_file)['Score']
        return ResultType.SUCCESS, score, elapsed_time
    else:
        return ResultType.JUDGE_FAILED, 0, elapsed_time
    
def measure(options: List = []):
    os.system(f"g++ -O2 -std=c++20 src/main.cpp -o {solver}")
    results = joblib.Parallel(n_jobs=num_parallel) (
        joblib.delayed(run)(seed, options) for seed in range(num_testcases)
    )
    results_summary(results)

def objective(trial: optuna.trial.Trial):
    x = trial.suggest_int("x", 10, 20)
    y = trial.suggest_int("y", 10, 100)
    results = joblib.Parallel(n_jobs=num_parallel) (
        joblib.delayed(run)(seed, [x, y]) for seed in range(num_testcases)
    )
    score = 0
    for _, s, _ in results:
        score += s
    return score
        
def optimize(options: List = []):  
    os.system(f"g++ -O2 -std=c++20 src/main.cpp -DOPTUNA -o {solver}")  
    study = optuna.create_study(
        direction="maximize",
        storage="sqlite:///ahc.db",
        study_name="tune_range",        
        load_if_exists=True,
    )
    study.optimize(objective, n_trials=100)

def results_summary(results: List[Tuple[ResultType, float, float]]):
    solver_failed = 0
    judge_failed = 0
    success = 0
    scores = []
    max_time = 0.0
    for stat, score, elapsed in results:
        if stat == ResultType.SOLVE_FAILED:
            solver_failed += 1
        elif stat == ResultType.JUDGE_FAILED:
            judge_failed += 1
        elif stat == ResultType.SUCCESS:
            success += 1
        scores.append(score)
        max_time = max(max_time, elapsed)
    print("=== Summary ===")
    print(f"- SOLVE_FAILED: {solver_failed}")
    print(f"- JUDGE_FAILED: {judge_failed}")
    print(f"- SUCCESS: {success}")
    print(f"- SCORE MEAN: {statistics.mean(scores):.4f}")
    print(f"- SCORE MIN: {min(scores):.4f} (seed={np.argmin(scores)})")
    print(f"- SCORE MAX: {max(scores):.4f} (seed={np.argmax(scores)})")
    print(f"- SCORE STDEV: {statistics.stdev(scores):.4f}")
    print(f"- MAX ELAPSED TIME: {max_time: .4f} sec")

if __name__ == "__main__":
    measure()
    #optimize()