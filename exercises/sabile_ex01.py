def pearson_cor(x, y, m, n):
    v = []
    for i in range(n):
        v.append(0)
        for j in range(m):
            v[i] = (m * getXjy(x,y,m,j) - getXj(x,m,j) * getY(y, m))
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





matrix = [[9,5,3],[4,2,6],[7,8,9]]
y = [3,1,4]
m = 3
n = 3

ans = pearson_cor(matrix, y, m, n)
print(ans)