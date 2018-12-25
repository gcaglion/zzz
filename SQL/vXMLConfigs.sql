create or replace view vXMLconfig as
select processid, min(uHistoryLen) HistoryLen, min(uTopology) Topology, min(uSampleLen) SampleLen, min(uDate0) Date0 from (
select processid, substr(parmval,1,256) uHistoryLen, NULL uTopology, NULL MaxEpochs, NULL uSampleLen, NULL uDate0 from xmlconfigs where upper(parmdesc) like '%HISTORYLEN%'
union
select processid, NULL uHistoryLen, substr(parmval,1,256) uTopology, NULL MaxEpochs, NULL uSampleLen, NULL uDate0  from xmlconfigs where upper(parmdesc) like '%LEVELRATIO%'
union
select processid, NULL uHistoryLen, NULL uTopology, substr(parmval,1,256) MaxEpochs, NULL uSampleLen, NULL uDate0  from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL MaxEpochs, substr(parmval,1,256) uSampleLen, NULL uDate0  from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL uHistoryLen, NULL uTopology, NULL MaxEpochs, NULL uSampleLen, substr(parmval,1,256) uDate0  from xmlconfigs where upper(parmdesc) like '%DATE0%'
) 
group by processid
;
