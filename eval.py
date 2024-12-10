import subprocess
import pandas as pd

sim_path = "./sim"
trace_path = "../ext_traces/spice.trace"

def run_test(case):
    if case["I-D"] == "Split":
        command = [
            sim_path,
            f"-bs {case['BS']}",
            f"-is {int(case['CS'].split()[0]) * 1024}",
            f"-ds {int(case['CS'].split()[0]) * 1024}",
            f"-a {case['Ass']}",
            "-wb" if case["Write"] == "WB" else "-wt",
            "-wa" if case["Alloc"] == "WA" else "-nw",
            trace_path,
        ]
    elif case["I-D"] == "Unified":
        command = [
            sim_path,
            f"-bs {case['BS']}",
            f"-us {int(case['CS'].split()[0]) * 1024}",
            f"-a {case['Ass']}",
            "-wb" if case["Write"] == "WB" else "-wt",
            "-wa" if case["Alloc"] == "WA" else "-nw",
            trace_path,
        ]
    else:
        print("failed to handle given inst.")
        return None
    result = subprocess.run(" ".join(command), shell=True, text=True, capture_output=True)
    return result.stdout

def parse_output(output):
    lines = output.split("\n")
    parsed_data = {}
    current = 0
    for line in lines:
        if "INSTRUCTIONS" in line:
            current = 1
        elif "DATA" in line:
            current = 2
        elif "misses:" in line:
            if current == 1:
               parsed_data["I-Misses"] = int(line.split()[-1]) 
            elif current == 2:
               parsed_data["D-Misses"] = int(line.split()[-1]) 
        elif "replace:" in line:
            if current == 1:
               parsed_data["I-repl"] = int(line.split()[-1]) 
            elif current == 2:
               parsed_data["D-repl"] = int(line.split()[-1])     
        elif "demand fetch:" in line:
            parsed_data["DF"] = int(line.split()[-1])
        elif "copies back:" in line:
            parsed_data["CB"] = int(line.split()[-1])
    return parsed_data

def compare_results(expected, actual):
    keys = ["I-Misses", "I-repl", "D-Misses", "D-repl", "DF", "CB"]
    for key in keys:
        if expected[key] != actual.get(key, 0):
            return False, f"Mismatch in {key}: Expected {expected[key]}, Got {actual.get(key, 0)}"
    return True, "Pass"


if __name__ == "__main__":
    file_path = "test_cases.csv"
    df = pd.read_csv(file_path)
    
    size = df.shape[0]
    print(f"Running {size} tests")
    for i in range(size):
        print(f"TESTING {i}, ", end="")
        result = run_test(df.loc[i])
        # print(result)
        
        parsed_data = parse_output(result)
        print("GET: ", end="")
        print(parsed_data)
        
        test_result = compare_results(df.loc[i], parsed_data)
        print("RESULT: ", end="")
        print(test_result)