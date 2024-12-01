# Bench01

This benchmark assumes we have 3 flat tensors of size `cfg_L` and we want to generate `cfg_N` expression with this
formula:

```
expr_i = Sum_{j=0}^{cfg_L} (a_j + b_j + c_j)^get_random_integer(min=1, max=cfg_P) for i in range(cfg_N)
```

## Remarks

- Without expanding exprs, it is observed that after deserialization from disk, memory usage is way higher than the
  state in which all exprs and the dictionary of symbols are on RAM.
- Using expand on exprs lead to extremely slow deserialization and even higher memory usage.