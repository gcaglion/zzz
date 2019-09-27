col high format 9.99999
select newdatetime, high from history.eurusd_h1 where newdatetime 
between '2018-07-25-02:00' and '2018-07-30-02:00' 
order by 1
/
