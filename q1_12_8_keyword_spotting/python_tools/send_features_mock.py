import argparse
import numpy as np

KW_IN, KW_H1, KW_OUT = 13, 20, 5

# Float model: model_kw_int.h ile paralel (dummy weights)
def load_dummy_float_model():
    # W1: 20x13, her satır sabit
    row_vals = [820,410,205,-205,-410,300,-300,150,-150,600,-600,250,-250,100,-100,700,-700,350,-350,50]
    W1 = np.vstack([np.full((KW_IN,), v, dtype=np.float32) for v in row_vals]) / 32768.0
    b1 = np.array([200,-100,50,0,75,-50,25,0,0,100,-80,40,0,0,20,60,-60,30,-30,10], dtype=np.float32) / 32768.0

    # W2: 5x20 (model_kw_int.h içindeki sayıları kopyaladık)
    W2 = np.array([
      [400,200,100,-100,-200,300,-300,150,-150,250,-250,120,-120,80,-80,220,-220,60,-60,40],
      [-300,-150,-80,80,150,-220,220,-120,120,-200,200,-60,60,-40,40,-180,180,-30,30,-20],
      [100,120,140,160,180,60,80,100,120,140,160,180,200,220,240,50,40,30,20,10],
      [-100,-120,-140,-160,-180,-60,-80,-100,-120,-140,-160,-180,-200,-220,-240,-50,-40,-30,-20,-10],
      [250,-250,220,-220,180,-180,140,-140,100,-100,60,-60,40,-40,30,-30,20,-20,10,-10],
    ], dtype=np.float32) / 32768.0
    b2 = np.array([50,-25,10,-10,0], dtype=np.float32) / 32768.0

    return W1, b1, W2, b2

def relu(x): return np.maximum(0.0, x)

def predict_mock(mfcc):
    W1, b1, W2, b2 = load_dummy_float_model()
    h1 = relu(W1 @ mfcc + b1)
    logits = W2 @ h1 + b2
    cls = int(np.argmax(logits))
    return cls, logits

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--mode", choices=["mock","uart"], default="mock")
    ap.add_argument("--port", type=str, default="COM5")
    ap.add_argument("--baud", type=int, default=2000000)
    args = ap.parse_args()

    rng = np.random.default_rng(args.seed)
    mfcc = rng.uniform(-1.0, 1.0, size=(13,)).astype(np.float32)

    print("MFCC(13) =", np.array2string(mfcc, precision=3))

    if args.mode == "mock":
        cls, logits = predict_mock(mfcc)
        labels = ["yes","no","up","down","stop"]
        print("Predicted class id =", cls)
        print("Predicted label    =", labels[cls])
        print("Logits             =", np.array2string(logits, precision=5))
        return

    # UART mode (opsiyonel)
    import serial, struct
    with serial.Serial(args.port, args.baud, timeout=2) as ser:
        ser.write(b"\xA5")
        # Q15 gönder
        mfcc_q15 = np.clip(np.round(mfcc * 32768.0), -32768, 32767).astype(np.int16)
        for v in mfcc_q15:
            ser.write(struct.pack("<h", int(v)))
        resp = ser.read(1)
        if len(resp) != 1:
            print("No response!")
            return
        cls = resp[0]
        labels = ["yes","no","up","down","stop"]
        print("STM32 returned class id =", cls, "label =", labels[cls] if cls < 5 else "?")

if __name__ == "__main__":
    main()
