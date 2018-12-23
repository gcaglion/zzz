create or replace view vXMLconfig as
select processid, parmval HistoryLen, NULL Topology, NULL MaxEpochs, NULL sampleLen, NULL Date0 from xmlconfigs where upper(parmdesc) like '%HISTORYLEN%'
union
select processid, NULL HistoryLen, parmval Topology, NULL MaxEpochs, NULL sampleLen, NULL Date0  from xmlconfigs where upper(parmdesc) like '%LEVELRATIO%'
union
select processid, NULL HistoryLen, NULL Topology, parmval MaxEpochs, NULL sampleLen, NULL Date0  from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL HistoryLen, NULL Topology, NULL MaxEpochs, parmval SampleLen, NULL Date0  from xmlconfigs where upper(parmdesc) like '%MAXEPOCHS%'
union
select processid, NULL HistoryLen, NULL Topology, NULL MaxEpochs, NULL SampleLen, parmval Date0  from xmlconfigs where upper(parmdesc) like '%DATE0%'
;
