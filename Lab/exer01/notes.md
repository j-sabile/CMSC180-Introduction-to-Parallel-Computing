## Example case from internet
https://www.scribbr.com/statistics/pearson-correlation-coefficient/

X: [3.63,3.02,3.82,3.42,3.59,2.87,3.03,3.46,3.36,3.3]
3.63
3.02
3.82
3.42
3.59
2.87 
3.03 
3.46 
3.36 
3.3

Y: [53.1,49.7,48.4,54.2,54.9,43.7,47.2,45.2,54.4,50.4]
53.1
49.7
48.4
54.2
54.9
43.7
47.2
45.2
54.4
50.4

r = 0.47


```
func pearson_cor(X as matrix, y as vector, m as integer, n as integer) as vector
begin
    define v(n) as vector;
    for i:=1 to n do
    begin
        v(i):=0;
        for j:=1 to m do
        begin
            v(i):= {see equation 1 above};
        end;
    end;
    pearson_cor:=v;
end;
```


