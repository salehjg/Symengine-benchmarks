#!/bin/bash

python ../plot_mem_usage.py --title Bench01 --file mem_usage_bench01.global.txt --file mem_usage_bench01.expr_gen.txt --file mem_usage_bench01.expr_save.txt --file mem_usage_bench01.wipe.txt --file mem_usage_bench01.expr_load.txt | tee /dev/tty