# Kịch bản thuyết trình 8–10 phút

## Chuẩn bị trước giờ học

Chạy `CHAY_THUYET_TRINH.bat`, thử một lượt rồi đóng/mở lại. Mở slide PowerPoint và bản PDF dự phòng. Khi mang máy khác, chép cả thư mục dự án. Lệnh tương đương là `run.bat --demo --seed 2026`. Chế độ này bắt đầu từ trạng thái sạch, giữ lịch sử trong RAM và không thay CSV của phiên thường. Seed cố định nhiễu ban đầu, không phải đáp án cho mọi lực đổ.

Điền họ tên, lớp và mã sinh viên vào slide bìa trước khi nộp nếu thầy yêu cầu. Bộ slide hiện không tự điền thông tin cá nhân chưa được cung cấp.

## 0:00–0:45 / Mở đầu

“Em thực hiện đề tài Mô phỏng Xúc xắc 3D và trò chơi đổ xúc xắc bằng C++ và OpenGL. Mục tiêu là trình bày được dữ liệu hình học, ma trận biến đổi, camera và phép chiếu. Em dùng một đối tượng quen thuộc để có thể kiểm chứng bằng hình ảnh và số liệu ngay trên ứng dụng.”

“Ứng dụng có năm phần. Phần trò chơi yêu cầu ghép hướng mặt trên và mặt trước, giúp áp dụng phép quay và thuật toán tìm kiếm BFS.”

## 0:45–2:00 / F3 mô hình

Chọn **Khối cơ bản**, bật Khung dây. Chỉ các mặt, cạnh và chấm. Tắt khung dây, đổi về Khối bo cạnh. Nhấn Tách ra hai lần, chọn mặt 3.

“Khối vuông có 8 vị trí góc, nhưng em lưu 24 bản ghi đỉnh để mỗi mặt có pháp tuyến riêng. Thân bo được sinh từ lưới chia nhỏ trên sáu mặt, tổng 2646 bản ghi đỉnh và 4800 tam giác. Đây là danh sách đỉnh và chỉ số tam giác, tương ứng phần mô hình mặt đa giác trong bài học.”

“Mỗi mặt có u, v, n với u nhân có hướng v bằng n. Tâm chấm tính từ các tọa độ 2D trên mặt rồi đưa ra 3D. Có 21 chấm, mỗi chấm là đĩa tròn 32 tam giác. Các mặt đối nhau có tổng bằng 7.”

Thu các mặt lại trước khi chuyển trang để phần sau dễ nhìn.

## 2:00–4:10 / F4 biến đổi

Chọn **Tịnh tiến**, trục X, rồi **Chạy**. Chọn **Quay**, trục Z, rồi Chạy để cho thấy mỗi phép hoạt động riêng. Tạm dừng rồi nhấn **Tiến 25%** để đối chiếu hình với ma trận. Chọn **Tổ hợp → T rồi R → Chạy**, sau đó **R rồi T → Chạy** để so sánh hai thứ tự.

“Với vector cột, T rồi R có ma trận R nhân T. Tâm đi đến (0,2,0). Đổi thứ tự thì tâm ở (2,0,0). Đây là ví dụ phép nhân ma trận không giao hoán.”

“Tài liệu của thầy dùng vector hàng P nhân M. Chương trình dùng vector cột M nhân P theo OpenGL. Em chuyển vị ma trận và đảo thứ tự viết tích để hai quy ước tương đương.”

“Chạy từng bước giữ riêng con trỏ phát lại. Ma trận và hình dùng cùng một tỉ lệ nội suy. Tạm dừng giữ nguyên trạng thái, mỗi lần Tiến 25% tăng một phần tư phép đang xem. Đổi phép sẽ chuẩn bị minh họa mới từ đầu.”

Chọn **Quay → Trục k → Tâm P**, rồi Chạy. Bật Pháp tuyến, chỉ điểm vàng P và đường k. Sau đó chọn **Tổ hợp → Phép ngược → Chạy**, rồi Xem M⁻¹.

“Quay quanh trục qua P dùng T(P), R, T(-P). Điểm trên trục giữ nguyên. Với tỉ lệ không đều hoặc biến dạng xiên, pháp tuyến cần nghịch đảo chuyển vị. Ma trận tỉ lệ bằng 0 không có nghịch đảo.”

## 4:10–5:25 / F5 camera và phép chiếu

Nhấn Phối cảnh, kéo camera một chút, nhấn C. Chuyển Trực giao rồi Xiên 1/2. Thử Mặt trước hoặc Nhìn trên.

“Pipeline biến điểm từ Model sang World, Eye và Clip bằng P V M. Sau đó chia cho w để ra NDC và ánh xạ vào viewport. Bảng bên phải tính đúng điểm vàng đang đánh dấu.”

“Phối cảnh làm vật xa nhỏ lại. Trực giao giữ tỉ lệ theo độ sâu khi giữ nguyên hộp nhìn. Chiếu Cabinet là phép xiên song song với hệ số chiều sâu một phần hai.”

“Kéo chuột đổi góc nhìn, cuộn đổi khoảng cách R. Góc FOV tác động độ phóng của ảnh. Eye space OpenGL nhìn theo -Z.”

## 5:25–6:50 / F2 mô phỏng

Giữ Space khoảng nửa giây rồi thả. Khi đang bay, nhấn P và Bước 1/120 s. Chỉ số vị trí, vận tốc và omega. Nhấn P để tiếp tục. Khi dừng, chỉ viền vàng mặt trên và phép cộng kết quả.

“Mỗi viên có vị trí, quaternion, vận tốc và vận tốc góc riêng. Em dùng Euler bán ẩn với bước thời gian cố định, sau đó kiểm tra tiếp xúc và áp xung lực.”

“Bàn và thành dùng support của khối bo. Va giữa các viên dùng cầu đại diện. Khi gần dừng có nội suy ổn định 0,4 giây. Đây là các xấp xỉ em công bố trong báo cáo.”

“Em không lấy số ngẫu nhiên rồi in lên giao diện. Kết quả là mặt có pháp tuyến sau quay cùng hướng +Y nhất. Vì vậy kết quả khớp mặt thực tế khi viên đã dừng.”

## 6:50–8:10 / F6 trò chơi

Chọn viên 1, quay X/Y/Z, nhấn H một lần để thấy gợi ý. Chỉ cột mặt trên/mặt trước trong bảng mục tiêu. Có thể giải một viên thay vì giải cả bài khi thời gian ít.

“Một khối lập phương có 24 hướng rời rạc. Mỗi trạng thái gồm cặp mặt trên và mặt trước. Một cạnh là phép quay ±90 độ quanh X, Y hoặc Z. Vì mọi bước cùng chi phí, BFS cho đường có ít phép quay nhất.”

“Gợi ý lấy bước đầu của đường BFS. Số bước tối ưu toàn bài là tổng ba đường ngắn nhất vì các viên độc lập. Hoạt ảnh dùng SLERP và khóa đầu vào trong 0,24 giây để tránh chồng lệnh.”

## 8:10–9:00 / Dữ liệu và kết thúc

“Em lưu lưới bằng danh sách đỉnh và chỉ số trong RAM, dùng chung cho các viên. Lịch sử lượt đổ lưu CSV. Em cũng xuất được thân lưới OBJ và danh sách phép biến đổi CSV để kiểm tra bên ngoài.”

“Đồ án đã kiểm tra công thức hình học, phép chiếu, BFS và các tình huống mô phỏng, cùng kiểm tra OpenGL trên cửa sổ thật. Giới hạn chính là va chạm giữa viên còn gần đúng và pipeline cố định. Hướng tiếp theo là bộ giải va chạm tốt hơn và chuyển sang shader.”

## Nếu chỉ có 5 phút

F3 tách mặt khoảng 45 giây. F4 so sánh hai thứ tự và quay qua P khoảng 90 giây. F5 phép chiếu khoảng 45 giây. F2 một lượt đổ khoảng 60 giây. F6 giới thiệu 24 trạng thái và BFS khoảng 45 giây. Dành phần còn lại cho kết luận.
