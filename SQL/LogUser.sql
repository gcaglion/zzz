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
	SimulationStartTrain varchar2(64),
	SimulationStartInfer varchar2(64),
	SimulationStartValid varchar2(64),
	DoTraining number,
	DoTrainRun number,
	DoTestRun number,
	clientXMLFile varchar2(256),
	shapeXMLFile varchar2(256),
	actionXMLFile varchar2(256),
	engineXMLFile varchar2(256)
) storage (initial 2M minextents 4 pctincrease 0);
alter table ClientInfo add constraint ClientInfo_PK primary key (ProcessId, DoTraining);

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
	EngineType number,
	-- DataShapeInfo
	DataSampleLen number,
	DataPredictionLen number,
	DataFeaturesCnt number,
	-- Persistor Info
	readFrom number,
	saveToDB number,
	saveToFile number,
	-- Persistor-OraData info
	Orausername varchar2(64),
	Orapassword varchar2(64),
	Oraconnstring varchar2(64)
	-- Persistor-FileData info
	-- ...
) storage (initial 2M minextents 4 pctincrease 0);
alter table Engines add constraint Engine_PK primary key(ProcessId) using index tablespace LogIdx;

drop table EngineCores purge;
create table EngineCores(
	EnginePid number,
	CoreId number,
	CoreThreadId number,
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

drop table CoreNNparms purge;
create table CoreNNparms(
	ProcessId number,
	ThreadId number,
	CoreId number,
	LevelRatioS varchar2(64),
	LevelActivationS varchar2(1024),
	UseContext number,
	UseBias number,
	--
	MaxEpochs number,
	TargetMSE number,
	NetSaveFrequency number,
	StopOnDivergence number,
	BPAlgo number,
	BPStd_LearningRate number,
	BPStd_LearningMomentum number,
	BPscgd_maxK number
);
alter table CoreNNparms add constraint CoreNNparms_PK primary key(ProcessId, ThreadId) using index tablespace LogIdx;

drop table CoreNNInternalsSCGD purge;
create table CoreNNInternalsSCGD(
	ProcessId number,
	ThreadId number,
	Iteration number, 
	delta number,
	mu number,
	alpha number,
	beta number,
	lambda number,
	lambdau number,
	Gtse_old number,
	Gtse_new number,
	comp number,
	pnorm number,
	rnorm number,
	dW number
);
alter table CoreNNInternalsSCGD add constraint CoreNNInternalsSCGD_PK primary key(ProcessId, ThreadId, Iteration) using index tablespace LogIdx;

drop table DBConnections purge;
create table DBConnections(
	ProcessId number,
	ThreadId number,
	DBConnId number,
	UserName varchar2(64),
	Password varchar2(64),
	ConnString varchar2(64)
);
alter table DBConnections add constraint DBConnections_PK primary key (ProcessId, ThreadId, DBConnId) using index tablespace LogIdx;
