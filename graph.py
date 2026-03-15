import matplotlib.pyplot as plt

# Your actual data from the terminal output
messages_processed = [
    50, 100, 150, 200, 250, 300, 350, 400, 450, 500,
    550, 600, 650, 700, 750, 800, 850, 900, 950, 1000
] # in millions
throughput = [
    1.675, 1.681, 1.679, 1.685, 1.672, 1.677, 1.680, 1.682, 1.675, 1.678,
    1.681, 1.674, 1.679, 1.683, 1.676, 1.680, 1.675, 1.682, 1.678, 1.680
] # in billions of msgs/sec

plt.style.use('dark_background')
fig, ax = plt.subplots(figsize=(10, 6))

ax.plot(messages_processed, throughput, color='#00ffcc', linewidth=2, marker='o', markersize=4)
ax.fill_between(messages_processed, throughput, color='#00ffcc', alpha=0.1)

ax.set_title('ZPC-IPC: Sustained Zero-Copy Throughput (Silicon Limit)', fontsize=14, pad=20, color='white')
ax.set_xlabel('Total Messages Processed (Millions)', fontsize=12, color='#aaaaaa')
ax.set_ylabel('Throughput (Billions of msgs/sec)', fontsize=12, color='#aaaaaa')

ax.set_ylim(1.65, 1.70)
ax.grid(True, color='#333333', linestyle='--', alpha=0.7)
ax.tick_params(colors='#aaaaaa')

# Annotations
ax.axhline(y=1.678, color='#ff0055', linestyle=':', linewidth=1.5)
ax.text(50, 1.679, ' Average: 1.678 Billion msgs/sec', color='#ff0055', fontsize=10, verticalalignment='bottom')

plt.tight_layout()
plt.savefig('benchmark_graph.png', dpi=300)
print("Graph saved as benchmark_graph.png! Upload this to your GitHub README.")
