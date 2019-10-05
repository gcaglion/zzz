col high format 9.99999
col highd format 9.99999
select newdatetime, high, high-lag(high) over(order by newdatetime) highd from history.eurusd_h1 where newdatetime
between '2018-06-24-13:00' and '2018-07-27-22:00'
order by 1
/

