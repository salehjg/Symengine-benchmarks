import argparse
import os
import seaborn as sns
import matplotlib.pyplot as plt
import datetime
import matplotlib.dates as mdates

def plot_mem_usage(files):
    # Metrics and their subplot indices
    metrics = [
        "Memory Usage",
        "Total Allocated",
        "Total In Use",
        "Total Free",
        #"External Fragmentation"
    ]

    num_metrics = len(metrics)
    fig, axes = plt.subplots(num_metrics, 1, figsize=(12, 8), sharex=True)
    colors = sns.color_palette("RdYlBu", len(files))  # Use warmer colors
    markers = ['o', 's', 'D', '^', 'v', '<', '>', 'p', '*', 'h']

    for idx, file in enumerate(files):
        if not os.path.isfile(file):
            print(f"File {file} does not exist.")
            continue

        dates = []
        memoryUsageInGB = []
        totalAllocatedSpace = []
        totalInUseSpace = []
        totalFreeSpace = []
        externalFragmentation = []

        print(f"Processing file: {file}")

        with open(file, 'r') as f:
            for line in f:
                parts = line.strip().split(',')
                if len(parts) != 6:
                    print(f"Skipping invalid line in {file}: {line}")
                    continue
                try:
                    date_time_str, mem_usage, total_alloc, total_in_use, total_free, ext_frag = parts
                    dates.append(datetime.datetime.strptime(date_time_str, "%Y/%m/%d %H:%M:%S.%f"))
                    memoryUsageInGB.append(float(mem_usage))
                    totalAllocatedSpace.append(float(total_alloc))
                    totalInUseSpace.append(float(total_in_use))
                    totalFreeSpace.append(float(total_free))
                    externalFragmentation.append(float(ext_frag))
                except ValueError as e:
                    print(f"Error parsing line in {file}: {line} -> {e}")
                    continue

        if not dates:
            print(f"No valid data in file: {file}")
            continue

        print(f"File {file} processed successfully with {len(dates)} entries.")

        # Plot data on corresponding subplots
        axes[0].plot(dates, memoryUsageInGB, label=os.path.basename(file), color=colors[idx], marker=markers[0 % len(markers)])
        axes[1].plot(dates, totalAllocatedSpace, label=os.path.basename(file), color=colors[idx], marker=markers[1 % len(markers)])
        axes[2].plot(dates, totalInUseSpace, label=os.path.basename(file), color=colors[idx], marker=markers[2 % len(markers)])
        axes[3].plot(dates, totalFreeSpace, label=os.path.basename(file), color=colors[idx], marker=markers[3 % len(markers)])
        #axes[4].plot(dates, externalFragmentation, label=os.path.basename(file), color=colors[idx], marker=markers[4 % len(markers)])

    # Set titles, labels, and legends for subplots
    for i, ax in enumerate(axes):
        ax.set_title(metrics[i])
        ax.set_ylabel("GB")
        ax.legend(loc="best", fontsize="small")
        ax.grid(True)

    axes[-1].set_xlabel("Date-Time")  # Set xlabel only for the last subplot
    axes[-1].xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
    axes[-1].xaxis.set_major_locator(mdates.AutoDateLocator())
    plt.setp(axes[-1].xaxis.get_majorticklabels(), rotation=45, ha="right")  # Rotate x-ticks for readability

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Plot memory usage from multiple files.')
    parser.add_argument('--file', type=str, action='append', required=True, help='List of files to plot')
    args = parser.parse_args()
    print(f"Files to process: {args.file}")
    plot_mem_usage(args.file)
