-- Views
create or replace  view vLastMSE as select v.processId, v.threadId, t.mse_t LastMSE
from trainLog t, 
(
	select processid, threadid, max(epoch) LastEpoch from trainlog group by processid, threadid
) v 
where t.processid=v.processid and t.threadid=v.threadid and t.epoch=v.LastEpoch order by 1,2;

create or replace view vRunStats   as select processId, threadId, netProcessId, netThreadId, max(posLabel) LastDate, max(pos) LastPos , avg(BarWidth) avgBarWidth, avg(Error) avgError from runlog group by processId, threadId, netProcessId, netThreadId;

//====== QUERIES =====

select 
lm.processid TrainPid, lm.threadId TrainTid, ci.SimulationStartTrain TrainDate0, lm.lastMSE TrainMSE,
rs.processid InferPid, rs.threadId InferTid, rs.LastDate InferDate0, rs.LastPos InferDate0, rs.avgBarWidth, rs.avgError,
decode(ci.SimulationStartTrain, rs.LastDate, 'IN-SAMPLE','OUT-OF-SAMPLE')
from vLastMSE lm, ClientInfo ci, vRunStats rs
where 
lm.processId=ci.processId and 
lm.processId=rs.NetProcessId 
--and lm.threadId=rs.NetThreadId
order by 1,2,5;

