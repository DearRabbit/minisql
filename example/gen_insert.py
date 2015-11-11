
#python gen.py>xxx.sql
print "create table a(name char(9) primary key);"
for x in range(20000):
	print ("insert into a values('%d');"%x);