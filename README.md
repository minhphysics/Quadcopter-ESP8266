# Quadcopter-ESP8266
Code for ESP8266 controlling quadcopter
Files included:<br>
Code Arduino file<br>
HTML File for controller webpage<br>
File simulink của quadcopter đã cập nhật. Simulink MATLAB 2018b<br>
# Hướng dẫn điều khiển ESP8266
Bước 1: Nạp code cho vi điều khiển ESP8266.
Dùng Arduino IDE để nạp code cho vi điều khiển. Sử dụng dây cab micro USB kết nối máy tính với kit MCU để nạp. <br>
Bước 2: Sau khi nạp code, đọc địa chỉ IP của vi điều khiển của ESP8266 trên Serial Monitor. Địa chỉ IP của con ESP8266 sử dụng trong quadcopter của chúng em là 192.168.4.1<br>
Bước 3: Dùng điện thoại/laptop kết nối với WIFI của con ESP8266, truy cập đến trang web http://192.168.4.1/transfile để gửi file HTML của trang web điều khiển lên.<br>
Bước 4: Truy cập địa chỉ trang web http://192.168.4.1/index để điều khiển drone<br>
Bước 5: Tiến hành điều khiển. Trang web có 4 thanh trượt điều  khiển: <br>
●	Thanh trượt throttle là duty cycle tính bằng micro giây, để điều khiển tốc độ của 4 động cơ. <br>
●	Thanh trượt roll dùng để điều khiển góc roll của quadcopter.<br>
●	Thanh trượt pitch dùng để điều khiển góc pitch của quadcopter.<br>
●	Thanh trượt yaw dùng để điều khiển góc yaw của quadcopter.<br>
