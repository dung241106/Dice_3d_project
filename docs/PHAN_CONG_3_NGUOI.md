# Phân công 3 người - Bản trình bày gọn

Mục tiêu: nói đủ các ý của đề tài, giải thích bằng demo và chỉ những đoạn code chính. Tổng thời gian gợi ý khoảng **8–9 phút**.

| Thành viên | Nội dung phụ trách | Thời gian |
|---|---|---|
| **Người 1** | Giới thiệu, cách làm, đối tượng và lưu trữ dữ liệu | 2–3 phút |
| **Người 2** | Biểu diễn xúc xắc, mô hình hóa thân/mặt/chấm và dựng hình | 3 phút |
| **Người 3** | Các phép biến đổi và thuật toán mô phỏng, trò chơi | 3 phút |

## Người 1 - Tổng quan, đối tượng và dữ liệu

**Cần nói những gì?**

1. **Giới thiệu:** đề tài mô phỏng xúc xắc 3D và trò chơi đổ xúc xắc bằng C++/OpenGL. Mục tiêu là áp dụng kiến thức đồ họa vào một đối tượng có thể tương tác.
2. **Cách làm:** xây dữ liệu hình học → dựng xúc xắc → thêm biến đổi/chuyển động → xử lý tương tác và trò chơi → lưu kết quả.
3. **Đối tượng:** cảnh có xúc xắc, khay, camera và ánh sáng. Trong code, `Mesh` lưu lưới, `Die` lưu vị trí/hướng quay/giá trị của một viên, `ThrowRecord` lưu kết quả một lượt.
4. **Lưu trữ:** lưới và trạng thái hiện tại nằm trong RAM. Phiên thường lưu lịch sử lượt đổ bằng CSV; chế độ thuyết trình giữ tạm trong RAM. Có thể xuất thân lưới ra OBJ để kiểm tra bên ngoài.

**Code cần chỉ:** nhóm cấu trúc `Mesh`, `Die` trong [dice.hpp](../include/dice.hpp), `ThrowRecord` trong [throw_sim.hpp](../include/throw_sim.hpp); nếu cần minh họa lưu file thì chỉ hàm `save()` trong [throw_sim.cpp](../src/throw_sim.cpp).

**Demo:** mở ứng dụng, giới thiệu nhanh các trang và chỉ vùng lịch sử kết quả. Phần này chưa cần đổ xúc xắc.

**Lời nói gợi ý:**

> “Đề tài của nhóm em là mô phỏng xúc xắc 3D bằng C++ và OpenGL. Nhóm xây dữ liệu hình học trước, sau đó thêm biến đổi, chuyển động và tương tác. Trong code, lưới hình học được tách khỏi trạng thái từng viên để có thể dùng chung mô hình. Phiên chạy thường lưu kết quả các lượt đổ vào file CSV.”

**Chuyển lời:** “Sau đây, bạn tiếp theo sẽ giải thích cách tạo thân, mặt và chấm của xúc xắc.”

## Người 2 - Mô hình hóa và dựng hình

**Cần nói những gì?**

1. **Biểu diễn xúc xắc:** bắt đầu từ khối lập phương có 6 mặt, rồi bo cạnh cho hình mềm hơn. Thân được biểu diễn bằng đỉnh và các tam giác nối giữa chúng.
2. **Mô hình hóa từng mặt:** mỗi mặt có số từ 1 đến 6 và một hướng pháp tuyến riêng. Các mặt đối nhau có tổng bằng 7.
3. **Mô hình hóa chấm:** chấm là các đĩa tròn bằng tam giác, đặt theo mẫu 1–6 trong tọa độ của từng mặt. Nhờ biết hướng của mặt, cùng một mẫu có thể đặt đúng lên cả sáu phía.
4. **Dựng hình:** OpenGL vẽ các tam giác của thân và chấm. Pháp tuyến giúp tính ánh sáng, depth buffer giúp xử lý che khuất. Camera và phép chiếu đưa cảnh 3D thành ảnh trên màn hình.

**Code cần chỉ:** `roundedCube()` trong [dice.cpp](../src/dice.cpp) và `pips()` trong [main.cpp](../src/main.cpp). Giải thích chúng tạo thân và đặt chấm như thế nào.

**Demo:** F3 → Khối cơ bản → bật Khung dây → đổi sang Khối bo cạnh → Tách ra → chọn một mặt. Nếu còn thời gian, chuyển F5 và đổi Phối cảnh/Trực giao để minh họa cách quan sát.

**Lời nói gợi ý:**

> “Xúc xắc được xây từ một khối lập phương gồm sáu mặt. Thân được chia thành các tam giác và bo cạnh để nhìn tự nhiên hơn. Mỗi mặt có hướng riêng để đặt các chấm đúng vị trí. OpenGL dựng phần thân và chấm, sau đó tính ánh sáng và xử lý các mặt che nhau. Em bật khung dây và tách mặt để thầy quan sát cấu trúc này.”

**Chuyển lời:** “Từ mô hình đã dựng, bạn cuối sẽ trình bày cách biến đổi hình và vận hành trò chơi.”

## Người 3 - Biến đổi hình và thuật toán trò chơi

**Cần nói những gì?**

1. **Biến đổi 3D:** tịnh tiến đổi vị trí, quay đổi hướng, tỉ lệ đổi kích thước; ngoài ra có đối xứng và biến dạng xiên. Các phép dùng ma trận 4×4. Tịnh tiến rồi quay có thể cho kết quả khác quay rồi tịnh tiến.
2. **Thuật toán đổ:** lấy lực → khởi tạo vận tốc và vận tốc góc → cập nhật chuyển động dưới trọng lực → xử lý va chạm → cho viên ổn định và đọc kết quả. Phần cập nhật chuyển động dùng Euler bán ẩn, tức cập nhật vận tốc trước vị trí.
3. **Đọc mặt trên:** quay pháp tuyến của sáu mặt theo hướng viên hiện tại, lấy mặt gần hướng lên +Y nhất. Giá trị hiển thị lấy từ mặt thực tế đó.
4. **Thuật toán trò ghép hướng:** có 24 hướng rời rạc của khối lập phương. Mỗi bước quay ±90° là một cạnh; BFS tìm chuỗi ít bước nhất để làm gợi ý.

**Code cần chỉ:** `TransformStep::matrix()` trong [lab.cpp](../src/lab.cpp) và `topFace()` trong [dice.cpp](../src/dice.cpp). Phần mô phỏng và BFS giải thích theo quy trình trên; chuẩn bị `integrate()` và `shortestTurns()` để mở khi thầy hỏi thêm.

**Demo:** F4 → chọn Tịnh tiến hoặc Quay → Chạy. Sau đó F2 → đổ một lượt → chỉ kết quả khớp mặt trên. Cuối cùng F6 → nhấn H để minh họa gợi ý BFS.

**Lời nói gợi ý:**

> “Chương trình dùng ma trận để thay đổi vị trí, hướng và kích thước của xúc xắc. Khi đổ, trạng thái được cập nhật theo thời gian, có trọng lực và phản hồi va chạm. Kết quả lấy từ mặt thực sự hướng lên khi viên dừng. Với trò ghép hướng, chương trình dùng BFS trên 24 hướng của khối để tìm chuỗi quay ít bước nhất.”

**Kết luận ngắn:**

> “Qua đề tài này, nhóm vận dụng mô hình hóa, biến đổi 3D, dựng hình và tương tác. Va chạm giữa các viên hiện được mô phỏng gần đúng bằng hình cầu và có bước ổn định trực quan. Nhóm có thể tiếp tục cải thiện phần vật lý và chất lượng hình ảnh.”

## Đối chiếu các ý yêu cầu

| Ý cần có | Người trình bày |
|---|---|
| Giới thiệu đề tài, cách làm | Người 1 |
| Tạo đối tượng gì, lưu trữ dữ liệu ra sao | Người 1 |
| Biểu diễn xúc xắc, mô hình hóa thân | Người 2 |
| Mô hình hóa mặt và chấm | Người 2 |
| Dựng hình như thế nào | Người 2 |
| Biến đổi hình ra sao | Người 3 |
| Đổ xúc xắc và xác định kết quả bằng cách nào | Người 3 |
| Thuật toán trò chơi | Người 3 |

Mỗi người tập theo thứ tự **nói ý chính → chỉ code → demo kết quả**. Các con số chi tiết và công thức sâu để trong báo cáo, dùng khi thầy hỏi thêm.
