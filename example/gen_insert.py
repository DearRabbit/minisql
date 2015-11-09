import os

#python gen.py>xxx.sql
print "create table a(id int, primary key(id));"
for x in range(1000):
	print ("insert into a values(%d);"%x);