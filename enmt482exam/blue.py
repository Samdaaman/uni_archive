
var1 = 0.25
var2 = 1

weight1 = (1/var1)/(1/var1+1/var2)
weight2 = (1/var2)/(1/var1+1/var2)
print(f"w1 = {weight1:.2f}, w2 = {weight2:.2f}")

print(f"BLUE = {weight1:.2f}*X1 + {weight2:.2f}*X2")