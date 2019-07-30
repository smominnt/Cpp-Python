###ackermann function###
def f(x,y):
    if x == 0:
        return y+1
    elif x>0 and y == 0:
        return f(x-1,1)
    else:
        return f(x-1,f(x,y+1))

f(2,4)

