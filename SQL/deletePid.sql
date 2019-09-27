set verify on
undefine pid
delete from CLIENTINFO where processid in(&&pid);
delete from COREIMAGE_NN_N where processid in(&&pid);
delete from COREIMAGE_NN_W where processid in(&&pid);
delete from CORELAYOUTS where enginepid in(&&pid);
delete from CORELOGGERPARMS where processid in(&&pid);
delete from CORENNINTERNALSSCGD where processid in(&&pid);
delete from CORENNPARMS where processid in(&&pid);
delete from DBCONNECTIONS where processid in(&&pid);
delete from ENGINECORES where enginepid in(&&pid);
delete from ENGINES where processid in(&&pid);
delete from ENGINESCALINGPARMS where processid in(&&pid);
delete from RUNLOG where processid in(&&pid);
delete from TRADEINFO where clientpid in(&&pid);
delete from TRAINLOG where processid in(&&pid);
delete from XMLCONFIGS where processid in(&&pid);