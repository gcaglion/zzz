delete from COREIMAGE_NN where processid in(&&1);
delete from COREIMAGE_SOM where processid in(&&1);
delete from COREIMAGE_SVM where processid in(&&1);
delete from CORELOGS_GA where processid in(&&1);
delete from CORELOGS_NN where processid in(&&1);
delete from CORELOGS_NN_SCGD where processid in(&&1);
delete from CORELOGS_SOM where processid in(&&1);
delete from CORELOGS_SVM where processid in(&&1);
delete from COREPARMS_GA where processid in(&&1);
delete from COREPARMS_NN where processid in(&&1);
delete from COREPARMS_SOM where processid in(&&1);
delete from COREPARMS_SVM where processid in(&&1);
delete from MYLOG_MSE where processid in(&&1);
delete from MYLOG_RUN where processid in(&&1);
delete from ENGINETHREADS where processid in(&&1);
delete from DATAPARMS where processid in(&&1);
delete from ENGINEPARMS where processid in(&&1);
delete from CLIENTINFO where processid in(&&1);
//commit;