"""Rebuild the Vietnamese technical report from the editable Markdown source."""
from pathlib import Path
from html import escape
import re
from PIL import Image
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, PageBreak, Image as PDFImage, Preformatted

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs'
for name, file in [('Arial', 'arial.ttf'), ('Arial-Bold', 'arialbd.ttf'), ('Consolas', 'consola.ttf')]:
    pdfmetrics.registerFont(TTFont(name, str(Path('C:/Windows/Fonts') / file)))
pdfmetrics.registerFontFamily('Arial', normal='Arial', bold='Arial-Bold', italic='Arial', boldItalic='Arial-Bold')

# These are crops of real application captures, not reconstructed illustrations.
for image in (DOCS/'images').glob('*.png'):
    if image.stem.endswith('_scene'):
        continue
    output = image.with_name(image.stem+'_scene.png')
    with Image.open(image) as im:
        bottom=872 if image.stem in {'05_model','06_faces','07_wireframe'} else 722
        im.crop((28, 174, 1018, bottom)).save(output)

def picture(name,width=499,maxheight=276):
    path=DOCS/'images'/name
    with Image.open(path) as im:
        ratio=im.height/im.width
    height=width*ratio
    if height>maxheight:
        width=maxheight/ratio
        height=maxheight
    return PDFImage(str(path),width=width,height=height)

styles = getSampleStyleSheet()
styles.add(ParagraphStyle('VBody', fontName='Arial', fontSize=10.5, leading=16.2, spaceAfter=10, alignment=TA_LEFT))
styles.add(ParagraphStyle('VTitle', fontName='Arial-Bold', fontSize=29, leading=38, textColor=colors.HexColor('#102c48'), spaceAfter=18))
styles.add(ParagraphStyle('VHeading', fontName='Arial-Bold', fontSize=18, leading=24, textColor=colors.HexColor('#123f59'), spaceAfter=18, keepWithNext=True))
styles.add(ParagraphStyle('VSmall', fontName='Arial', fontSize=9, leading=13.5, textColor=colors.HexColor('#516578'), spaceAfter=10))
styles.add(ParagraphStyle('VTOC', fontName='Arial', fontSize=12, leading=24, spaceAfter=4))
styles.add(ParagraphStyle('VCode', fontName='Consolas', fontSize=9.2, leading=13, spaceAfter=14))

def clean(s):
    s = escape(s).replace('–', '-').replace('—', '-')
    s = re.sub(r'\*\*(.*?)\*\*', r'<b>\1</b>', s)
    return re.sub(r'`(.*?)`', r'<font name="Consolas">\1</font>', s)

def footer(canvas, doc):
    canvas.saveState()
    w, h = A4
    if doc.page > 1:
        canvas.setFont('Arial', 8.5)
        canvas.setFillColor(colors.HexColor('#516578'))
        canvas.drawString(48, h-30, 'DICE / 3D LAB    Đồ án Đồ họa máy tính')
        canvas.drawRightString(w-48, 28, f'{doc.page}')
    canvas.restoreState()

doc = SimpleDocTemplate(str(DOCS/'BAO_CAO.pdf'), pagesize=A4, leftMargin=48, rightMargin=48,
    topMargin=56, bottomMargin=47, title='Mô phỏng Xúc xắc 3D - Báo cáo kỹ thuật', author='Dice 3D Lab')
story = [Spacer(1, 40), Paragraph('MÔ PHỎNG<br/>XÚC XẮC 3D', styles['VTitle']),
    Paragraph('VÀ TRÒ CHƠI ĐỔ XÚC XẮC', styles['VHeading']),
    Paragraph('Đồ án môn Đồ họa máy tính<br/>C++17 / OpenGL / FreeGLUT', styles['VTOC']), Spacer(1, 25)]
story += [picture('01_simulation_scene.png'), Spacer(1, 24),
    Paragraph('Phiên bản học thuật 2.0<br/>Mô hình hóa, biến đổi 3D, phép chiếu và hệ quan sát', styles['VTOC']),
    Paragraph('Tài liệu tham khảo chính: bài giảng 3D Graphics, 51 trang, do người học cung cấp. Các ảnh minh họa là ảnh chụp ứng dụng C++/OpenGL đi cùng báo cáo.', styles['VSmall']), PageBreak()]

source = (DOCS/'BAO_CAO.md').read_text(encoding='utf-8')
sections = re.split(r'^## ', source, flags=re.M)[1:]
story.append(Paragraph('MỤC LỤC NỘI DUNG', styles['VHeading']))
for section in sections:
    story.append(Paragraph(clean(section.splitlines()[0]), styles['VTOC']))
story.append(Spacer(1, 20))
story.append(Paragraph('Tài liệu kèm theo gồm slide thuyết trình, kịch bản demo 8-10 phút, câu hỏi bảo vệ và bảng đối chiếu số trang bài giảng.', styles['VSmall']))

illustrations = {1:'01_simulation', 4:'05_model', 5:'06_faces', 9:'18_puzzle', 10:'13_perspective'}
for section in sections:
    lines = section.strip().splitlines()
    title = lines[0]
    number = int(title.split('.')[0])
    story += [PageBreak(), Paragraph(clean(title), styles['VHeading'])]
    body='\n'.join(lines[1:]).strip()
    if '```' in body:
        parts=re.split(r'```[^\n]*\n(.*?)```',body,flags=re.S)
        for part_index,part in enumerate(parts):
            if part_index%2:
                story.append(Preformatted(part.rstrip(),styles['VCode']))
            else:
                for para in re.split(r'\n\s*\n',part.strip()):
                    if para.strip(): story.append(Paragraph(clean(' '.join(para.splitlines())),styles['VBody']))
        continue
    paragraphs = re.split(r'\n\s*\n', body)
    for index, paragraph in enumerate(paragraphs):
        if not paragraph.strip():
            continue
        text = ' '.join(paragraph.splitlines())
        if text.startswith('- '):
            for item in paragraph.splitlines():
                story.append(Paragraph(clean(item), styles['VBody']))
        else:
            story.append(Paragraph(clean(text), styles['VBody']))
        if index == 0 and number in illustrations:
            file = illustrations[number]
            story.extend([Spacer(1, 5), picture(f'{file}_scene.png'),
                Spacer(1, 8), Paragraph('Minh họa từ ứng dụng. Ảnh đầy đủ và các chế độ khác nằm trong docs/images.', styles['VSmall'])])
doc.build(story, onFirstPage=footer, onLaterPages=footer)
print('Wrote docs/BAO_CAO.pdf')
