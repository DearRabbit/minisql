create table a(id int, primary key(id));
insert into a values(2);
delete from a where id = 2;
insert into a values(2);

select * from a;
execfile ../example/insert_sample.sql;
delete from a;
insert into a values(251);
insert into a values(252);
insert into a values(253);
insert into a values(254);
insert into a values(256);
insert into a values(505);
insert into a values(506);