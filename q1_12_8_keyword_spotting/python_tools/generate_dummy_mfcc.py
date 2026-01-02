import numpy as np

def make_dummy_mfcc(seed=0):
    rng = np.random.default_rng(seed)
    # -1..1 arası float
    x = rng.uniform(-1.0, 1.0, size=(13,)).astype(np.float32)
    return x

if __name__ == "__main__":
    x = make_dummy_mfcc(42)
    print("Dummy MFCC(13):", np.array2string(x, precision=3))
