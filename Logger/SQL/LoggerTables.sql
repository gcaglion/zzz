drop table TrainLog purge;
create table TrainLog(
	ProcessId number,
	ThreadId number,
	Epoch number,
	MSE_T number,
	MSE_V number
) storage (initial 100m next 100m freelists 8);
alter table TrainLog add constraint TrainLog_PK primary key( ProcessId, ThreadId, Epoch ) using index tablespace LogIdx;
--alter table TrainLog add constraint TrainLog_AdderId_NN check(AdderId is not null);

drop table RunLog purge;
create table RunLog(
	ProcessId number,
	ThreadId number,
	NetProcessId number,
	NetThreadId number,
	Pos number,
	Feature number,
	StepAhead number,
	ActualTRS number,
	PredictedTRS number,
	ErrorTRS number,
	ActualTR number,
	PredictedTR number,
	ErrorTR number,
	Actual number,
	Predicted number,
	Error number,
	BarWidth number,
	ErrorP number
) storage (initial 1024M minextents 8 pctincrease 0);
alter table RunLog add constraint RunLog_PK primary key( ProcessId, ThreadId, Pos, Feature, StepAhead ) using index tablespace LogIdx;

