# Jerico Sabile
# CSMC180: Introduction to Parallel Computing
# Exer01: Computing the Pearson Correlation Coefficient of a Matrix and a Vector
# Feb. 5, 2024 - 12:00

import time
import random
import threading

def getRuns(remaining, threads):
    if remaining >= threads: return threads
    return remaining

def getNumOfCols(n ,t, i):
    if i >= n%t: return n//t
    return n//t + 1


def pearson_cor_threads(x, y, m, n, t):
    subMatrices = []
    inserted = 0
    subMatricesCols = []
    for i in range(t):
        subMatricesCols.append(getNumOfCols(n,t,i))
        temp = []
        for j in range(len(x)): temp.append(x[j][inserted:inserted+subMatricesCols[i]])
        inserted+=subMatricesCols[i]
        subMatrices.append(temp)

    start_time = time.time()
    threads = []
    for i in range(t):
        threads.append(threading.Thread(target=pearson_cor, args=(subMatrices[i],y,n,subMatricesCols[i])))
        threads[i].start()
    for thread in threads: thread.join()

    time_elapsed = time.time() - start_time
    print(f"time elapsed: {time_elapsed} seconds")


    # v = [0] * n
    # done = 0
    # threads.append(threading.Thread(target=solve, args=(x,y,m,0,v)))
    # threads[0].start()
    # print(threads[0].is_alive())
    # threads[0].join()
    # print(threads)
    # numberOfLoops = n//t
    # for a in range(numberOfLoops):
    #     for i in range(t):
    #         threads[i] = threading.Thread(target=solve, args=(x,y,m,done,v))
    #         done+=1
    #         threads[i].start()
    #     for thread in threads: thread.join()


    # while 1:
    #     for i in range(getRuns(n-done, t)):
    #         threads[i] = threading.Thread(target=solve, args=(x,y,m,done,v))
    #         done += 1
    #         threads[i].start()
    #     for thread in threads: thread.join()
    #     if n == done: break
    # for i in range(t):
    #     threads[i].join()
    #     done += 1
    
    # while 1:
    #     for i in range((n-done)%t):
    #         threads[i] = threading.Thread(target=solve, args=(x,y,m,i,v))
        
    
    # for i in range(n):
    #     threads.append(threading.Thread(target=solve, args=(x,y,m,i,v)))
    #     threads[i].start()        
    # for i in range(n):
    #     threads[i].join()
    # return v

def pearson_cor(x, y, m, n):
    v = []
    for i in range(n):
        v.append((m*sumXY(x,y,m,i)-sumX(x,m,i)*sumY(y,m))/((m*sumX2(x,m,i)-sumX(x,m,i)**2)*((m*sumY2(y,m))-sumY(y,m)**2))**0.5)
    return v

def solve(x, y, m, i, v):
    v[i] = (m*sumXY(x,y,m,i)-sumX(x,m,i)*sumY(y,m))/((m*sumX2(x,m,i)-sumX(x,m,i)**2)*((m*sumY2(y,m))-sumY(y,m)**2))**0.5

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
# matrix = [[3.63,3.63,3.63,3.63,3.63,3.63,3.63,3.63,3.63,3.63],[3.02,3.02,3.02,3.02,3.02,3.02,3.02,3.02,3.02,3.02],[3.82,3.82,3.82,3.82,3.82,3.82,3.82,3.82,3.82,3.82],[3.42,3.42,3.42,3.42,3.42,3.42,3.42,3.42,3.42,3.42],[3.59,3.59,3.59,3.59,3.59,3.59,3.59,3.59,3.59,3.59],[2.87,2.87,2.87,2.87,2.87,2.87,2.87,2.87,2.87,2.87],[3.03,3.03,3.03,3.03,3.03,3.03,3.03,3.03,3.03,3.03],[3.46,3.46,3.46,3.46,3.46,3.46,3.46,3.46,3.46,3.46],[3.36,3.36,3.36,3.36,3.36,3.36,3.36,3.36,3.36,3.36],[3.3,3.33,3.33,3.33,3.33,3.33,3.33,3.33,3.33,3.3]]
# y = [53.1,49.7,48.4,54.2,54.9,43.7,47.2,45.2,54.4,50.4]
# m = 10
# n = 10
# t = 1

size = int(input("Size: "))
t = int(input("Threads: "))
matrix = generateRandom(size, size)
y = generateRandom(size, 0)
m = n = size

# start_time = time.time()
ans = pearson_cor_threads(matrix, y, m, n, t)
# time_elapsed = time.time() - start_time
# print(f"time elapsed: {time_elapsed} seconds")
# print(f"r={ans}")
