drop database [SvcBrokerTest];
restore database [SvcBrokerTest]
	from disk='C:\ProgramData\Microsoft SQL Server\SvcBrokerTest.bak'
	with recovery, enable_broker;