def pearson_cor(x, y, m, n):
    v = []
    for i in range(n):
        v.append(0)
        for j in range(m):
            v[i] = (m * getXjy(x,y,m,j) - getXj(x,m,j) * getY(y, m)) / ((m * getX2j(x,m,j) - getX(x)**2) * (m * getY2(y,m) - getY(y,m)**2)) **0.5
    return v

def getXjy(x, y, m, j):
    sum = 0
    for i in range(m): sum += x[i][j] * y[i] 
    return sum

def getXj(x, m, j):
    sum = 0
    for i in range(m): sum += x[i][j]
    return sum

def getY(y, m):
    sum = 0
    for i in range(m): sum += y[i]
    return sum

def getX2j(x, m, j):
    sum = 0
    for i in range(m): sum += (x[i][j])**2
    return sum

def getX(x):
    sum = 0
    for i in x: 
        for j in i: sum += j
    return sum

def getY2(y, m):
    sum = 0
    for i in range(m): sum += y[i] ** 2
    return sum

def getY(y, m):
    sum = 0
    for i in range(m): sum += y[i]
    return sum

# func pearson_cor(X as matrix, y as vector, m as integer, n as integer) as vector
# begin
#     define v(n) as vector;
#     for i:=1 to n do
#     begin
#         v(i):=0;
#         for j:=1 to m do
#         begin
#             v(i):= {see equation 1 above};
#         end;
#     end;
#     pearson_cor:=v;
# end;




matrix = [[9,5,3],[4,2,6],[7,8,9]]
y = [3,1,4]
m = 3
n = 3

ans = pearson_cor(matrix, y, m, n)
print(ans)