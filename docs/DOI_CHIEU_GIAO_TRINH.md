# Đối chiếu tài liệu môn học với ứng dụng

Số trang tính theo PDF 51 trang `reference/CG_3D_Graphics.pdf`, trùng số in trên các trang nội dung.

| Trang PDF | Nội dung | Cách chứng minh trong ứng dụng | Mã nguồn |
|---|---|---|---|
| 5–6 | Tọa độ thuần nhất, tịnh tiến | F4, ma trận 4×4, cột tịnh tiến, O' | `lab.cpp` / Mat4, TransformStep |
| 7–10 | Đối xứng, tỉ lệ, tâm bất kỳ | F4 chọn Đối xứng/Tỉ lệ, X/Y/Z, bật qua P | `TransformStep::matrix` |
| 11–14 | Quay X/Y/Z và trục bất kỳ | F4 mẫu Quay qua P, hiện đường k và điểm P | `Quat::axisAngle`, `TransformStep::matrix` |
| 15 | Phép biến đổi ngược | F4 mẫu Phép ngược, nút M⁻¹ | `Mat4::inverse` |
| 16 | Kết hợp biến đổi | So sánh T rồi R với R rồi T | `Lab::at` |
| 19–23 | Wireframe, đỉnh/cạnh | F3 đổi khối cơ bản, bật Khung dây | `basicCube`, `body` |
| 24–26 | Polygon mesh, danh sách mặt | F3 lưới bo 2646 bản ghi đỉnh/4800 tam giác, xuất OBJ | `roundedCube`, `exportOBJ` |
| 28–33 | Chiếu song song, trực giao, xiên | F5 trực giao, ba hướng nhìn, Cabinet 1/2 | `orthographic`, `cabinet` |
| 34–36 | Chiếu phối cảnh | F5 phối cảnh, so sánh các khối ở các độ sâu | `perspective` |
| 38–45 | Hệ quan sát | F5 thay camera, xem tọa độ Eye và ma trận V | `lookAt` |
| 46–49 | Chiếu ra màn hình, θ/φ/R/D | F5 kéo/zoom/FOV, theo dõi Clip/NDC/Pixel | `project`, `setupView` |

## Hai quy ước cần nói rõ

Tài liệu dùng vector hàng `Q_hàng = P_hàng M_hàng`. Chương trình dùng vector cột `q_cột = M_cột p_cột`, với `M_cột = transpose(M_hàng)`. Đổi quy ước làm đảo thứ tự viết tích. Thực hiện T rồi R có tích `T_hàng R_hàng` trong tài liệu, tương ứng `R_cột T_cột` trong chương trình.

OpenGL lưu mảng 16 số theo **column-major**. Đây là quy ước lưu bộ nhớ, cần phân biệt với quy ước vector hàng/cột. Cả hai đã được thống nhất trong `Mat4` và `glMultMatrixd`.

Phần quan sát của tài liệu xây hệ mắt có hướng trục riêng. Chương trình giữ quy ước OpenGL, camera nhìn theo **-Z** trong eye space. Không chép nguyên ma trận hệ mắt của tài liệu vào OpenGL khi chưa chuyển quy ước.

## Điều kiện của phép nghịch đảo

Trang 15 nêu cách tìm nghịch đảo các phép cơ sở. Với phép tỉ lệ, các hệ số phải khác 0. Ma trận affine tổng quát chỉ có nghịch đảo khi định thức phần tuyến tính khác 0. `Mat4::inverse` kiểm tra suy biến bằng Gauss-Jordan có chọn phần tử trụ. Đối xứng thực hiện rời rạc ở cuối bước để tránh đi qua tỉ lệ bằng 0 khi nội suy.

## Các phần mở rộng của đồ án

Quaternion, SLERP, hỗ trợ va chạm, bóng phẳng, vật liệu và BFS là phần ứng dụng/mở rộng do chương trình triển khai. Không gán những phần này thành nội dung đã được trình bày trong PDF. Tài liệu trên cung cấp nền tảng về mô hình, biến đổi, phép chiếu và quan sát.
