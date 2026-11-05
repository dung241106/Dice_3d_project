# Câu hỏi bảo vệ đồ án

## 1. Tại sao chọn xúc xắc?

Đối tượng đủ đơn giản để giải thích dữ liệu hình học nhưng có đủ sáu hướng mặt, chuyển động tịnh tiến/quay và nhiều trường hợp quan sát. Có thể kiểm tra kết quả trực tiếp bằng mặt trên, tránh chỉ trình diễn một hình trang trí.

## 2. Xúc xắc có mấy đỉnh? Sao mã có 24 hoặc 2646?

Khối cơ bản có 8 vị trí góc duy nhất. Lưới flat shading tách bản ghi theo mặt nên có 24 bản ghi position/normal. Lưới bo n=20 có 6(20+1)^2=2646 bản ghi và 4800 tam giác. Đây là các biểu diễn khác nhau của đối tượng.

## 3. Vì sao mỗi mặt có chấm đúng vị trí?

Mỗi mặt có u,v,n trực chuẩn. Các vị trí (a,b) dùng chung một mẫu 2D, đổi ra 3D bằng n(h+epsilon)+u*a+v*b. Không viết lại sáu bộ công thức tọa độ.

## 4. Chấm là texture hay hình học?

Là đĩa tam giác thủ tục, 32 tam giác/chấm, tổng 21 chấm. Texture hiện dùng cho vân gỗ. Chấm chưa khắc lõm.

## 5. Ma trận trong chương trình khác slide thầy?

Tài liệu dùng vector hàng P*M. Chương trình dùng vector cột M*P, ma trận tương ứng là chuyển vị. T rồi R viết T*R theo hàng và R*T theo cột. Đặc biệt column-major chỉ nói cách lưu bộ nhớ, không tự quyết định quy ước vector.

## 6. Tại sao T rồi R khác R rồi T?

Phép quay quanh gốc làm quay cả vị trí đã tịnh tiến nếu quay sau. T(2,0,0) rồi Rz(90°) đưa tâm tới (0,2,0). Nếu R trước rồi T sau, tâm tới (2,0,0). Mở F4 để chứng minh.

## 7. Vì sao chọn phép rồi Chạy không làm các phép trước đó?

Mỗi lựa chọn chuẩn bị một minh họa riêng gồm một ma trận biến đổi và trạng thái ban đầu. Chạy chỉ nội suy phép đang chọn. Các ví dụ nhiều phép được đặt riêng trong mục Tổ hợp. Vì vậy chọn Quay sẽ không tự chạy một phép tịnh tiến cũ trước đó.

## 8. Quay quanh một điểm hoặc trục bất kỳ như thế nào?

Tịnh tiến P về gốc, quay quanh hướng k, đưa P trở lại, tức T(P)*R(k,alpha)*T(-P). Trong mã quay theo quaternion trục-góc. Mẫu dùng P=(1,0,0), k=(1,1,0), góc 60°. Mọi điểm P+lambda*k phải giữ nguyên.

## 9. Mọi ma trận đều có nghịch đảo?

Không. Ma trận suy biến không có nghịch đảo. Tỉ lệ bằng 0 là ví dụ. Tịnh tiến và quay có nghịch đảo bằng đổi dấu tham số, tỉ lệ dùng nghịch đảo từng hệ số khác 0. Tổ hợp có (A*B)^-1=B^-1*A^-1.

## 10. Vì sao pháp tuyến phải dùng nghịch đảo chuyển vị?

Để giữ n' vuông góc với tiếp tuyến A*t sau tỉ lệ không đều hoặc shear. Nếu n'=A^-T*n, thì n'·(A*t)=n·t=0. Chuẩn hóa lại sau phép biến đổi.

## 11. Quaternion có thay thế ma trận không?

Quaternion lưu và nội suy hướng quay. Khi dựng vẫn đổi thành ma trận 4×4 để ghép với tịnh tiến và camera. Nó không biểu diễn toàn bộ phép tịnh tiến/tỉ lệ của model trong cấu trúc Quat này.

## 12. Phép chiếu và camera khác nhau thế nào?

View đưa thế giới vào hệ mắt. Projection quyết định cách ánh xạ từ thể tích nhìn ra clip space. Chúng có vai trò riêng trong P*V*M. Thay vị trí camera thay hệ quan sát, thay FOV/hộp nhìn thay phép chiếu.

## 13. Vì sao cần chia cho w?

Ma trận phối cảnh tạo w phụ thuộc độ sâu. Chia x,y,z cho w tạo hiệu ứng vật xa nhỏ lại. Trong OpenGL chuẩn ở đây w=-z_eye. Cần xử lý w gần 0 và clipping trước khi kết luận điểm hiển thị được.

## 14. Chiếu Cabinet khác trực giao?

Cabinet thêm độ lệch x/y theo z trước khi trực giao. Hệ số 1/2 làm chiều sâu biểu diễn ngắn đi. Các tia vẫn song song, không hội tụ như phối cảnh.

## 15. Kết quả đổ tính bằng random hay bằng mặt trên?

Mô phỏng random vận tốc ban đầu và omega. Khi dừng, lấy argmax dot(R(q)*n_i,Y). Giá trị mặt này là giá trị lưu CSV và hiện giao diện. Trò ghép hướng random mục tiêu riêng trên 24 hướng.

## 16. Có mô phỏng vật lý chính xác hoàn toàn không?

Chưa. Bàn/thành dùng support khối bo, va giữa viên dùng proxy cầu và có SLERP ổn định gần dừng. Các xấp xỉ được ghi rõ. Chưa có SAT/GJK, contact manifold hay kiểm định phân phối công bằng.

## 17. Vì sao không cập nhật vật lý mỗi lần vẽ?

Tốc độ vẽ thay đổi giữa các máy. Bước cố định giúp kết quả ổn định hơn và tái hiện được. Vòng lặp tích lũy thời gian và tiến từng dt=1/120 giây, với bước con không quá 1/240 giây.

## 18. Sao BFS phù hợp với trò chơi?

Đồ thị hữu hạn, 24 đỉnh, mọi phép quay có cùng chi phí 1. BFS thăm theo lớp nên tìm ít phép quay nhất. Nếu mỗi phép quay có giá khác nhau, cần cân nhắc Dijkstra.

## 19. Vì sao 24 hướng, không phải 36 cặp mặt?

Chọn mặt trên có 6 cách. Mặt trước phải kề mặt trên, chỉ có 4 cách, không thể là chính mặt trên hoặc mặt đối diện. Tổng 6*4=24. Đây là các hướng vuông góc của khối, không phải số quaternion liên tục.

## 20. Lưu những gì? Có phát lại quỹ đạo không?

Lưu kết quả, số viên, lực, thời gian và số va bàn/thành trong CSV. Mở lại phục hồi mặt trên ở vị trí chuẩn. Chưa lưu chuỗi vị trí/quaternion theo thời gian nên chưa có replay quỹ đạo chính xác. OBJ xuất thân lưới, CSV biến đổi xuất các thao tác của Lab.

## 21. Ánh sáng và bóng được tạo thế nào?

Chiếu sáng cố định ambient/diffuse/specular từ hai đèn và pháp tuyến. Bóng phẳng dùng ma trận chiếu theo nguồn sáng, stencil giới hạn trên bàn và alpha blending. Bóng tiếp xúc thêm ellipse. Chưa dùng shadow map hay PBR.

## 22. Nét riêng về đồ họa của đồ án?

Tự sinh lưới bo và chấm theo cơ sở mặt, có thí nghiệm ma trận với lịch sử đúng từng bước, so sánh ba phép chiếu, theo dõi một đỉnh qua toàn pipeline và đọc kết quả từ hướng hình học. Có thể trình bày mỗi ý bằng thao tác và số liệu cụ thể.
