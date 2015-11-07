create table a(id int primary key, name char(5), phone char(11) unique);

insert into a values(1, 'Bob', '12345678901');
insert into a values(2, 'Amy', '22345678901');
insert into a values(3, 'Lex', '32345678901');
insert into a values(4, 'Cally', '42345678901');
insert into a values(5, 'Dexter', '52345678901');
insert into a values(6, 'Emma', '62345678901');
insert into a values(7, 'Fred', '72345678901');
insert into a values(8, 'Greg', '82345678901');
insert into a values(9, 'Imp', '92345678901');
insert into a values(10, 'Jack', '10345678901');
insert into a values(11, 'King', '11345678901');

create index b on a(phone);
drop index b;
select * from a where phone = '12345678901';
select * from a where id < 5;
select * from a where id > 5 and id < 8;
delete from a where name = 'Lex';

delete from a where id > 9;
select * from a;
drop table a;