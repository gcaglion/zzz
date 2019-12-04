set verify off
undefine pid
col forecasth format 99999.99999
col forecastl format 99999.99999
select processid, threadid, lasttime, lag(FH) over(order by lasttime) ForecastH, lag(FL) over(order by lasttime) ForecastL from (
select rr1.processid, rr1.threadid, max(rr0.poslabel) lasttime, rr1.prdh FH, rr2.prdl FL from (
     select r1.processid, r1.threadid, r1.predictedbase prdh from runlog r1,
             (select processid, threadid, feature, min(poslabel) plh, null pll from runlog where actualbase is null and feature=1 and processid in(&&pid) group by processid, threadid, feature) r2
     where
  r1.processid=r2.processid and r1.threadid=r2.threadid and r1.feature=r2.feature and r1.poslabel=r2.plh
  ) rr1, (
  select r1.processid, r1.threadid, r1.predictedbase prdl from runlog r1,
  (select processid, threadid, feature, min(poslabel) plh, null pll from runlog where actualbase is null and feature=2 and processid in(&&pid) group by processid, threadid, feature) r2
  where
  r1.processid=r2.processid and r1.threadid=r2.threadid and r1.feature=r2.feature and r1.poslabel=r2.plh
  ) rr2,
  runlog rr0
  where rr0.processid=rr2.processid and rr0.threadid=rr2.threadid and
  rr1.processid=rr2.processid and rr1.threadid=rr2.threadid and
  actualbase is not null
  group by rr1.processid, rr1.threadid, rr1.prdh, rr2.prdl
  )
 order by 3
 /
 