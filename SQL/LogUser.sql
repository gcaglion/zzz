--drop user LogUser cascade;
--create user LogUser identified by LogPwd default tablespace LogData;
--grant dba to LogUser;
--connect LogUser/LogPwd@Algo

drop table CoreInfo purge;
create table CoreInfo(
	ProcessId number,
	ThreadId number,
	CoreType number,
	sampleLen number,
	targetLen number, 
	inputCnt number,
	outputCnt number,
	batchSize number
);
alter table CoreInfo add constraint CoreInfo_PK primary key(ProcessId, ThreadId) using index tablespace LogIdx;

drop table CoreScalingInfo;
create table CoreScalingInfo(	
	ProcessId number,
	ThreadId number,
	InputId number,
	TRmin number,
	TRmax number
);
alter table CoreScalingInfo add constraint CoreScalingInfo_PK primary key(ProcessId, ThreadId, InputId);

drop table TrainLog purge;
create table TrainLog(
	ProcessId number,
	ThreadId number,
	Epoch number,
	Duration number,
	MSE_T number,
	MSE_V number
) storage (initial 100m next 100m freelists 8);
alter table TrainLog 
add constraint TrainLog_PK primary key( ProcessId, ThreadId, Epoch ) using index tablespace LogIdx;
--alter table TrainLog add constraint TrainLog_AdderId_NN check(AdderId is not null);

drop table RunLog purge;
create table RunLog(
	ProcessId number,
	ThreadId number,
	NetProcessId number,
	NetThreadId number,
	SequenceId number,
	mseR number,
	Pos number,
	PosLabel varchar2(64),
	DataSource number,
	Feature number,
	WTlevel number,
	ActualTRS number,
	PredictedTRS number,
	ActualTR number,
	PredictedTR number,
	ActualBASE number,
	PredictedBASE number
) storage (initial 1024M minextents 8 pctincrease 0);
alter table RunLog add constraint RunLog_PK primary key( ProcessId, ThreadId, Pos, DataSource, Feature, WTlevel ) using index tablespace LogIdx;

drop table ClientInfo purge;
create table ClientInfo(
	ProcessId  number,
	SequenceId number,
	SimulationId number,
	NetProcessId number,
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
alter table ClientInfo add constraint ClientInfo_PK primary key (ProcessId, SequenceId, DoTraining);

drop table CoreImage_NN_W purge;
create table CoreImage_NN_W(
	ProcessId number,
	ThreadId number,
	Epoch number,
	WId number,
	W number,
	constraint CoreImage_NN_W_PK primary key( ProcessId, ThreadId, Epoch, WId )
) 
organization index
partition by range(ProcessId)
(
partition p0 values less than (1000),
partition p1 values less than (2000),
partition p2 values less than (3000),
partition p3 values less than (4000),
partition p4 values less than (5000),
partition p5 values less than (6000),
partition p6 values less than (7000),
partition p7 values less than (8000),
partition p8 values less than (9000),
partition p9 values less than (10000),
partition p10 values less than (11000),
partition p11 values less than (12000),
partition p12 values less than (13000),
partition p13 values less than (14000),
partition p14 values less than (15000),
partition p15 values less than (16000),
partition p16 values less than (17000),
partition p17 values less than (18000),
partition p18 values less than (19000),
partition p19 values less than (20000),
partition p99 values less than (MAXVALUE)
 );
--alter table CoreImage_NN_W add constraint CoreImage_NN_W_PK primary key( ProcessId, ThreadId, Epoch, WId ) using index tablespace LogIdx;

drop table CoreImage_NN_N purge;
create table CoreImage_NN_N(
	ProcessId number,
	ThreadId number,
	Epoch number,
	NId number,
	F number,
	constraint CoreImage_NN_N_PK primary key( ProcessId, ThreadId, Epoch, NId )
) 
organization index
storage (initial 1024M minextents 8 pctincrease 0);
--alter table CoreImage_NN_N add constraint CoreImage_NN_N_PK primary key( ProcessId, ThreadId, Epoch, NId ) using index tablespace LogIdx;

drop table Engines purge;
create table Engines(
	ProcessId number,
	EngineType number,
	-- DataShapeInfo
	DataSampleLen number,
	DataPredictionLen number,
	DataFeaturesCnt number,
	DataBatchSize number,
	-- WNN info
	WNNdecompLevel number,
	WNNwaveletType number,
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
	Layer number,
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

drop table CoreLoggerParms purge;
create table CoreLoggerParms(
	ProcessId number,
	ThreadId number,
	ReadFrom number,
	SaveToDB number,
	SaveToFile number,
	SaveMSEFlag number,
	SaveRunFlag number,
	SaveInternalsFlag number,
	SaveImageFlag number
);
alter table CoreLoggerParms add constraint CoreLoggerParms_PK primary key(ProcessId, ThreadId) using index tablespace LogIdx;

drop table EngineScalingParms purge;
create table EngineScalingParms(
	ProcessId number,
	DecompLevel number,
	Feature number,
	trMin number,
	trMax number
);
alter table EngineScalingParms add constraint EngineScalingParms_PK primary key(ProcessId, DecompLevel, Feature) using index tablespace LogIdx;

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

drop table TradeInfo purge;
create table TradeInfo(
	ClientPid number,
	SessionId number,
	TicketId number,
	AccountId number,
	EnginePid number,
	OpenTime date,
	LastBarT date,
	LastBarO number,
	LastBarH number,
	LastBarL number,
	LastBarC number,
	LastBarV number,
	LastForecastO number,
	LastForecastH number,
	LastForecastL number,
	LastForecastC number,
	LastForecastV number,
	ForecastO number,
	ForecastH number,
	ForecastL number,
	ForecastC number,
	ForecastV number,
	TradeScenario number,
	TradeResult number,
	TPhit number,
	SLhit number
);
alter table TradeInfo add constraint TradeInfo_PK primary key(ClientPid, SessionId, LastBarT) using index tablespace LogIdx;

drop table XMLConfigs purge;
create table XMLConfigs(
	SimulationId number,
	ProcessId number,
	ThreadId number,
	FileId number,
	ParmId number,
	ParmDesc varchar2(4000),
	ParmVal varchar2(4000)
);
alter table XMLConfigs add constraint XMLConfigs_PK primary key(SimulationId, ProcessId, ThreadId, FileId, ParmId) using index tablespace LogIdx;


-- Views
create or replace  view vLastMSE as select v.processId, v.threadId, v.LastEpoch, t.mse_t LastMSE
from trainLog t, 
(
	select processid, threadid, max(epoch) LastEpoch from trainlog group by processid, threadid
) v 
where t.processid=v.processid and t.threadid=v.threadid and t.epoch=v.LastEpoch order by 1,2;
--
create or replace view vRunStats   as select processId, threadId, netProcessId, netThreadId,
decode(Feature,0,'OPEN',1,'HIGH',2,'LOW',3,'CLOSE',4,'VOLUME','UNKNOWN') feature,
max(posLabel) LastDate, max(pos) LastPos , avg(mseR) avgMSER, avg(abs(ActualBase-PredictedBase)) avgErrorBase , avg(abs(ActualTRS-PredictedTRS)) avgErrorTRS from runlog group by processId, threadId, netProcessId, netThreadId, Feature;
--
create or replace view vXMLconfig as
select processid, min(uHistoryLen) HistoryLen, min(uTopology) Topology, min(uMaxEpochs) MaxEpochs, avg(uMaxK) MaxK, min(uSampleLen) SampleLen, min(uDate0) Date0, min(uSymbol0) Symbol0, min(uSymbol1) Symbol1, min(uSymbol2) Symbol2, min(uTimeFrame) TimeFrame, min(uDataTransformation) DataTransformation, min(uUseCtx0) Core0UseContext, min(uTimeSeriesCount) TimeSeriesCount, min(uUseBias0) Core0UseBias, min(uTrainBatchSize) TrainBatchSize, min(uInferBatchSize) InferBatchSize from (
select processid, to_number(replace(parmval,chr(0),''),'999999') uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%HISTORYLEN%'
union
select processid, NULL uHistoryLen, substr(parmval,1,256) uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%LEVELRATIO%'
union
select processid, NULL uHistoryLen, NULL uTopology, to_number(replace(parmval,chr(0),''),'99999') uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, to_number(replace(parmval,chr(0),''),'99999') uMaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%MAXK%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, to_number(replace(parmval,chr(0),''),'99999') uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%SAMPLELEN%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, substr(parmval,1,256) uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%DATE0%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, substr(parmval,1,256) uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%TIMESERIE0%SYMBOL%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, substr(parmval,1,256) uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%TIMESERIE1%SYMBOL%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, substr(parmval,1,256) uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%TIMESERIE2%SYMBOL%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, substr(parmval,1,256) uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%TIMEFRAME%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, substr(parmval,1,256) uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%DATATRANSFORMATION%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, substr(parmval,1,256) uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%CORE0%USECONTEXT%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, substr(parmval,1,256) uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%TIMESERIESCOUNT%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, substr(parmval,1,256) uUseBias0, NULL uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%CORE0%USEBIAS%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, to_number(replace(parmval,chr(0),''),'99999') uTrainBatchSize, NULL uInferBatchSize from xmlconfigs where upper(parmdesc) like '%BATCHSIZE%' and fileid=1
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol0, NULL uSymbol1, NULL uSymbol2, NULL uTimeFrame, NULL uDataTransformation, NULL uUseCtx0, NULL uTimeSeriesCount, NULL uUseBias0, NULL uTrainBatchSize, to_number(replace(parmval,chr(0),''),'99999') uInferBatchSize from xmlconfigs where upper(parmdesc) like '%BATCHSIZE%' and fileid=2
) 
group by processid
;
--
