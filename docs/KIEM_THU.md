# Kiểm thử bản học thuật

Lần xác nhận ngày 05/11/2026, Windows, MSYS2 UCRT64. Đây là kết quả của phiên bản nằm trong thư mục project_3d_version2.

## Lệnh đã chạy

```powershell
.\build.ps1 -Test
.\build\Dice3D_Edu.exe --smoke captures
```

Kết quả thực tế:

```text
Build OK: build\Dice3D_Edu.exe
PASS: 273266 geometry / projection / BFS checks
PASS: 246 transform checks
PASS: 169540 throw/audio checks (90 simulated rounds)
OpenGL smoke: PASS (0 errors) / 4.6.0 - Build 32.0.101.7026
```

Tổng 443052 lần kiểm tra điều kiện. Đây là các assertion, không phải 443052 tình huống độc lập. Vòng kiểm tra mô phỏng có nhiều assertion ở từng bước thời gian.

## Phạm vi

| Nhóm | Điều đã xác nhận |
|---|---|
| Hình học | Cơ sở mặt, số chấm, mặt đối, hướng tam giác, số phần tử lưới, pháp tuyến |
| Ma trận | T/R/S/shear/mirror, nghịch đảo, suy biến, trục bất kỳ, quaternion tương ứng ma trận |
| Phép chiếu | View, phối cảnh theo độ sâu, ortho, Cabinet, viewport, w=0 và điểm sau mắt |
| Playback | Cả 5 phép chạy riêng trên X/Y/Z, dừng giữ nguyên hình, mỗi lần tiến đúng 25%, chọn phép mới bỏ chuỗi cũ |
| BFS | 24 hướng duy nhất, đường đi cho mọi cặp hướng, bài giải tối ưu và khóa hoạt ảnh |
| Mô phỏng | Nảy bàn, đổi hướng ở thành, xung lực giữa viên, lực đổ, 90 lượt với 1–3 viên |
| Kết quả | Giá trị bản ghi bằng mặt trên thật, viên dừng có mặt nằm phẳng, không ghi hai lần |
| Dữ liệu | CSV round-trip, file lỗi không đổi RAM, OBJ đúng số phần tử, CSV biến đổi có đủ pivot/frame/trục |
| OpenGL | 26 cảnh chụp, ma trận CPU khớp GL, không GL error, khung hình thay kích thước |
| Đầu vào | Giữ/thả Space, nút lấy lực qua hitbox thật, kéo chuột phải, tạm dừng và single step |

Kiểm tra --smoke không đọc/ghi lịch sử thường. Nó tạo các ảnh BMP trong thư mục được chỉ định, đã chuyển thành PNG tại docs/images để làm tài liệu.

## Kiểm tra tài liệu

PowerPoint đã mở thành công bộ 14 slide, xuất PDF và 14 ảnh xem trước. Đã kiểm tra ảnh thực tế để sửa tương thích bảng và chỗ chữ quá dài. Báo cáo dùng font nhúng tiếng Việt, đã render PDF bằng Poppler để kiểm tra bố cục.

## Chưa đo hoặc chưa chứng minh

- Chưa đo FPS và mức sử dụng GPU trên nhiều máy.
- Chưa kiểm định thống kê tính công bằng của kết quả mô phỏng.
- Kiểm tra PCM xác nhận cấu trúc dữ liệu âm thanh. Chưa thay thế việc nghe thử chất lượng loa.
- Va giữa viên dùng hình cầu đại diện, không kiểm chứng tiếp xúc chính xác giữa hai lưới.
- Build hiện xác nhận trên Windows. Các nhánh font/audio dự phòng ngoài Windows chưa được chạy thực tế.

## Kiểm tra nhanh trước giờ trình bày

Chạy CHAY_THUYET_TRINH.bat, thử F3 tách mặt, F4 hai thứ tự biến đổi, F5 ba phép chiếu, F2 một lượt và F6 gợi ý BFS. Kiểm tra F11 trên màn chiếu, thử âm thanh ở mức phù hợp và chuẩn bị bản PDF slide dự phòng.
