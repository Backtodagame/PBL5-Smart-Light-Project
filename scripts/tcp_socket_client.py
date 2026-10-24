import socket

# BƯỚC QUAN TRỌNG: Sửa IP này thành IP thực tế của ESP32 (xem trên Terminal)
ESP_IP = "172.20.10.2" 
PORT = 3333

def send_tcp_command(command):
    # 1. Khởi tạo socket TCP (Lưu ý: SOCK_STREAM dành cho TCP, khác với SOCK_DGRAM của UDP)
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.settimeout(3.0) # Thời gian chờ kết nối tối đa 3 giây
    
    try:
        # 2. Gọi điện (Kết nối) tới Server ESP32
        client.connect((ESP_IP, PORT))
        
        # 3. Gửi lệnh điều khiển (Chuyển chuỗi thành dạng byte)
        client.send(command.encode('utf-8'))
        print(f"> Đã gửi lệnh: '{command}'")
        
        # 4. Lắng nghe ESP32 trả lời
        response = client.recv(1024).decode('utf-8')
        print(f"< ESP32 phản hồi: {response}\n")
        
    except ConnectionRefusedError:
        print("[LỖI] ESP32 từ chối kết nối. Hãy kiểm tra lại IP và chắc chắn ESP32 đang chạy Server.\n")
    except socket.timeout:
        print("[LỖI] Hết thời gian chờ. Mất kết nối mạng.\n")
    finally:
        # 5. Cúp máy (Đóng kết nối TCP)
        client.close()

if __name__ == "__main__":
    print("="*40)
    print("   BẢNG ĐIỀU KHIỂN ĐÈN THÔNG MINH TCP")
    print("="*40)
    
    while True:
        print("[1] Bật đèn (Open the light)")
        print("[2] Tắt đèn (Close the light)")
        print("[3] Thoát chương trình")
        
        choice = input("Nhập lựa chọn của bạn (1/2/3): ").strip()
        
        if choice == '1':
            send_tcp_command("Open the light")
        elif choice == '2':
            send_tcp_command("Close the light")
        elif choice == '3' or choice.lower() == 'q':
            print("Đã đóng bảng điều khiển.")
            break
        else:
            print("\n[CẢNH BÁO] Lựa chọn không hợp lệ, vui lòng thử lại.\n")