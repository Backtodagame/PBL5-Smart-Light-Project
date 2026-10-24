import socket

# --- CẤU HÌNH ---
ESP_IP = "172.20.10.2"  # IP của mạch ESP32
PORT = 3333              # Port mà UDP Server trên ESP32 đang lắng nghe

def send_udp_command(command):
    # Khởi tạo UDP Socket (AF_INET = IPv4, SOCK_DGRAM = UDP)
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Đặt thời gian chờ tối đa (nếu ESP32 có gửi phản hồi lại)
    client_socket.settimeout(3.0) 
    
    try:
        print(f"\nĐang 'ném' gói tin UDP tới {ESP_IP}:{PORT} ...")
        
        # UDP KHÔNG CÓ client_socket.connect(). Nó ném thẳng dữ liệu tới đích.
        client_socket.sendto(command.encode('utf-8'), (ESP_IP, PORT))
        print(f"> Đã ném lệnh: '{command}'")
        
        # Lắng nghe ESP32 phản hồi (recvfrom dùng cho UDP)
        response, server_address = client_socket.recvfrom(1024)
        print(f"< ESP32 tại {server_address} phản hồi: {response.decode('utf-8')}")
        
    except socket.timeout:
        # Với TCP, timeout nghĩa là lỗi mạng. 
        # Nhưng với UDP, timeout chỉ đơn giản là ESP32 không gửi thư trả lời, hoặc thư trả lời bị rớt. Lệnh bật đèn vẫn có thể đã thành công!
        print("< [THÔNG BÁO] Không nhận được phản hồi (Đặc thù của UDP).")
    except Exception as e:
        print(f"[LỖI MẠNG] Chi tiết: {e}")
    finally:
        client_socket.close()

if __name__ == "__main__":
    print("="*45)
    print("   BẢNG ĐIỀU KHIỂN ĐÈN THÔNG MINH - UDP")
    print("="*45)
    
    while True:
        print("\n[1] Bật đèn (Gửi 'Open the light')")
        print("[2] Tắt đèn (Gửi 'Close the light')")
        print("[3] Thoát")
        
        choice = input("Nhập lựa chọn của bạn (1/2/3): ").strip()
        
        if choice == '1':
            send_udp_command("Open the light")
        elif choice == '2':
            send_udp_command("Close the light")
        elif choice == '3' or choice.lower() == 'q':
            print("Đã đóng chương trình.")
            break
        else:
            print("[!] Lựa chọn không hợp lệ.")