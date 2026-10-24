from coapthon.client.helperclient import HelperClient
import time

# --- CẤU HÌNH ---
ESP_IP = "172.20.10.2"  # Thay bằng IP thực tế của ESP32
PORT = 5683              # Port mặc định của CoAP
PATH = "light"           # Tên Resource đăng ký trong hàm coap_resource_init() của code C

def send_coap_put(command):
    print(f"\nĐang kết nối tới coap://{ESP_IP}:{PORT}/{PATH} ...")
    client = HelperClient(server=(ESP_IP, PORT))
    
    try:
        # Trong CoAP/HTTP, việc thay đổi trạng thái (bật/tắt) sẽ dùng phương thức PUT
        response = client.put(PATH, payload=command, timeout=5)
        
        print(f"> Đã gửi lệnh PUT: '{command}'")
        if response:
            # response.code = 68 tương đương với mã 2.04 (Changed/Thành công) của CoAP
            print(f"< ESP32 Trả về mã trạng thái: {response.code}")
            if response.payload:
                print(f"< Nội dung phản hồi: {response.payload}")
        else:
            print("< [LỖI] Không nhận được phản hồi từ ESP32.")
            
    except Exception as e:
        print(f"\n[LỖI MẠNG] Không thể kết nối tới ESP32. Chi tiết: {e}")
    finally:
        client.stop()

if __name__ == "__main__":
    print("="*45)
    print("   BẢNG ĐIỀU KHIỂN ĐÈN THÔNG MINH - CoAP")
    print("="*45)
    
    while True:
        print("\n[1] Bật đèn (Open the light)")
        print("[2] Tắt đèn (Close the light)")
        print("[3] Kiểm tra trạng thái hiện tại (Gửi lệnh GET)")
        print("[4] Thoát")
        
        choice = input("Nhập lựa chọn của bạn (1/2/3/4): ").strip()
        
        if choice == '1':
            send_coap_put("Open the light")
        elif choice == '2':
            send_coap_put("Close the light")
        elif choice == '3':
            # Phương thức GET dùng để hỏi xem đèn đang bật hay tắt (nếu code C của bạn có hỗ trợ hàm GET)
            client = HelperClient(server=(ESP_IP, PORT))
            res = client.get(PATH)
            print(f"< Trạng thái hiện tại: {res.payload if res else 'Không rõ'}")
            client.stop()
        elif choice == '4' or choice.lower() == 'q':
            print("Đã đóng chương trình.")
            break
        else:
            print("[!] Lựa chọn không hợp lệ.")