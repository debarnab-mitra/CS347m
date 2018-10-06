Airline Reservation System

Group Members:Akash Doshi: 140010008
		   Debarnab Mitra: 140070037

1> INPUT: input is from the file 'transaction.txt' whose format is described below.


FORMAT of 'transaction.txt'
status 	1
book 		2
cancel 	2
book 		3
book 		7
status 	2
cancel 	3
END

The fisrt column contains the type of query and the 2nd column contains the airline no. The last query in the file is END.

2> Master thread reads a query from the file 'transactions.txt' and assigns the query to a blocked slave if present or waits for a slave to get blocked. If there is no blocked slave at the moment then the  master thread gets blocked and waits on a condition variable called 'all_slaves_not_blocked' which is signalled by a slave which is just about to get blocked. THERE IS NO BUSY WAIT.

3>SLAVE: all the slaves initialy get blocked, the i^th waiting on the condition variable slave_signalled[i] each of which is signalled by the master when it gets a query to assign. After performing the query the slaves get blocked by again by waiting on the same condition variable. Again THERE IS NO BUSY WAIT.

4> Mutexes have been used appropriately for mutual exclusion on critical sections and control synchronization.

5> OUTPUT: 
	i) whenever master assigns a query to slave it prints
		'QUERY : [name_of_query] assigned to slave [slave no.]'

	ii)whenever slave receives a query it prints
		'SLAVE [slave_no.] received a query [query_name]'

	iii) whenever a slave completes a query it print
		'slave [slave_no] [query_result]'
	iv)when all queries are over the master prints
		'all queries have been processed'








