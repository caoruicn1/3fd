drop database [SvcBrokerTest];
restore database [SvcBrokerTest]
	from disk='C:\Users\felip\source\repos\3fd\SvcBrokerTest.bak'
	with recovery, enable_broker;