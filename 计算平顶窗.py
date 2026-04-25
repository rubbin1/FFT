#计算平顶窗数组
import numpy as np
N = 1024
n = np.arange(N)
w = 0.26526 - 0.5*np.cos(2*np.pi*n/(N-1)) + 0.23474*np.cos(4*np.pi*n/(N-1))
for val in w:
    print(f"{val:.6f},", end='')

#将平顶窗保存至docs文件中，命名为flat_top_window.txt
import math

N = 1024
with open("docs/flat_top_window.txt", "w", encoding="utf-8") as f:
    f.write("const float FlapTop_Window[1024] = {\n")
    for i in range(N):
        w = 0.26526 - 0.5*math.cos(2*math.pi*i/(N-1)) + 0.23474*math.cos(4*math.pi*i/(N-1))
        f.write(f"    {w:.6f},")
        if (i + 1) % 10 == 0:
            f.write("\n")
    f.write("\n};\n")