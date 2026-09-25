import socket

def find_smart_light():
    # Define some parameters 
    UDP_IP = "255.255.255.255" # Broadcast IPv4 Address
    UDP_PORT = 3333
    MESSAGE = b"Are you Espressif IOT Smart Light"

    # 1. Initialize UDP Socket 
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

    # 2.Set Broadcast authorization for socket
    ret = client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    if (ret != None) :
        print(f"Set SO_BROADCAST option fail \n")

    # 3. Set timout for 5 seconds
    client.settimeout(5.0)
    
    try:
        # 4. Send message
        ret = client.sendto(MESSAGE, (UDP_IP, UDP_PORT))
        if (ret < 0) :
            print(f"Error occured during sending \n")
        else :
            print(f"Message sent successfully \n")
        # 5. Listen to responded message
        while True:
            data, addr = client.recvfrom(1024)
            print(f"Receive udp unicast from {addr[0]}, data is {data.decode('utf-8')}\n")
            break # 
            
    except socket.timeout:
        print("Timeout \n")
    finally:
        client.close()

if __name__ == "__main__":
    find_smart_light()