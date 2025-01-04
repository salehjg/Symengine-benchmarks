#!/bin/bash

python ../plot_mem_usage.py --title bench05 --file mem_usage_bench05.global.txt --file mem_usage_bench05.expr_gen.txt --file mem_usage_bench05.expr_save.txt --file mem_usage_bench05.wipe.txt --file mem_usage_bench05.expr_load.txt | tee /dev/tty