import matplotlib.pyplot as plt
from pathlib import Path
from typing import List

num_testcase = 100
working_dir = Path.cwd() / "tools"

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

def load_values(key: str) -> List:
    res = []
    for seed in range(num_testcase):    
        log_file = working_dir / "log" / f"{seed:04}.txt"
        res.append(get_value_from_log(log_file)[key])
    return res
    
def plot(key_x: str, key_y: str):
    x = load_values(key_x)
    y = load_values(key_y)
    plt.figure()
    plt.scatter(x, y)
    plt.xlabel(key_x)
    plt.ylabel(key_y)
    plt.savefig(f'{key_x}-{key_y}.png')

def hist(key_x: str):
    x = load_values(key_x)
    plt.figure()
    plt.hist(x)
    plt.xlabel(key_x)
    plt.savefig(f'{key_x}-hist.png')


if __name__ == "__main__":
    hist('Score')