delete from coreimage_nn where processid in (select processid from clientinfo where simulationid=&&1);
delete from corelayouts where enginepid in (select processid from clientinfo where simulationid=&&1);
delete from coreloggerparms where processid in (select processid from clientinfo where simulationid=&&1);
delete from corennparms where processid in (select processid from clientinfo where simulationid=&&1);
delete from enginecores where enginepid in (select processid from clientinfo where simulationid=&&1);
delete from engines where processid in (select processid from clientinfo where simulationid=&&1);
delete from trainlog where processid in (select processid from clientinfo where simulationid=&&1);
delete from runlog where processid in (select processid from clientinfo where simulationid=&&1);
