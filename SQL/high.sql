col high format 9.99999
col highd format 9.99999
select newdatetime, high, high-lag(high) over(order by newdatetime) highd from history.eurusd_h1 where newdatetime
between to_date('2018-08-26-21:00','YYYY-MM-DD-HH24:MI') and to_date('2018-08-30-00:00', 'YYYY-MM-DD-HH24:MI')
order by 1
/

