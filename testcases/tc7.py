def sumodd(n,m):
   if n >= m:
      return n
   else:
      return n+sumodd(n+2, m)
	  
sumodd(1,10)

