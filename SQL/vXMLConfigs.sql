create or replace view vXMLconfig as
select processid, min(uHistoryLen) HistoryLen, min(uTopology) Topology, min(uMaxEpochs) MaxEpochs, avg(uMaxK) MaxK, min(uSampleLen) SampleLen, min(uDate0) Date0, min(uSymbol) Symbol, min(uTimeFrame) TimeFrame from (
select processid, to_number(replace(parmval,chr(0),''),'999999') uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol, NULL uTimeFrame from xmlconfigs where upper(parmdesc) like '%HISTORYLEN%'
union
select processid, NULL uHistoryLen, substr(parmval,1,256) uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%LEVELRATIO%'
union
select processid, NULL uHistoryLen, NULL uTopology, to_number(replace(parmval,chr(0),''),'99999') uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, to_number(replace(parmval,chr(0),''),'99999') uMaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%MAXK%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, to_number(replace(parmval,chr(0),''),'99999') uSampleLen, NULL uDate0, NULL uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%SAMPLELEN%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, substr(parmval,1,256) uDate0, NULL uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%DATE0%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, substr(parmval,1,256) uSymbol, NULL uTimeFrame  from xmlconfigs where upper(parmdesc) like '%SYMBOL%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL uMaxEpochs, NULL umaxK, NULL uSampleLen, NULL uDate0, NULL uSymbol, substr(parmval,1,256) uTimeFrame  from xmlconfigs where upper(parmdesc) like '%TIMEFRAME%'
) 
group by processid
;

