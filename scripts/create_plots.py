import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import seaborn as sns
import pandas as pd
from glob import glob
import scienceplots

REPS=100
USE_INSTANCES_FROM_PAPER = 0

INSTANCES_FOR_RELATIVE_ERROR_PLOT = ["dblp", "trackers", "soc-LiveJournal", "orkut", "reuters", "friendster"]
INSTANCES_FOR_VARIANCE_PLOT = ["dblp", "trackers", "orkut", "friendster"]
INSTANCES_FOR_VARIANCE_IN_PARAMETER_S_PLOT = ["trackers","orkut","reuters","dblp"]
INSTANCES_FOR_TABLES = ["movielens-10m", "dblp", "reuters", "livejournal-groups", "trackers", "orkut", "pubmed", "flickr", "soc-LiveJournal","friendster"]
INSTANCE_FOR_RUNTIME_PLOT = "reuters"

# --- Configuration ---
sns.set_theme(style="whitegrid")
plt.style.use(['science'])

PALETTE = ["#9b59b6","#3498db","#f1c40f", "#2ecc71",  "#e74c3c", "#CC79A7"]
ALGO_NAMES = {
    "fleet3": r"\textsc{Fleet3}", "abacus": r"\textsc{Abacus}",
    "NIS": r"\texttt{NIS} (Ours)", "EIS": r"\texttt{EIS} (Ours)", "3ES": r"\texttt{3ES}",
    "EISm": r"\texttt{EISm} (Ours)"
}
LEGEND_ORDER = ["fleet3", "abacus", "3ES", "NIS", "EIS", "EISm"]
ALGO_COLORS = {algo: color for algo, color in zip(LEGEND_ORDER, PALETTE)}

# S-Analysis Config
S_ALGO_NAMES = {
    "EIS": r"\texttt{EIS}", "2-EIS": r"\texttt{EISm-2}", "4-EIS": r"\texttt{EISm-4}",
    "8-EIS": r"\texttt{EISm-8}", "16-EIS": r"\texttt{EISm-16}", "32-EIS": r"\texttt{EISm-32}", "64-EIS": r"\texttt{EISm-64}"
}
S_LEGEND = ["EIS", "4-EIS","8-EIS","16-EIS","32-EIS","64-EIS"]
S_PALETTE = sns.color_palette("flare_r", n_colors=len(S_LEGEND))
S_COLORS = {algo: color for algo, color in zip(S_LEGEND, S_PALETTE)}


# --- Helpers ---
def save_fig(fig, output_dir, name):
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, name), format='pdf')
    plt.close(fig)
    print(f"Saved {name}")

def get_handles(order, colors):
    algo_names = ALGO_NAMES
    if "4-EIS" in order:
        algo_names = S_ALGO_NAMES
    return [mpatches.Patch(color=colors.get(a, 'k'), label=algo_names.get(a, a)) for a in order if a in colors]

# --- Data Loading ---
def load_data(input_dir):
    data = []
    for file_path in glob(os.path.join(input_dir, "*.out")):
        try:
            parts = os.path.basename(file_path).replace('.out','').split('_')
            if len(parts) != 3: continue
            instance, k, algo = parts[0], int(parts[1]), parts[2].split('.')[0]

            with open(file_path, 'r') as f:
                lines = [l.strip() for l in f if l.strip()]
            
            # Runtime
            avg_runtime = 0.0
            rt_lines = [l for l in lines if "algowithoutio" in l] #backwards compatibility
            if rt_lines:
                val = max(float(x) for x in rt_lines[0].split() if x.replace('.','',1).isdigit())
                avg_runtime = val / REPS
            else:
                rt_lines = [l for l in lines if ("ms" in l and f"{REPS}" in l)]
                val = max(float(x) for x in rt_lines[0].split() if x.replace('.','',1).isdigit())
                avg_runtime = val / REPS

            # Estimates
            est_lines = [l.split() for l in lines if not any(c.isalpha() or c in "-," for c in l)]
            if len(est_lines) == REPS:
                for x in est_lines:
                    data.append({"instance": instance, "k": k, "algo": algo, "estimate": int(x[0]), "runtime": avg_runtime})

        except Exception: continue
    return pd.DataFrame(data)


# --- Plotting ---
def plot_relative_error(df, stats, output_dir, instances, trimming=0.0):
    rows = int(np.ceil(len(instances)/3))
    fig, axes = plt.subplots(rows, 3, figsize=(8.5, 2*rows), sharex=True, sharey=True)
    for ax, instance in zip(axes.flatten(), instances):
        idf = df[df['instance'] == instance]
        if idf.empty: continue
        for algo in LEGEND_ORDER:
            adf = idf[idf['algo'] == algo]
            if adf.empty: continue
            adf = adf.groupby('k', as_index=False).agg({
                'estimate': lambda x: x.sort_values().iloc[int(len(x)*trimming):int(len(x)*(1-trimming))].mean()
            })
            rel_err = abs(adf['estimate'] - stats[instance]['T']) / stats[instance]['T']
            ax.plot(adf['k'], rel_err, label=algo, marker='o', markersize=3, color=ALGO_COLORS[algo])
        
        ax.set_title(rf"\texttt{{{instance}}}")
        ax.set_xscale("log")
        ax.set_xticks(sorted(df['k'].unique()))
        ax.get_xaxis().set_major_formatter(plt.FuncFormatter(lambda x, _: f"{x/1000:.1f}" if x < 1000 else f"{int(x/1000)}"))
        ax.minorticks_off()
        ax.xaxis.set_tick_params(top=False, which='both')
        ax.tick_params(axis='x', labelbottom=True)
        ax.set_yticks(np.arange(0, 1.1, 0.2))
        ax.set_ylim(-0.06, 1.06)
        if ax.get_subplotspec().is_last_row(): ax.set_xlabel(r"\# stored edges $k$ ($\cdot 10^3$)", fontsize=11)
        if ax.get_subplotspec().is_first_col(): ax.set_ylabel("abs. relative error", fontsize=11)

    fig.legend(handles=get_handles(LEGEND_ORDER, ALGO_COLORS), loc='upper center', ncol=len(ALGO_COLORS), bbox_to_anchor=(0.5, 1.03))
    save_fig(fig, output_dir, f"relative_error_plot_trim{trimming}.pdf" if trimming > 0 else "relative_error_plot.pdf")

def plot_variance(df, stats, output_dir, instances, min_k=0, ylim=(-1.1, 1.0), suffix="", colors=ALGO_COLORS, order=LEGEND_ORDER):
    rows = int(np.ceil(len(instances)/2))
    cols = 2
    if suffix=="_s_analysis":
        rows = int(np.ceil(len(instances)/4))
        cols = 4
    fig, axes = plt.subplots(rows,cols, figsize=(8.5, 2*rows), sharex=True, sharey=True)
    axes = np.array(axes).flatten()
    
    for ax, instance in zip(axes, instances):
        idf = df[(df['instance'] == instance) & (df['k'] >= min_k)].copy()
        if idf.empty: continue
        idf['err'] = (idf['estimate'] - stats[instance]['T']) / stats[instance]['T']
        
        sns.boxplot(data=idf, x='k', y='err', hue='algo', ax=ax, palette=colors, width=0.8, 
                    fliersize=0.06, linewidth=0.03, order=sorted(idf['k'].unique()), hue_order=order)
        ax.legend_.remove()
        ax.set_title(rf"\texttt{{{instance}}}")
        ax.axhline(0, color='gray', linestyle='--', linewidth=0.8)
        ax.set_yticks([-1,-0.5,0,0.5,1])
        ax.set_ylim(ylim)
        
        ax.minorticks_off()
        ax.xaxis.set_tick_params(top=False, which='both')
        ax.tick_params(axis='x', labelbottom=True)
        xticks = sorted(idf['k'].unique())
        ax.set_xticks(range(len(xticks)))
        ax.set_xticklabels([f"{x/1000:.1f}" if x < 1000 else f"{int(x/1000)}" for x in xticks])

        if ax.get_subplotspec().is_last_row(): ax.set_xlabel(r"\# stored edges $k$ ($\cdot 10^3$)", fontsize=11)
        if ax.get_subplotspec().is_first_col(): ax.set_ylabel("signed relative error", fontsize=11)

    fig.legend(handles=get_handles(order, colors), loc='upper center', ncol=len(colors), bbox_to_anchor=(0.5, 1.03 if len(instances)>4 else 1.06))
    save_fig(fig, output_dir, f"variance_boxplot{suffix}.pdf")

def plot_runtime(df, stats, output_dir):
    # 1. Runtime vs m
    fig, ax = plt.subplots(figsize=(2.82, 1.7))
    k_val = 32000
    rdf = df[df['k'] == k_val].groupby(['instance', 'algo']).mean().reset_index()
    rdf['edges'] = rdf['instance'].map(lambda x: stats[x]['m'])
    if USE_INSTANCES_FROM_PAPER:
        rdf = rdf[rdf['edges']>=10**7]
    rdf = rdf.sort_values('edges')
    
    for algo in ["fleet3", "EIS", "abacus", "EISm", "3ES", "NIS"]:
        adf = rdf[rdf['algo'] == algo]
        ax.plot(adf['edges'], adf['runtime']/1000, label=algo, marker='o', markersize=3, color=ALGO_COLORS[algo])
    
    ax.set_xlabel("$m$", fontsize=9)
    ax.set_ylabel("runtime (s)", fontsize=9)
    ax.tick_params(labelsize=9)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.xaxis.set_tick_params(top=False, which='both')
    ax.yaxis.set_tick_params(right=False, which='both')
    save_fig(fig, output_dir, "runtime_by_m.pdf")

    # 2. Legend
    fig, ax = plt.subplots(figsize=(2.8, 0.2))
    ax.set_axis_off()
    fig.legend(handles=get_handles(["fleet3","NIS", "abacus", "EIS", "3ES","EISm"], ALGO_COLORS), 
               loc='upper center', ncol=3, bbox_to_anchor=(0.53, 1.13), fontsize=9)
    save_fig(fig, output_dir, "runtime_legend.pdf")

    # 3. Runtime vs k
    fig, ax = plt.subplots(figsize=(2.82, 1.7))
    rdf = df[df['instance'] == INSTANCE_FOR_RUNTIME_PLOT].groupby(['k', 'algo'], as_index=False).agg({'runtime': 'mean'}).sort_values('k')
    
    for algo in rdf['algo'].unique():
        adf = rdf[rdf['algo'] == algo]
        ax.plot(adf['k'], adf['runtime']/1000, label=algo, marker='o', markersize=3, color=ALGO_COLORS[algo])
    
    ax.set_xlabel(r"\# stored edges $k$ ($\cdot 10^3$)", fontsize=9)
    ax.set_ylabel("runtime (s)", fontsize=9)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xticks(sorted(rdf['k'].unique()))
    ax.set_xticklabels([f"{x/1000:.1f}" if x < 1000 else f"{int(x/1000)}" if x<256000 else '256' for x in sorted(rdf['k'].unique())])
    ax.tick_params(labelsize=9)
    ax.xaxis.set_tick_params(top=False, which='both')
    ax.yaxis.set_tick_params(right=False, which='both')
    save_fig(fig, output_dir, "runtime_by_stored_edges.pdf")

# --- Tables ---
def create_tables(df, stats, output_dir, instances):
    def write_tex(filename, content):
        with open(os.path.join(output_dir, filename), "w") as f: f.write(content)
    
    # Min K (Mean based)
    rows = []
    for inst in instances:
        idf = df[df['instance'] == inst]
        row = f"\\{inst:<20}"
        for err in [0.1, 0.05]:
            for algo in LEGEND_ORDER:
                adf = idf[idf['algo'] == algo].groupby('k').agg({'estimate': 'mean'}).reset_index()
                adf['rel'] = abs(adf['estimate'] - stats[inst]['T']) / stats[inst]['T']
                min_k = adf[adf['rel'] <= err]['k'].min()
                row += f" & {f'{min_k/1000:.1f}k' if pd.notna(min_k) else '–':<10}"
        rows.append(row + " \\\\")
    
    header = "\\begin{table*}[ht]\n\\centering\n\\begin{tabular}{l" + "r"*len(LEGEND_ORDER)*2 + "}\n\\toprule\nInstance & \\multicolumn{6}{c}{Error 10\\%} & \\multicolumn{6}{c}{Error 5\\%} \\\\ \n" + " & " + ' & '.join(LEGEND_ORDER) + ' & ' + ' & '.join(LEGEND_ORDER) + "\\\\ \n"
    write_tex("error_table_mean.tex", header + "\n".join(rows) + "\n\\bottomrule\n\\end{tabular}\n\\end{table*}")

    # Min K (Counter based)
    rows = []
    for inst in instances:
        idf = df[df['instance'] == inst]
        row = f"\\{inst:<20}"
        for thresh in [50, 90]:
            for algo in LEGEND_ORDER:
                adf = idf[idf['algo'] == algo]
                valid_k = []
                for k in adf['k'].unique():
                    kdf = adf[adf['k'] == k]
                    count = sum(1 for e in kdf['estimate'] if abs(e - stats[inst]['T'])/stats[inst]['T'] <= 0.1)
                    if count >= thresh: valid_k.append(k)
                min_k = min(valid_k) if valid_k else None
                row += f" & {f'{min_k/1000:.1f}k' if min_k else '–':<10}"
        rows.append(row + " \\\\")
    write_tex("error_table_individual_runs.tex", header.replace("Error 10%", "Error 10% >50%").replace("Error 5%", "Error 10% >90%") + "\n".join(rows) + "\n\\bottomrule\n\\end{tabular}\n\\end{table*}")


def get_instance_stats():
    stats={}
    for file_path in glob(os.path.join(args.input, "*stats.out")):
        filename = os.path.basename(file_path)
        instance, _ = filename.split('_')
        with open(file_path, 'r') as f:
            lines = f.readlines()
        stats[instance]={}
        stats[instance]['m'] = [int(line.strip().split(" ")[-1]) for line in lines if "m:" in line][0]
        stats[instance]['T'] = [int(line.strip().split(" ")[-1]) for line in lines if "T:" in line][0]
    return stats    
    
# --- Main ---
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Directory where .out files are stored")
    args = parser.parse_args()

    output_dir = f"plots/{args.input.split('/')[1]}"
    os.makedirs(output_dir, exist_ok=True)
    
    
    stats = get_instance_stats()
    print(stats)
    print("Collected stats")
    
    df = load_data(args.input)
    df = df[df['instance'].isin(list(stats.keys()))]
    print(df)
    print("Collected run data")
    
    if not USE_INSTANCES_FROM_PAPER:
        INSTANCES_FOR_RELATIVE_ERROR_PLOT = df['instance'].unique()
        INSTANCES_FOR_TABLES = df['instance'].unique()
        INSTANCES_FOR_VARIANCE_IN_PARAMETER_S_PLOT = [i for i in df['instance'].unique() if not df[(df['instance'] == i) & (df['algo'] == "8-EIS")].empty]
        INSTANCES_FOR_VARIANCE_PLOT = df['instance'].unique()
        INSTANCE_FOR_RUNTIME_PLOT = df['instance'].unique()[0]
    
    
    # Analysis of parameter s
    plot_variance(df, stats, output_dir, INSTANCES_FOR_VARIANCE_IN_PARAMETER_S_PLOT, min_k=32000, ylim=(-0.5, 0.5), suffix=f"_s_analysis", colors=S_COLORS, order=S_LEGEND)

    
    df = df[~df['algo'].str.contains(r'-EIS') | (df['algo'] == '32-EIS')]
    df.loc[df['algo'] == '32-EIS', 'algo'] = 'EISm'
    
    create_tables(df, stats, output_dir, INSTANCES_FOR_TABLES)
    plot_relative_error(df, stats, output_dir, INSTANCES_FOR_RELATIVE_ERROR_PLOT)
    plot_variance(df, stats, output_dir, INSTANCES_FOR_VARIANCE_PLOT, suffix="_main")
    plot_runtime(df, stats, output_dir)
