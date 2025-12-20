#!/usr/bin/env python3

import math
import os
import glob
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime
import sys

# Config
EXE_PATHS = {"EISm": "build/EISm", 
             "NIS": "build/NIS",
             "EIS": "build/EIS",
             "3ES": "build/3ES",
             "stats": "build/stats"}
SPACE_VALUES = [500,1000,2000,4000,8000,16000,32000,64000,128000,256000]
REPS = 100
MAX_THREADS = 30


def collectInputFiles(search_dir = "data/"):
    files = []
    if not os.path.exists(search_dir):
        search_dir = f"../{search_dir}"
        
    patterns = sys.argv[1:]
    if not patterns:
        patterns = ["*"]

    for pattern in patterns:
        found = glob.glob(os.path.join(search_dir, pattern))
        if not found:
            found = glob.glob(os.path.join(search_dir, f"*{pattern}*"))
        files.extend(found)
        
    return sorted(list(set([f for f in files if os.path.isfile(f)])))


def run(command, output_dir, run_name="run1"):
    """
    Runs the command and writes the output to the output_dir
    :param command: Command string to execute
    :param output_dir: Directory where the output file will be stored
    :param run_name: Identifier for the output file name
    """

    print(f"Running {run_name}    --   {command}")
    # Output file path
    output_file = os.path.join(output_dir ,f"{run_name}.out")

    # Run the executable with the input file
    with open(output_file, 'w') as out_f:
        try:
            result = subprocess.run(command, shell=True, stdout=out_f, stderr=subprocess.PIPE, text=True)
            if result.stderr:
                print(f"Error running {run_name}: {result.stderr}")
        except Exception as e:
            print(f"Failed to run {run_name}: {e}")
            
    print(f"Finished {run_name}")


def run_experiment():
    
    identifier = 1
    while os.path.exists(f"out/exp_{identifier}/"):
        identifier += 1
    output_folder = f"out/exp_{identifier}/"
    
    input_files = collectInputFiles()
    print(f"Selected {len(input_files)} instances: {','.join(input_files)}")
    
    selected_algos = [exe for exe, path in EXE_PATHS.items() if os.path.exists(path)]
    print(f"Selected {len(selected_algos)} algorithms: {','.join(selected_algos)}")

    completed_runs=0
    
    if len(input_files) * len(selected_algos)==0:
        exit()
    
    os.makedirs(output_folder, exist_ok=True)
    
    
    # Run instances in parallel
    with ThreadPoolExecutor(max_workers=MAX_THREADS) as executor:
        futures = []
        for input_file in input_files:
            base_name = os.path.basename(input_file)
            for k in SPACE_VALUES:
                run_name = f"{base_name}_{k}"
                if "EIS" in selected_algos:
                    cmd = f"{EXE_PATHS['EIS']} {input_file} -k {k} -r {REPS}"
                    futures.append(executor.submit(run, cmd, output_folder, run_name + "_EIS"))
                if "NIS" in selected_algos:
                    cmd = f"{EXE_PATHS['NIS']} {input_file} -k {k} -r {REPS}"
                    futures.append(executor.submit(run, cmd, output_folder, run_name + "_NIS"))
                if "3ES" in selected_algos:
                    cmd = f"{EXE_PATHS['3ES']} {input_file} -k {k} -r {REPS}"
                    futures.append(executor.submit(run, cmd, output_folder, run_name + "_3ES"))
                if "EISm" in selected_algos:
                    for s in [2,4,8,16,32,64]:
                        run_name_s = f"{base_name}_{k}_{s}-EIS"
                        cmd = f"{EXE_PATHS['EISm']} {input_file} -k {k} -r {REPS} -s {s}"
                        futures.append(executor.submit(run, cmd, output_folder, run_name_s))
            
            #Stats run
            run_name = f"{base_name}_stats"
            cmd = f"{EXE_PATHS['stats']} {input_file}"
            futures.append(executor.submit(run, cmd, output_folder, run_name))
            
                                
        print(f"Total runs to be started: {len(futures)}")
        # Wait for all futures to complete
        for future in as_completed(futures):
            result = future.result()
            completed_runs += 1
            print(f"{completed_runs}/{len(futures)} completed.") 
        

    print("All runs completed.")

run_experiment()