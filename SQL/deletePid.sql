set verify on
undefine pid
delete from CLIENTINFO where processid in(&&pid);
delete from COREIMAGE_NN where processid in(&&pid);
delete from CORELAYOUTS where enginepid in(&&pid);
delete from ENGINECORES where enginepid in(&&pid);
delete from ENGINES where processid in(&&pid);
delete from RUNLOG where processid in(&&pid);
delete from TRAINLOG where processid in(&&pid);
delete from CORELOGGERPARMS where processid in(&&pid);
delete from CORENNPARMS where processid in(&&pid);
