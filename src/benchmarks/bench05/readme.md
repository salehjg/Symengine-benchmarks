# Bench02

This benchmark assumes we have 3 flat tensors of size `cfg_L` and we want to generate `cfg_N` expression with this
formula:

```
expr_i = Sum_{j=0}^{cfg_L} (a_j + b_j + c_j)^get_random_integer(min=1, max=cfg_P) for i in range(cfg_N)
```

This benchmark uses the vector serialization and deserialization methods in Symengine::Basic.

## Remarks

-  xxxx