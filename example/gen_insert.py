
#python gen.py>xxx.sql
print "create table a(name char(9) unique);"
for x in range(2000):
	print ("insert into a values('%d');"%x);