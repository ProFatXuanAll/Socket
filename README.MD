OS support: 
	Linux

compile: 
	make

usage:
	./lab1_file_transfer <tcp | udp> <send | recv> <ip> <port> [filename]

note:
	recv need to startup first, recv don't need filename

	ex:
		./lab1_file_transfer tcp recv 127.0.0.1 5566
		./lab1_file_transfer tcp send 127.0.0.1 5566 test_input.txt
		
		./lab1_file_transfer udp recv 127.0.0.1 5566
		./lab1_file_transfer udp send 127.0.0.1 5566 test_input.txt
