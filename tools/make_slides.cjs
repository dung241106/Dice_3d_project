// Rebuild the editable presentation. No network assets are required.
const path = require('node:path');
const fs = require('node:fs');
const PptxGenJS = require('C:/Users/LOQ/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/pptxgenjs');
const root = path.resolve(__dirname, '..');
const pptx = new PptxGenJS();
pptx.layout = 'LAYOUT_WIDE';
pptx.author = 'Dice 3D Lab';
pptx.subject = 'Đồ án Đồ họa máy tính';
pptx.title = 'Mô phỏng Xúc xắc 3D';
pptx.company = 'Computer Graphics';
pptx.lang = 'vi-VN';
pptx.theme = { headFontFace:'Arial', bodyFontFace:'Arial', lang:'vi-VN' };
const C = { bg:'091320', white:'ECF4FC', muted:'ACC0D0', accent:'4DD6CB', gold:'FFC570' };
const image = name => path.join(root, 'docs', 'images', name+'_scene.png');
const manifest = [];
function text(slide, value, x, y, w, h, opts={}) {
  slide.addText(value, {x,y,w,h,fontFace:'Arial',fontSize:22,color:C.white,margin:0,breakLine:false,
    valign:'top',paraSpaceAfterPt:14,...opts});
}
function slide(title, notes, source='Mã nguồn và báo cáo của đồ án') {
  const s = pptx.addSlide(); s.background={color:C.bg};
  text(s,title,.6,.52,11.7,.78,{fontSize:32,bold:true});
  text(s,String(pptx._slides.length).padStart(2,'0'),12.2,.60,.5,.3,{fontSize:14,color:C.muted,align:'right'});
  text(s,source,.65,7.03,12,.25,{fontSize:12,color:C.muted});
  s.addNotes(notes+'\n\nNguồn: '+source);
  manifest.push({title, notes, source}); return s;
}
function scene(slide,name,x=5.1,y=1.82,w=7.6) {
  const file=image(name),png=fs.readFileSync(file),ratio=png.readUInt32BE(20)/png.readUInt32BE(16);
  const maxH=w*548/990;let h=w*ratio;
  if(h>maxH){const nextW=maxH/ratio;x+=(w-nextW)/2;w=nextW;h=maxH;}
  slide.addImage({path:file,x,y,w,h});
}
function points(slide, lines, x=.65,y=1.8,w=4.05) {
  lines.forEach((line,i)=>text(slide,line,x,y+i*1.12,w,1.0,{fontSize:22,color:i===0?C.white:C.muted}));
}
function table(slide, rows, x=.65,y=1.8,w=12.0, widths) {
  const styled=rows.map((row,i)=>row.map(value=>({text:String(value),options:{color:i===0?C.accent:C.white,bold:i===0}})));
  slide.addTable(styled,{x,y,w,border:{type:'solid',color:'243B50',pt:.5},fill:C.bg,color:C.white,
    fontFace:'Arial',fontSize:19,margin:12,rowH:.59,colW:widths,autoPage:false,
    bold:false,paraSpaceAfterPt:2,valign:'middle'});
}
(async()=>{
  let s=pptx.addSlide();s.background={color:C.bg};
  text(s,'XÚC XẮC\n3D',.65,1.15,4.2,2.1,{fontSize:52,bold:true});
  text(s,'Mô phỏng và trò chơi\nĐồ án Đồ họa máy tính',.65,3.65,4.2,1.2,{fontSize:25,color:C.muted});
  text(s,'C++17 / OpenGL / FreeGLUT',.65,5.45,4.2,.7,{fontSize:20,color:C.accent});
  scene(s,'01_simulation',5.05,1.6,7.8);
  s.addNotes('Giới thiệu đề tài. Mục tiêu là biểu diễn được mô hình, giải thích ma trận biến đổi, phép chiếu và camera. Phần trò chơi là ghép hướng mặt để ứng dụng phép quay và BFS. Chuyển qua các phần có thể chứng minh trực tiếp trên ứng dụng.');
  manifest.push({title:'Mô phỏng Xúc xắc 3D',notes:'Giới thiệu đề tài.',source:'Ứng dụng C++/OpenGL của đồ án'});

  s=slide('Nội dung đồ án và bài giảng','Nêu bốn nhóm nội dung trong tài liệu. Năm trang chức năng phân tách phần thao tác và phần quan sát. Chỉ cần demo một ví dụ đại diện cho mỗi nhóm.', 'Bài giảng 3D Graphics, tr. 2, 5–16, 19–26, 28–49');
  text(s,'Mô hình hóa',.8,1.9,5,.5,{fontSize:29,color:C.accent});text(s,'Đỉnh, tam giác, pháp tuyến và chấm',.8,2.55,5,1,{fontSize:24});
  text(s,'Biến đổi 3D',7,1.9,5,.5,{fontSize:29,color:C.accent});text(s,'Các phép cơ sở, tổ hợp và phép ngược',7,2.55,5,1,{fontSize:24});
  text(s,'Phép chiếu',.8,4.3,5,.5,{fontSize:29,color:C.accent});text(s,'Phối cảnh, trực giao và Cabinet',.8,4.95,5,1,{fontSize:24});
  text(s,'Hệ quan sát',7,4.3,5,.5,{fontSize:29,color:C.accent});text(s,'Camera và tọa độ qua pipeline',7,4.95,5,1,{fontSize:24});

  s=slide('Các đối tượng và dữ liệu','Giải thích một Mesh được dùng lại cho nhiều Die. Mỗi viên chỉ có vị trí/hướng riêng. Lab và trò chơi giữ trạng thái độc lập, mã dựng hình đọc các trạng thái này. Tránh nói mỗi viên tạo lại toàn bộ đỉnh trong từng khung.');
  table(s,[['Đối tượng','Dữ liệu chính','Vai trò'],['Mesh','vertices, indices','Lưới dùng chung'],['Face','value, u, v, normal','Mặt và hệ chấm'],['Die / ThrowBody','p, q, v, omega','Trạng thái từng viên'],['Lab','TransformStep, cursor, phase','Biến đổi và phát lại'],['FacePuzzle','24 hướng, target, moves','Trò chơi và BFS'] ],.65,1.8,12,[2.6,4.8,4.6]);
  text(s,'Nhập lệnh, cập nhật trạng thái và dựng hình',.8,6.2,11.8,.5,{fontSize:23,color:C.gold});

  s=slide('Thân xúc xắc từ lưới tam giác','Khối cơ bản có 8 vị trí góc, nhưng 24 bản ghi để pháp tuyến từng mặt độc lập. Khối bo chia mỗi mặt n=20. Điểm c là điểm p đã chặn vào hộp bên trong. Vertex=c+r normalize(p-c). Có 2646 đỉnh và 4800 tam giác. Demo F3 khung dây và khối bo.', 'Bài giảng tr. 19–26; src/dice.cpp / roundedCube, basicCube');
  points(s,['8 góc hình học\n6 mặt logic','2646 bản ghi đỉnh\n4800 tam giác thân','Lưới chia 20 × 20 ô\ntrên mỗi mặt']);scene(s,'05_model');
  text(s,'v = c + r normalize(p − c)',5.25,6.32,7.4,.45,{fontSize:24,color:C.gold});

  s=slide('Mặt và chấm trong hệ cơ sở u, v, n','Chỉ mặt đang chọn và ba vector. Các mẫu chấm 2D được đưa ra 3D bằng n(h+epsilon)+u*a+v*b. Mỗi chấm dùng quạt 32 tam giác. Các mặt đối tổng 7. Chấm là đĩa hình học bề mặt, chưa phải lỗ lõm. Demo tách mặt.', 'src/dice.cpp / faces, pipCenters; src/main.cpp / pips');
  points(s,['u × v = n','21 chấm\n32 tam giác / chấm','Các mặt đối diện\ncó tổng bằng 7']);scene(s,'06_faces');
  text(s,'c_chấm = n(h + ε) + u a + v b',5.25,6.32,7.4,.45,{fontSize:23,color:C.gold});

  s=slide('Thứ tự biến đổi','T rồi R dùng R*T theo vector cột. Tâm đến (0,2,0). R rồi T dùng T*R, tâm đến (2,0,0). Tài liệu dùng vector hàng, cần chuyển vị ma trận và đảo thứ tự viết tích. Demo F4 mục Tổ hợp, chọn ví dụ rồi Chạy. Các phép đơn chọn riêng trước khi chạy.', 'Bài giảng tr. 5–16; src/lab.cpp / Lab::at');
  text(s,'T rồi R     M = R T',.7,1.65,5.8,.6,{fontSize:27,color:C.accent});text(s,'R rồi T     M = T R',6.95,1.65,5.8,.6,{fontSize:27,color:C.accent});
  scene(s,'08_transform_TR',.65,2.4,6.0);scene(s,'10_transform_RT',6.85,2.4,6.0);
  text(s,"O' = (0, 2, 0)",.8,6.05,5.5,.5,{fontSize:25,color:C.gold});text(s,"O' = (2, 0, 0)",7.05,6.05,5.5,.5,{fontSize:25,color:C.gold});

  s=slide('Quay quanh trục qua P','Ảnh mẫu dùng P=(1,0,0), k=(1,1,0), 60 độ. Trên giao diện chọn Quay, Trục k, Tâm P rồi Chạy. Thứ tự tác động là T(-P), quay, T(P). Điểm trên trục không đổi. Với shear hoặc scale không đều, normal cần A^-T.', 'Bài giảng tr. 10, 13–16; src/lab.cpp / TransformStep::matrix');
  points(s,['P = (1, 0, 0)\nk = (1, 1, 0)','A = T(P) R(k, α) T(−P)','Điểm trên trục k\ngiữ nguyên vị trí']);scene(s,'11_arbitrary_axis');
  text(s,"P' = P",5.25,6.32,7.4,.45,{fontSize:28,color:C.gold});

  s=slide('Phối cảnh và trực giao','Giữ các điều kiện camera, đối chiếu kích thước vật xa/gần. Trực giao giữ kích thước ảnh theo độ sâu khi không đổi hộp nhìn. Cabinet là shear trong eye space rồi ortho, hệ số 1/2. Demo F5 đổi ba chế độ.', 'Bài giảng tr. 28–36; src/view.cpp');
  text(s,'PHỐI CẢNH',.75,1.65,5.8,.5,{fontSize:27,color:C.accent});text(s,'TRỰC GIAO',6.95,1.65,5.8,.5,{fontSize:27,color:C.accent});
  scene(s,'13_perspective',.65,2.4,6);scene(s,'14_orthographic',6.85,2.4,6);
  text(s,'Kích thước ảnh phụ thuộc độ sâu',.8,6.1,5.8,.6,{fontSize:22});text(s,'Kích thước ảnh không đổi theo độ sâu',7.0,6.1,5.8,.6,{fontSize:22});

  s=slide('Một đỉnh qua pipeline','Các tọa độ minh họa lấy từ capture 13_perspective, đỉnh V6. Model=(.62,.62,.62). Ma trận của inspector và ma trận nạp OpenGL giống nhau. Pixel là hệ tọa độ trong viewport, gốc góc trên trái. Clip chia w cho ra NDC, rồi viewport. Nói rõ camera OpenGL nhìn -Z.', 'Bài giảng tr. 38–49; capture 13_perspective; src/view.cpp / project');
  table(s,[['Không gian','Tọa độ V6, làm tròn'],['Model','(0.62, 0.62, 0.62)'],['World','(0.77, 0.54, 0.51)'],['Eye','(0.50, 0.01, −12.55)'],['Clip','(0.71, 0.03, 12.38, 12.55)'],['NDC','(0.06, 0.00, 0.99)'],['Pixel trong khung','(523.2, 273.4)'] ],.65,1.65,7.5,[2.55,4.95]);
  text(s,'p_clip = P V M p',8.55,2.05,4.1,1,{fontSize:26,color:C.gold});
  text(s,'NDC = clip / w',8.55,3.55,4.1,1,{fontSize:26,color:C.accent});
  text(s,'Camera nhìn theo −Z\ntrong eye space',8.55,5.0,4.1,1.2,{fontSize:24});

  s=slide('Chuyển động và va chạm','Giữ Space lấy lực. Euler bán ẩn cập nhật vận tốc trước vị trí. Quaternion cập nhật theo omega. Bàn/thành dùng support khối bo. Va giữa viên dùng cầu đại diện bán kính .78. Khi gần dừng, nội suy .4 giây để ổn định. Đây là giới hạn được công bố, chưa là bộ giải vật rắn đầy đủ.', 'src/throw_sim.cpp / integrate, contact, constrain, settle');
  points(s,['dt = 1/120 giây\nBước con ≤ 1/240 giây','v mới = v + g dt\np mới = p + v mới dt','Tiếp xúc, xung lực\nvà giảm chấn']);scene(s,'02_in_motion');
  text(s,'Va giữa viên dùng cầu đại diện và có bước ổn định',5.2,6.3,7.5,.65,{fontSize:21,color:C.gold});

  s=slide('Kết quả từ mặt hướng lên','Không sinh một số độc lập để in lên giao diện của trang mô phỏng. Đọc sáu pháp tuyến đã quay, mặt nào có dot với +Y lớn nhất là kết quả. Khi dừng, CSV lấy chính giá trị này. Chưa có kiểm định phân phối công bằng do va chạm xấp xỉ và ổn định.', 'src/dice.cpp / topFace; src/throw_sim.cpp / step');
  points(s,['Quay pháp tuyến\nbằng quaternion q','So sánh với\nvector hướng lên Y','Lấy mặt có\ntích vô hướng lớn nhất']);scene(s,'04_result');
  text(s,'value = argmax dot(R(q) nᵢ, Y)',5.2,6.3,7.5,.6,{fontSize:24,color:C.gold});

  s=slide('Trò ghép hướng và BFS','6 lựa chọn mặt trên, 4 mặt trước kề nhau tạo 24 trạng thái. Mỗi cạnh quay ±90 độ quanh X/Y/Z, cùng chi phí. BFS cho đường ngắn nhất, gợi ý hiển thị bước đầu. Tổng ba khoảng cách là tối ưu toàn bài vì các viên độc lập. Demo chọn viên và H.', 'src/puzzle.cpp / shortestTurns, FacePuzzle');
  points(s,['6 × 4 = 24 hướng','Mỗi bước là\nphép quay ±90°','BFS tìm đường\nít phép quay nhất']);scene(s,'18_puzzle');
  text(s,'O(V + E), V = 24, E ≤ 144',5.2,6.3,7.5,.5,{fontSize:24,color:C.gold});

  s=slide('Lưu dữ liệu và kiểm tra','RAM dùng chung một Mesh. throws.csv lưu kết quả, không lưu quỹ đạo. OBJ xuất thân lưới chưa có chấm, transforms.csv xuất chuỗi Lab và chưa nhập lại. Bộ kiểm tra xác nhận công thức, mô phỏng, BFS và OpenGL. Không tuyên bố đã đo FPS hoặc kiểm định thống kê.', 'src/storage.cpp, src/tests.cpp, src/throw_tests.cpp; docs/KIEM_THU.md');
  table(s,[['Dữ liệu','Cách lưu'],['Hình học','Mesh chung trong RAM, xuất thân lưới OBJ'],['Lượt mô phỏng','CSV kết quả, lực, thời gian, số va chạm'],['Chuỗi biến đổi','CSV từng bước, trục, tâm biến đổi'],['Kiểm tra toán học','Ma trận, pháp tuyến, phép chiếu, 24 hướng'],['Kiểm tra thực thi','90 lượt mô phỏng và 26 cảnh OpenGL'] ],.65,1.8,12,[3.3,8.7]);
  text(s,'Giới hạn: va chạm gần đúng, bước ổn định và pipeline cố định',.8,6.4,11.8,.5,{fontSize:22,color:C.gold});

  s=slide('Demo và hướng phát triển','Kết thúc bằng các phần chứng minh được trực tiếp. Nếu còn thời gian mời thầy chọn thứ tự biến đổi hoặc phép chiếu. Hướng phát triển SAT/GJK, nhiều tiếp xúc và shader là tương lai, chưa có trong phiên bản này.');
  points(s,['F3  Mặt và lưới','F4  Tổ hợp biến đổi','F5  Phép chiếu\nF2/F6  Mô phỏng và trò chơi']);scene(s,'19_puzzle_solved');
  text(s,'Tiếp theo: SAT/GJK, GLSL và lưu replay',5.2,6.3,7.5,.65,{fontSize:21,color:C.gold});
  fs.writeFileSync(path.join(root,'docs','slides_manifest.json'),JSON.stringify(manifest,null,2),'utf8');
  await pptx.writeFile({fileName:path.join(root,'docs','SLIDE_THUYET_TRINH.pptx')});
  console.log('Wrote 14 editable slides with Vietnamese speaker notes');
})();
