create table b(sname char(10) primary key, name char(20) unique, id int, gender char(1));
select * from b;
select * from b where sname = 'name1' and id = 10;
delete from b where sname = 'name2' and id = 10;
create index qqq on b (name);
drop index qqq;
drop table b;
