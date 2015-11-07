create table b(sname char(10) primary key, name char(20) unique, id int, gender char(1));
select * from b;
select * from b where sname = 'name1' and id = 9;
delete from b where sname = 'name2' and id = 10;
create index qqq on b (name);
drop index qqq;
drop table b;

insert into b values('name7', 'unique0', 10, 'M');
create table a(id int, primary key(id));
insert into a values(2);
