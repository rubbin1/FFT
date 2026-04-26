#计算平顶窗数组
import math

N = 1024
with open("docs/flat_window_1024.txt", "w") as f:
    f.write("const float FlatTop_Window[1024] = {\n")
    for i in range(N):
        w = 0.26526 - 0.5*math.cos(2*math.pi*i/(N-1)) + 0.23474*math.cos(4*math.pi*i/(N-1))
        f.write(f"{w:.6f},")
        if (i+1) % 10 == 0:
            f.write("\n")
    f.write("\n};\n")