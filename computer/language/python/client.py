import socket
client_socket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(("localhost",8888))
while True:
    # 客户端发送消息
    msg=input(">>")
    client_socket.send(msg.encode("utf-8"))
    # 客户端接收消息
    rece_data=client_socket.recv(1024)
    print("服务端说:",rece_data.decode("utf-8"))
