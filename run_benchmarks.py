import subprocess
import time
import re
import matplotlib.pyplot as plt
import os
import signal

def run_and_capture(command, cwd, duration=6):
    print(f"🚀 Running: {' '.join(command)}")
    
    # Copy the OS environment and inject PYTHONPATH
    my_env = os.environ.copy()
    my_env["PYTHONPATH"] = "."

    process = subprocess.Popen(
        command, 
        cwd=cwd,
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE,
        text=True,
        env=my_env 
    )
    
    time.sleep(duration)
    
    process.send_signal(signal.SIGINT)
    process.terminate()
    try:
        outs, errs = process.communicate(timeout=2)
    except:
        process.kill()
        outs, errs = process.communicate()
        
    rates = []
    for line in outs.split('\n'):
        match = re.search(r'Active Rate:\s*([\d,]+)\s*msgs/sec', line)
        if match:
            rate = int(match.group(1).replace(',', ''))
            rates.append(rate)
            
    if not rates:
        print("❌ Failed to capture data.")
        print(f"   Crash Log: {errs.strip()}")
        return [0]
        
    valid_rates = rates[1:] if len(rates) > 2 else rates
    avg_rate = sum(valid_rates) / len(valid_rates)
    print(f"✅ Captured {len(valid_rates)} ticks. Avg: {avg_rate:,.0f} msgs/sec\n")
    return valid_rates

def main():
    print("--- ZPC-IPC AUTOMATED BENCHMARK SUITE ---\n")
    
    baseline_rates = run_and_capture(['python3', '-u', 'benchmarks/baseline.py'], cwd='.', duration=6)

    print("⚙️ Starting C++ Zero-Copy Writer...")
    writer = subprocess.Popen([ './writer'], cwd='./build', stdout=subprocess.DEVNULL)
    time.sleep(1) 
    
    cpp_rates = run_and_capture(['./reader'], cwd='./build', duration=6)
    zpc_rates = run_and_capture(['python3', '-u', '../tests/read_shm.py'], cwd='./build', duration=6)

    writer.terminate()
    print("🛑 Benchmarks complete. Generating Time-Series Line Graph...")

    # Passes the duration to the graphing function
    generate_line_graph(baseline_rates, zpc_rates, cpp_rates, duration=6.0)

# The updated function that accepts 'duration' and scales the X-axis
def generate_line_graph(baseline, zpc, cpp, duration=6.0):
    plt.style.use('dark_background')
    fig, ax = plt.subplots(figsize=(12, 7))
    
    # Normalize the X-axis to represent actual Time (Seconds)
    x_base = [i * (duration / len(baseline)) for i in range(1, len(baseline) + 1)]
    x_zpc = [i * (duration / len(zpc)) for i in range(1, len(zpc) + 1)]
    x_cpp = [i * (duration / len(cpp)) for i in range(1, len(cpp) + 1)]

    # Plot the lines
    ax.plot(x_cpp, cpp, color='#b84dff', linewidth=3, marker='^', markersize=8, label='Pure C++ Native (Hardware Limit)')
    ax.plot(x_zpc, zpc, color='#00ffcc', linewidth=3, label='zpc-ipc Python Bridge (NumPy)')
    ax.plot(x_base, baseline, color='#ff3366', linewidth=3, marker='o', markersize=8, label='Standard OS Pipes (multiprocessing)')
    
    # Formatting
    ax.set_title('ZPC-IPC: Sustained Throughput Over Time', fontsize=18, pad=20, fontweight='bold', color='white')
    ax.set_xlabel('Time (Seconds)', fontsize=12, color='#aaaaaa')
    ax.set_ylabel('Messages Per Second (Log Scale)', fontsize=12, color='#aaaaaa')
    
    ax.set_yscale('log')
    
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_color('#555555')
    ax.spines['bottom'].set_color('#555555')
    ax.grid(axis='y', color='#333333', linestyle='--', alpha=0.7)
    
    ax.legend(loc='center right', frameon=True, facecolor='#111111', edgecolor='#333333', fontsize=11)
    ax.text(0.5, -0.12, '*Note: Y-Axis is Logarithmic. ZPC-IPC is significantly faster than Standard OS Pipes.', 
            ha='center', va='center', transform=ax.transAxes, color='#aaaaaa', fontsize=10, style='italic')

    plt.tight_layout()
    plt.savefig('final_benchmark_line.png', dpi=300, bbox_inches='tight')
    print("📈 Saved elegant line graph to 'final_benchmark_line.png'.")

if __name__ == "__main__":
    main()