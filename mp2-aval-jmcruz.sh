# Terminal 1:
#	server 3 "ultra_top_secret"	# server starts with admin password

# Terminal 2
user 0 "wrong_password" 10 0 "1 25 top_secret"	# admin fails in account creation
user 0 "ultra_top_secret" 10 0 "1 25 top_secret"	# admin creates account 1
user 1 "top_secret" 1234 1 "" &	# account 1 gets balance
user 1 "top_secret" 5670 2 "2 10" &	# acc1 fails transfer to acc2
user 0 "ultra_top_secret" 1000 0 "2 3 a_secret" &	# admin creates acc2
user 1 "top_secret" 5670 2 "2 10" &	# acc1 transfers to acc2
user 2 "a_secret" 8900 1 "" &	# account 2 gets balance
user 1 "top_secret" 5000 3 "" &	# acc1 fails bank shutdown
user 0 "ultra_top_secret" 5000 3 "" &	# admin shuts bank down
user 1 "top_secret" 5000 1 ""	# too late a customer!
