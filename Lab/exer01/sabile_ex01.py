# Jerico Sabile
# CSMC180: Introduction to Parallel Computing
# Exer01: Computing the Pearson Correlation Coefficient of a Matrix and a Vector
# Feb. 5, 2024 - 12:00

import time
import random

def pearson_cor(x, y, m, n):
    v = []
    for i in range(n):
        v.append((m*sumXY(x,y,m,i)-sumX(x,m,i)*sumY(y,m))/((m*sumX2(x,m,i)-sumX(x,m,i)**2)*((m*sumY2(y,m))-sumY(y,m)**2))**0.5)
    return v

def sumY2(y,m):
    sum = 0
    for i in range(m): sum += y[i]**2
    return sum

def sumX2(x,m,j):
    sum = 0
    for i in range(m): sum += x[i][j]**2
    return sum

def sumXY(x,y,m,j):
    sum = 0
    for i in range(m): sum += x[i][j] * y[i] 
    return sum

def sumX(x,m,j):
    sum = 0
    for i in range(m): sum += x[i][j]
    return sum

def sumY(y,m):
    sum = 0
    for i in range(m): sum += y[i]
    return sum

def generateRandom(x, y):
    numbers = []
    for i in range(x):
        if y == 0: numbers.append(random.randint(0,100))
        else: 
            new = [] 
            for j in range(y): new.append(random.randint(0,100))
            numbers.append(new)
    return numbers 

# TEST CASE
# ANSWER: 0.47
# matrix = [[3.63],[3.02],[3.82],[3.42],[3.59],[2.87],[3.03],[3.46],[3.36],[3.3]]
# y = [53.1,49.7,48.4,54.2,54.9,43.7,47.2,45.2,54.4,50.4]
# m = 10
# n = 1

size = int(input("Size: "))
matrix = generateRandom(size, size)
y = generateRandom(size, 0)
m = n = size

start_time = time.time()
ans = pearson_cor(matrix, y, m, n)
time_elapsed = time.time() - start_time
print(f"time elapsed: {time_elapsed} seconds")
# print(f"r={ans}")
