import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("",8888))
s.listen()
client_socket,client_info=s.accept()
while True:
	recv_data = client_socket.recv(1024)
	print("client:", recv_data.decode("utf-8"))
	msg=input(">>")
	client_socket.send(msg.encode("utf-8"))
