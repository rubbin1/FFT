import math

N = 1024
with open("docs/hanning_window_1024.txt", "w") as f:
    f.write("const float Hanning_Window[1024] = {\n")
    for i in range(N):
        # 汉宁窗公式：0.5 * (1 - cos(2*pi*i/(N-1)))
        w = 0.5 * (1.0 - math.cos(2.0 * math.pi * i / (N - 1)))
        f.write(f"{w:.6f}")
        if i != N - 1:
            f.write(", ")
        if (i + 1) % 10 == 0:
            f.write("\n")
    f.write("\n};\n")