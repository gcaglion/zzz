--drop user LogUser cascade;
--create user LogUser identified by LogPwd default tablespace LogData;
--grant dba to LogUser;
--connect LogUser/LogPwd@Algo

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
	PosLabel varchar2(64),
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

drop table ClientInfo purge;
create table ClientInfo(
	ProcessId  number,
	SimulationId number,
	ClientName varchar2(128),
	ClientStart date,
	Duration number,
	SimulationStartTrain date,
	SimulationStartInfer date,
	SimulationStartValid date,
	DoTraining number,
	DoTrainRun number,
	DoTestRun number
) storage (initial 2M minextents 4 pctincrease 0);
alter table ClientInfo add constraint ClientIngo_PK primary key (ProcessId, SimulationId);

drop table CoreImage_NN purge;
create table CoreImage_NN(
	ProcessId number,
	ThreadId number,
	Epoch number,
	WId number,
	W number
) storage (initial 1024M minextents 8 pctincrease 0);
alter table CoreImage_NN add constraint CoreImage_NN_PK primary key( ProcessId, ThreadId, Epoch, WId ) using index tablespace LogIdx;

drop table Engines purge;
create table Engines(
	ProcessId number,
	EngineType number	
) storage (initial 2M minextents 4 pctincrease 0);
alter table Engines add constraint Engine_PK primary key(ProcessId) using index tablespace LogIdx;

drop table EngineCores purge;
create table EngineCores(
	EnginePid number,
	CoreId number,
	CoreType number
);
alter table EngineCores add constraint EngineCores_PK primary key(EnginePid, CoreId) using index tablespace LogIdx;

drop table CoreLayouts purge;
create table CoreLayouts(
	EnginePid number,
	CoreId number,
	ParentCoreId number,
	ParentConnType number
);
alter table CoreLayouts add constraint CoreLayouts_PK primary key(EnginePid, CoreId, ParentCoreId) using index tablespace LogIdx;

drop table CoreParametersNN purge;
create table CoreParametersNN(
	EnginePid number,
	CoreId number,
	LevelRatioS varchar2(64),
	LevelActivationS varchar2(1024),
	UseContext	number,
	UseBias number,
	--
	MaxEpochs number,
	TargetMSE number,
	NetSaveFrequency number,
	StopOnDivergence number,
	BPAlgo number,
	BPStd_LearningRate number,
	BPStd_LearningMomentum number
);
alter table CoreParametersNN add constraint CoreParametersNN_PK primary key(EnginePid, CoreId) using index tablespace LogIdx;
