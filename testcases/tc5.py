def f(m,n):
   if m==2:
      return n-1
   else:
      return f(m-1,n+2)

def f2(i):
   if i==0:
      return 1
   else:
      return f(i)

f(3,4)
f2(0)

