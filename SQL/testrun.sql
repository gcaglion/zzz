set verify off
undefine pid
col forecasth format 99999.99999
col forecastl format 99999.99999
col actualh format 99999.99999
col actuall format 99999.99999
col errorh format 99999.99999
col errorl format 99999.99999
select 
rx.processid, rx.threadid, rx.sequenceid, rx.lasttime, 
ra.actualbase ActualH, rx.ForecastH, abs(ra.actualbase-rx.ForecastH) errorH, rb.actualbase ActualL, rx.ForecastL, abs(rb.actualbase-rx.ForecastL) errorL
from (
	select processid, threadid, sequenceid, lasttime, lag(FH) over(order by lasttime) ForecastH, lag(FL) over(order by lasttime) ForecastL from (
		select rr1.processid, rr1.threadid, rr1.sequenceid, max(rr0.poslabel) lasttime, rr1.prdh FH, rr2.prdl FL from (
			select r1.processid, r1.threadid, r1.sequenceid, r1.predictedbase prdh from runlog r1,
				(select processid, threadid, sequenceid, feature, min(poslabel) plh, null pll from runlog where actualbase is null and feature=1 and processid in(&&pid) group by processid, threadid, sequenceid, feature) r2
				where
				r1.processid=r2.processid and r1.threadid=r2.threadid and r1.sequenceid=r2.sequenceid and r1.feature=r2.feature and r1.poslabel=r2.plh
			) rr1, 
			(select r1.processid, r1.threadid, r1.sequenceid, r1.predictedbase prdl from runlog r1,
				(select processid, threadid, sequenceid, feature, min(poslabel) plh, null pll from runlog where actualbase is null and feature=2 and processid in(&&pid) group by processid, threadid, sequenceid, feature) r2
			where
			r1.processid=r2.processid and r1.threadid=r2.threadid and r1.sequenceid=r2.sequenceid and r1.feature=r2.feature and r1.poslabel=r2.plh
			) rr2,
		runlog rr0
		where 
		rr0.processid=rr2.processid and rr0.threadid=rr2.threadid and rr0.sequenceid=rr1.sequenceid and
		rr1.processid=rr2.processid and rr1.threadid=rr2.threadid and rr1.sequenceid=rr2.sequenceid and
		actualbase is not null
		group by rr1.processid, rr1.threadid, rr1.sequenceid, rr1.prdh, rr2.prdl
	)
) rx, runlog ra, runlog rb 
where 
rx.processid=ra.processid and rx.threadid=ra.threadid and rx.sequenceid=ra.sequenceid and ra.poslabel=rx.lasttime and ra.feature=1
and  rx.processid=rb.processid and rx.threadid=rb.threadid and rx.sequenceid=rb.sequenceid and rb.poslabel=rx.lasttime and rb.feature=2
order by 3,4
 /
 