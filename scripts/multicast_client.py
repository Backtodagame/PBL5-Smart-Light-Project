import socket
import struct

def find_multicast_light():
    # Define some parameters 
    MULTICAST_GROUP = '232.10.11.12' #  Multicast IPv4 Address
    PORT = 3333
    MESSAGE = b"Are you Espressif IOT Smart Light"

    # 1. Initialize UDP Socket 
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    
    # 2. Config TTL (Time-to-live) 
    # TTL = 1 means TTL settings are also added to 
    # ensure that the multicast group can only be performed in the LAN of this route.
    ttl = struct.pack('b', 1)
    ret = client.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

    if(ret !=None):
        print("Set IP_MULTICAST_TTL option fail ")
    # 3. Set timeout for 5 seconds
    client.settimeout(5.0)
    
    try:
        # 4. Send message to multicast group
        ret=client.sendto(MESSAGE, (MULTICAST_GROUP, PORT))
        if (ret < 0) :
            print(f"Error occured during sending \n")
        else :
            print(f"Message sent successfully \n")
            
        while True:
            data, addr = client.recvfrom(1024)
            print(f"Receive udp unicast from {addr[0]}:{addr[1]}, data is {data.decode('utf-8')}\n")
            break # 
            
    except socket.timeout:
        print("Timeout \n")
    finally:
        client.close()

if __name__ == "__main__":
    find_multicast_light()