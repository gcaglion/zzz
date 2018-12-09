set verify on
undefine pid
delete from CLIENTINFO where processid=&&pid;
delete from COREIMAGE_NN where processid=&&pid;
delete from CORELAYOUTS where processid=&&pid;
delete from ENGINECORES where processid=&&pid;
delete from ENGINES where processid=&&pid;
delete from RUNLOG where processid=&&pid;
delete from TRAINLOG where processid=&&pid;
delete from CORELOGGERPARMS where processid=&&pid;
delete from CORENNPARMS where processid=&&pid;
